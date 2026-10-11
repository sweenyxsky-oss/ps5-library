#include "library_index.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <set>
#include <sstream>
#include <system_error>

#include <nlohmann/json.hpp>

namespace ps5lib::library {
namespace fs = std::filesystem;
using nlohmann::json;

namespace {
constexpr int kIndexSchema = 1;

std::string lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

std::string safe_relative(const fs::path& path, const fs::path& root) {
    const auto relative = path.lexically_relative(root);
    if (relative.empty() || relative.is_absolute()) return {};
    for (const auto& part : relative) if (part == "..") return {};
    return relative.generic_string();
}

ItemKind parse_kind(const std::string& value) {
    if (value == "game") return ItemKind::Game;
    if (value == "dlc") return ItemKind::Dlc;
    if (value == "update") return ItemKind::Update;
    return ItemKind::Unknown;
}

Confidence parse_confidence(const std::string& value) {
    if (value == "verified") return Confidence::Verified;
    if (value == "manifest") return Confidence::Manifest;
    if (value == "provisional") return Confidence::Provisional;
    return Confidence::Unknown;
}

json item_json(const LibraryItem& item) {
    return {{"kind", to_string(item.kind)}, {"relative_path", item.relative_path},
            {"format", item.format},
            {"size_bytes", item.size_known ? json(item.size_bytes) : json(nullptr)}};
}

json entry_json(const LibraryEntry& entry) {
    json items = json::array();
    for (const auto& item : entry.items) items.push_back(item_json(item));
    return {{"entry_id", entry.entry_id}, {"title", entry.title},
            {"title_id", entry.title_id}, {"title_source", entry.title_source},
            {"confidence", to_string(entry.confidence)},
            {"relative_directory", entry.relative_directory},
            {"cover_path", entry.cover_path}, {"items", std::move(items)}};
}

bool read_manifest(const fs::path& manifest, const fs::path& root,
                   LibraryEntry* entry, std::string* error) {
    std::ifstream input(manifest);
    if (!input) return false;
    try {
        json data;
        input >> data;
        if (!data.is_object() || data.value("schema_version", 0) != 1 ||
            !data.contains("title") || !data["title"].is_string() ||
            !data.contains("items") || !data["items"].is_array()) {
            if (error) *error = "manifest schema or required fields are invalid: " + manifest.string();
            return false;
        }
        entry->title = data["title"].get<std::string>();
        if (entry->title.empty()) {
            if (error) *error = "manifest title is empty: " + manifest.string();
            return false;
        }
        if (data.contains("title_id") && data["title_id"].is_string())
            entry->title_id = data["title_id"].get<std::string>();
        if (data.contains("cover") && data["cover"].is_string()) {
            const fs::path cover = manifest.parent_path() / data["cover"].get<std::string>();
            entry->cover_path = safe_relative(cover.lexically_normal(), root);
        }
        entry->title_source = "manifest";
        entry->confidence = Confidence::Manifest;
        for (const auto& value : data["items"]) {
            if (!value.is_object() || !value.contains("path") || !value["path"].is_string())
                continue;
            const fs::path rel(value["path"].get<std::string>());
            if (rel.is_absolute()) continue;
            bool escapes = false;
            for (const auto& part : rel) if (part == "..") escapes = true;
            if (escapes) continue;
            LibraryItem item;
            item.kind = value.contains("kind") && value["kind"].is_string()
                            ? parse_kind(value["kind"].get<std::string>()) : ItemKind::Unknown;
            item.relative_path = (fs::path(entry->relative_directory) / rel).lexically_normal().generic_string();
            if (item.relative_path.empty() || item.relative_path.rfind("..", 0) == 0) continue;
            item.format = value.contains("format") && value["format"].is_string()
                              ? value["format"].get<std::string>() : lower(rel.extension().string());
            if (value.contains("size_bytes") && value["size_bytes"].is_number_unsigned()) {
                item.size_bytes = value["size_bytes"].get<std::uint64_t>();
                item.size_known = true;
            } else if (value.contains("size_bytes") && value["size_bytes"].is_number_integer() &&
                       value["size_bytes"].get<std::int64_t>() >= 0) {
                item.size_bytes = static_cast<std::uint64_t>(value["size_bytes"].get<std::int64_t>());
                item.size_known = true;
            }
            entry->items.push_back(std::move(item));
        }
        return true;
    } catch (const std::exception& ex) {
        if (error) *error = std::string("cannot parse manifest ") + manifest.string() + ": " + ex.what();
        return false;
    }
}
} // namespace

const char* to_string(ItemKind value) {
    switch (value) {
        case ItemKind::Game: return "game";
        case ItemKind::Dlc: return "dlc";
        case ItemKind::Update: return "update";
        default: return "unknown";
    }
}
const char* to_string(Confidence value) {
    switch (value) {
        case Confidence::Verified: return "verified";
        case Confidence::Manifest: return "manifest";
        case Confidence::Provisional: return "provisional";
        default: return "unknown";
    }
}

ScanResult scan_local_root(const std::string& root_text) {
    ScanResult result;
    std::error_code ec;
    const fs::path root = fs::weakly_canonical(fs::path(root_text), ec);
    if (ec || !fs::exists(root, ec) || !fs::is_directory(root, ec)) {
        result.error = "selected root does not exist or is not a readable directory";
        return result;
    }

    std::set<std::string> visited_manifests;
    std::set<std::string> grouped_directories;
    fs::recursive_directory_iterator it(root, fs::directory_options::none, ec), end;
    if (ec) {
        result.error = "cannot enumerate selected root";
        return result;
    }

    for (; it != end; it.increment(ec)) {
        if (ec) {
            // An inaccessible subtree makes the scan partial; preserve the error.
            result.error = "one or more directories could not be scanned";
            ec.clear();
            continue;
        }
        const fs::directory_entry de = *it;
        std::error_code type_ec;
        if (de.is_symlink(type_ec)) {
            if (de.is_directory(type_ec)) it.disable_recursion_pending();
            continue;
        }
        if (!de.is_regular_file(type_ec)) continue;

        const std::string filename = lower(de.path().filename().string());
        if (filename == "game.json") {
            const std::string rel_dir = safe_relative(de.path().parent_path(), root);
            if (rel_dir.empty() && de.path().parent_path() != root) continue;
            LibraryEntry entry;
            entry.relative_directory = rel_dir;
            if (read_manifest(de.path(), root, &entry, &result.error)) {
                entry.entry_id = rel_dir.empty() ? "." : rel_dir;
                for (auto& item : entry.items) {
                    const fs::path actual = root / item.relative_path;
                    type_ec.clear();
                    if (!fs::exists(actual, type_ec) || type_ec ||
                        !fs::is_regular_file(actual, type_ec) || type_ec) {
                        // Keep declared entries visible, but never invent their size.
                        continue;
                    }
                    if (!item.size_known) {
                        const auto actual_size = fs::file_size(actual, type_ec);
                        if (!type_ec) {
                            item.size_bytes = actual_size;
                            item.size_known = true;
                        }
                    }
                }
                visited_manifests.insert(de.path().string());
                grouped_directories.insert(rel_dir);
                result.entries.push_back(std::move(entry));
            }
        }
    }

    // Add standalone candidate files not already covered by a game.json manifest.
    ec.clear();
    fs::recursive_directory_iterator files(root, fs::directory_options::none, ec);
    for (; files != end; files.increment(ec)) {
        if (ec) { result.error = "one or more directories could not be scanned"; ec.clear(); continue; }
        const auto& de = *files;
        std::error_code file_ec;
        if (de.is_symlink(file_ec) || !de.is_regular_file(file_ec)) continue;
        const std::string name = de.path().filename().string();
        const std::string ext = lower(de.path().extension().string());
        if (name == "game.json" || ext != ".exfat" && ext != ".ffpkg" &&
            ext != ".ffpfs" && ext != ".ffpfsc" && ext != ".pkg") continue;

        const std::string rel = safe_relative(de.path(), root);
        if (rel.empty()) continue;
        const std::string rel_dir = safe_relative(de.path().parent_path(), root);
        if (grouped_directories.count(rel_dir)) continue;

        LibraryEntry entry;
        entry.entry_id = rel;
        entry.relative_directory = rel_dir;
        entry.title = de.path().stem().string();
        entry.title_source = "filename";
        entry.confidence = Confidence::Provisional;
        LibraryItem item;
        item.relative_path = rel;
        item.format = ext.substr(1);
        item.kind = ItemKind::Unknown; // Do not infer game/DLC/update from extension.
        const auto size = de.file_size(file_ec);
        if (!file_ec) { item.size_bytes = size; item.size_known = true; }
        entry.items.push_back(std::move(item));
        result.entries.push_back(std::move(entry));
    }

    result.complete = result.error.empty();
    return result;
}

bool save_index_atomic(const std::string& path, const ScanResult& result,
                       const std::string& source_id, std::string* error) {
    if (!result.complete) {
        if (error) *error = "refusing to replace the last-good index with an incomplete scan";
        return false;
    }
    const fs::path target(path);
    const fs::path temporary = target.string() + ".tmp";
    try {
        json entries = json::array();
        for (const auto& entry : result.entries) entries.push_back(entry_json(entry));
        json document = {{"schema_version", kIndexSchema}, {"source_id", source_id},
                         {"scan_status", "complete"}, {"entries", std::move(entries)}};
        {
            std::ofstream output(temporary, std::ios::binary | std::ios::trunc);
            if (!output) {
                if (error) *error = "cannot open temporary index for writing";
                return false;
            }
            output << document.dump(2) << "\n";
            output.flush();
            if (!output) {
                if (error) *error = "failed writing temporary index";
                return false;
            }
        }
        std::error_code ec;
        fs::rename(temporary, target, ec);
        if (ec) {
            if (error) *error = "cannot atomically replace index: " + ec.message();
            return false;
        }
        return true;
    } catch (const std::exception& ex) {
        if (error) *error = ex.what();
        return false;
    }
}

bool load_index(const std::string& path, ScanResult* result,
                std::string* source_id, std::string* error) {
    if (!result) { if (error) *error = "result pointer is null"; return false; }
    try {
        std::ifstream input(path, std::ios::binary);
        if (!input) { if (error) *error = "index does not exist or cannot be opened"; return false; }
        json document;
        input >> document;
        if (!document.is_object() || document.value("schema_version", 0) != kIndexSchema ||
            document.value("scan_status", std::string()) != "complete" ||
            !document.contains("entries") || !document["entries"].is_array()) {
            if (error) *error = "index schema or status is invalid";
            return false;
        }
        ScanResult loaded;
        loaded.complete = true;
        for (const auto& value : document["entries"]) {
            if (!value.is_object()) continue;
            LibraryEntry entry;
            entry.entry_id = value.value("entry_id", std::string());
            entry.title = value.value("title", std::string());
            entry.title_id = value.value("title_id", std::string());
            entry.title_source = value.value("title_source", std::string());
            entry.confidence = parse_confidence(value.value("confidence", std::string("unknown")));
            entry.relative_directory = value.value("relative_directory", std::string());
            entry.cover_path = value.value("cover_path", std::string());
            if (value.contains("items") && value["items"].is_array()) {
                for (const auto& item_value : value["items"]) {
                    if (!item_value.is_object()) continue;
                    LibraryItem item;
                    item.kind = parse_kind(item_value.value("kind", std::string("unknown")));
                    item.relative_path = item_value.value("relative_path", std::string());
                    item.format = item_value.value("format", std::string());
                    if (item_value.contains("size_bytes") && item_value["size_bytes"].is_number_unsigned()) {
                        item.size_bytes = item_value["size_bytes"].get<std::uint64_t>();
                        item.size_known = true;
                    }
                    entry.items.push_back(std::move(item));
                }
            }
            loaded.entries.push_back(std::move(entry));
        }
        if (source_id) *source_id = document.value("source_id", std::string());
        *result = std::move(loaded);
        return true;
    } catch (const std::exception& ex) {
        if (error) *error = ex.what();
        return false;
    }
}

} // namespace ps5lib::library

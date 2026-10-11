#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ps5lib::library {

enum class ItemKind { Game, Dlc, Update, Unknown };
enum class Confidence { Verified, Manifest, Provisional, Unknown };

struct LibraryItem {
    ItemKind kind{ItemKind::Unknown};
    std::string relative_path;
    std::string format;
    std::uint64_t size_bytes{0};
    bool size_known{false};
};

struct LibraryEntry {
    std::string entry_id;
    std::string title;
    std::string title_id;
    std::string title_source;
    Confidence confidence{Confidence::Unknown};
    std::string relative_directory;
    std::string cover_path;
    std::vector<LibraryItem> items;
};

struct ScanResult {
    bool complete{false};
    std::string error;
    std::vector<LibraryEntry> entries;
};

// Scans a local directory tree. An .exfat extension is never treated as proof
// of a game's identity; without a manifest, its title is provisional.
ScanResult scan_local_root(const std::string& root);

// Reads/writes the versioned index. Writes use a sibling temporary file and
// rename only after successful serialization.
bool save_index_atomic(const std::string& path, const ScanResult& result,
                       const std::string& source_id, std::string* error);
bool load_index(const std::string& path, ScanResult* result,
                std::string* source_id, std::string* error);

const char* to_string(ItemKind value);
const char* to_string(Confidence value);

} // namespace ps5lib::library

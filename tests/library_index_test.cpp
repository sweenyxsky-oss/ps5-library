#include "library_index.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
namespace fs = std::filesystem;
using namespace ps5lib::library;
namespace {
void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}
void write_file(const fs::path& path, const std::string& content) {
    fs::create_directories(path.parent_path());
    std::ofstream out(path, std::ios::binary);
    out << content;
    require(static_cast<bool>(out), "failed to create test fixture: " + path.string());
}
}
int main() {
    const fs::path temp = fs::temp_directory_path() / "ps5-library-index-tests";
    std::error_code ec;
    fs::remove_all(temp, ec);
    fs::create_directories(temp / "Loose");
    fs::create_directories(temp / "ManifestGame");
    write_file(temp / "Loose" / "mystery.exfat", "not a real game image");
    write_file(temp / "ManifestGame" / "base.pkg", "pkg");
    write_file(temp / "ManifestGame" / "game.json",
        R"({"schema_version":1,"title":"Manifest Test Game","title_id":"TEST00001","cover":"cover.png","items":[{"path":"base.pkg","kind":"game","format":"pkg"}]})");
    write_file(temp / "ManifestGame" / "cover.png", "cover");
    try {
        const auto scan = scan_local_root(temp.string());
        require(scan.complete, "valid test tree should scan completely: " + scan.error);
        require(scan.entries.size() == 2, "expected one manifest entry and one loose image");
        bool found_loose = false, found_manifest = false;
        for (const auto& entry : scan.entries) {
            if (entry.title == "mystery") {
                found_loose = true;
                require(entry.confidence == Confidence::Provisional, ".exfat must remain provisional");
                require(entry.items.size() == 1, "loose image should have one item");
                require(entry.items[0].kind == ItemKind::Unknown, "extension must not infer item kind");
                require(entry.items[0].size_known, "local file size should be available");
            }
            if (entry.title == "Manifest Test Game") {
                found_manifest = true;
                require(entry.title_id == "TEST00001", "manifest title ID missing");
                require(entry.confidence == Confidence::Manifest, "manifest confidence missing");
                require(entry.items.size() == 1, "manifest item should be present");
                require(entry.items[0].relative_path == "ManifestGame/base.pkg", "manifest path mismatch");
                require(entry.cover_path == "ManifestGame/cover.png", "cover path should be relative");
            }
        }
        require(found_loose, "loose .exfat entry not found");
        require(found_manifest, "manifest entry not found");
        const fs::path index = temp / "cache" / "library-index.json";
        fs::create_directories(index.parent_path());
        std::string error;
        require(save_index_atomic(index.string(), scan, "test-source", &error), "save failed: " + error);
        ScanResult loaded;
        std::string source_id;
        require(load_index(index.string(), &loaded, &source_id, &error), "load failed: " + error);
        require(loaded.complete && loaded.entries.size() == scan.entries.size(), "loaded count mismatch");
        require(source_id == "test-source", "source ID was not persisted");
        const auto previous_bytes = fs::file_size(index);
        ScanResult incomplete;
        incomplete.complete = false;
        require(!save_index_atomic(index.string(), incomplete, "test-source", &error),
                "incomplete scan must not replace cache");
        require(fs::file_size(index) == previous_bytes, "incomplete scan changed last-good cache");
        require(!scan_local_root((temp / "does-not-exist").string()).complete,
                "nonexistent root must fail closed");
        fs::remove_all(temp, ec);
        std::cout << "library_index_tests: all checks passed\n";
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << "library_index_tests: " << ex.what() << '\n';
        fs::remove_all(temp, ec);
        return 1;
    }
}

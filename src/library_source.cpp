#include "library_source.hpp"

#include <filesystem>

namespace ps5lib::library {
namespace fs = std::filesystem;

bool probe_default_hostapp(std::string* root_path, std::string* error) {
    std::error_code ec;
    const fs::path root("/hostapp");
    if (!fs::exists(root, ec) || ec || !fs::is_directory(root, ec) || ec) {
        if (error) *error = "/hostapp is unavailable; choose a local folder or connect an NFS source first";
        return false;
    }
    if (root_path) *root_path = root.string();
    if (error) error->clear();
    return true;
}

bool validate_source_config(const SourceConfig& config, std::string* error) {
    if (config.source_id.empty()) {
        if (error) *error = "source_id must not be empty";
        return false;
    }
    if (config.root_path.empty() || config.root_path.front() != '/') {
        if (error) *error = "root_path must be an absolute path";
        return false;
    }
    const fs::path path(config.root_path);
    for (const auto& part : path) {
        if (part == "..") {
            if (error) *error = "root_path must not contain parent traversal";
            return false;
        }
    }
    if (config.kind == SourceKind::MountedNetworkDirectory && config.root_path != "/hostapp") {
        if (error) *error = "mounted network sources must use the managed /hostapp mountpoint";
        return false;
    }
    if (error) error->clear();
    return true;
}

} // namespace ps5lib::library

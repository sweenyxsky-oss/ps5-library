#pragma once

#include <string>

namespace ps5lib::library {

enum class SourceKind {
    LocalDirectory,
    MountedNetworkDirectory
};

struct SourceConfig {
    SourceKind kind{SourceKind::LocalDirectory};
    std::string source_id;
    std::string root_path;
};

// Resolves the requested default only when /hostapp exists and is a directory.
// It never silently substitutes another directory.
bool probe_default_hostapp(std::string* root_path, std::string* error);

// Validates source identity and root path shape without performing network I/O.
// For a direct NFS connection, the UI/backend must mount or provide a libnfs-backed
// provider before handing a root to the scanner.
bool validate_source_config(const SourceConfig& config, std::string* error);

} // namespace ps5lib::library

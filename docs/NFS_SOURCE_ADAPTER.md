# NFS source adapter contract

## Scope

This is the integration contract for the library index. It is not a claim that direct NFS connectivity is already wired into the PS5 Library application.

The current source validator accepts a mounted network root only at /hostapp. The separate PS5-Network-Mounting project is the current candidate for mounting TrueNAS NFS exports there. A future direct libnfs provider may implement the same read-only file operations, but must be verified against the actual PS5 target toolchain before selection.

## Connection form

- Server address or IP
- Export path (absolute path as exported by TrueNAS)
- Port (default NFS port 2049)
- Read-only browsing
- Optional connection label

Do not ask for a generic NFS username/password as though NFS were SMB. NFS access is normally controlled by export rules and UID/GID mapping; only expose authentication fields if the selected NFS implementation explicitly supports them.

## Required provider behavior

1. Validate host and export path before connecting.
2. Use a bounded connection timeout and a cancelable connect/list/read operation.
3. Return normalized relative paths and metadata (file type, size when available, and last-modified time when available).
4. Never follow symlinks outside the selected root.
5. Report permission and I/O failures as partial scans; do not replace the last-good cache on partial results.
6. Treat the share as read-only; no rename, delete, write, or install operation is part of source browsing.
7. Handle disconnect/reconnect without hanging the controller UI.
8. Keep local-root scanning and NFS-root scanning behind the same source contract.

## Library identity rules

A filename extension is not a game identity. An .exfat file with no manifest or trusted metadata must remain provisional. Group base game, DLC, and updates only when a trusted title ID or explicit game manifest establishes the relationship. Package format alone must not be treated as proof that a file is installable on firmware 13.60.

## Current implementation boundary

- Implemented: source config shape validation and conservative /hostapp probing in the portable C++ component.
- Not implemented here: NFS network connection, mount lifecycle, UI folder picker, direct libnfs adapter, or PS5 console integration.

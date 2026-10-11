# Persistent Library Index Model

The index is a cache, not the source of truth. The active source remains the configured local path or NFS export.

## Top-level document

- `schema_version`: integer; increment for incompatible index changes.
- `source_id`: stable opaque ID for the configured source (never include credentials).
- `last_scan_started`, `last_scan_completed`: timestamps.
- `scan_status`: `complete`, `partial`, or `failed`.
- `entries`: array of game groups and unresolved files.

## Game group

- `entry_id`: stable ID derived from source ID and normalized relative path, not from a mutable display title.
- `title_id`: nullable, only when read from trustworthy metadata or supplied by a manifest.
- `title`: display title.
- `title_source`: `embedded_metadata`, `manifest`, `directory_name`, or `manual`.
- `identification_confidence`: `verified`, `manifest`, `provisional`, or `unknown`.
- `cover_path`: optional local/source-relative image.
- `items`: records with `kind` (`game`, `dlc`, `update`, `unknown`), relative path, extension/format, byte size, modified time if available, and detected support status.

## Scan rules

- Never use extension alone to assign a title ID.
- Do not assume every `.exfat` file can be parsed without mounting.
- If the source is temporarily unavailable, retain the last complete index and report that it may be stale.
- During a scan, write a temporary index and atomically replace the last-good index only after validation.
- A partial scan must not delete entries merely because a directory was inaccessible.
- Normalize relative paths and reject traversal outside the selected root.
- Store no passwords, tokens, or other source credentials in the index.

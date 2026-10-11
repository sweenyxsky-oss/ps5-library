# Library Browser Acceptance Tests

These are implementation acceptance cases, not a claim that they have already passed on a PS5.

## Source selection

- [ ] First launch offers `/hostapp` as the default local root and probes whether it exists and is readable.
- [ ] A folder picker can navigate up/down and select the current directory.
- [ ] NFS source accepts server and export path, validates required fields, and tests connectivity before save.
- [ ] NFS authentication controls are shown only for an authentication method supported by the chosen backend.
- [ ] Credentials, if supported, are stored outside the game index and never written to logs.
- [ ] When a source is offline, the previous cached library remains visible with a stale/offline indicator.

## Scanner and identification

- [ ] A filename ending in `.exfat` is classified as an image candidate, not automatically assigned a game title.
- [ ] Verified embedded metadata or a supported mount API can assign a title ID.
- [ ] A valid manifest groups Game, DLC, and Updates under one title.
- [ ] A folder-name fallback is marked provisional.
- [ ] Unknown files remain accessible in an unresolved-files section.
- [ ] Paths containing spaces and non-ASCII characters are handled safely.
- [ ] Symlink/path traversal cannot escape the selected root where the backend exposes such paths.
- [ ] A failed or interrupted scan preserves the last-good index.
- [ ] A full rescan removes entries only after a complete successful scan.

## Library UI

- [ ] Installed Games and Library Browser are separate destinations.
- [ ] Cover grid and horizontal cover-row layouts can be selected without losing scan state.
- [ ] Game detail view shows title, identification confidence, source path, file size, and file format.
- [ ] Game, DLC, and Update tabs list files and expose format/compatibility status.
- [ ] Install All DLC runs sequentially, reports each result, and stops safely when a required operation fails.
- [ ] Unsupported operations are disabled with a reason rather than appearing successful.

## Integration and hardware-only checks

- [ ] Confirm existing PS5 Library build/tests pass before and after each integration change.
- [ ] Build libnfs for the exact PS5 toolchain before choosing direct NFS as the shipping provider.
- [ ] Verify NFS connect/list/read against a real TrueNAS export on target hardware.
- [ ] Verify ShadowMountPlus API compatibility and status reporting against the installed payload version.
- [ ] Verify image mounting/installation and actual game launch separately.
- [ ] Do not claim PS5 hardware validation from host-only CI.

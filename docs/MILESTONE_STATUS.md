# PS5 Library + TrueNAS integration milestones

This is a development branch. The default branch is not modified.

## Milestone 1 — Data contract and source configuration
- [x] Document source model and /hostapp default probing behavior.
- [x] Add example manifest and JSON schema.
- [x] Define persistent index and cache expectations.
- [x] Document acceptance criteria.
- [ ] Validate schema with a JSON Schema validator in CI.

## Milestone 2 — Portable scanner and cache
- [x] Add C++17 local-tree scanner and versioned index serialization.
- [x] Keep unknown .exfat / package files provisional; do not infer title or item kind from extension.
- [x] Refuse to overwrite last-good index after an incomplete scan.
- [x] Add standalone CMake build and host unit tests.
- [ ] Confirm GitHub Actions host test passes (no run/status is visible yet).
- [ ] Test symlink, permission-denied, malformed-manifest, and cache-corruption cases.

## Milestone 3 — Source selection and NFS adapter
- [ ] Integrate /hostapp auto-detection and folder-picker UI into the original PS5 Library app.
- [x] Define the source-config boundary and mounted-root contract; full provider interface still pending.
- [ ] Confirm which existing PS5 NFS API/library is compatible with the actual payload runtime.
- [ ] Add NFS connection test, read-only browse, timeout, cancellation, and disconnect recovery.

## Milestone 4 — Game grouping and details
- [ ] Group base game, DLC, and updates only using trusted manifest/title metadata.
- [ ] Add title-ID validation and metadata precedence rules.
- [ ] Add cover/metadata cache and game detail model.
- [ ] Connect the model to the PS5 controller-friendly UI.

## Milestone 5 — Console integration and release gate
- [ ] Restore or obtain the original PS5 Library source/build configuration in this fork.
- [ ] Integrate with existing installer/mounting entry points after inspecting actual source.
- [ ] Build the PS5 ELF with a verified PS5 toolchain.
- [ ] Test on firmware 13.60 hardware and document format-specific compatibility.
- [ ] Do not advertise untested formats as mountable or installable.

## Current blocker
The fork's README and docs are present, but the original application source and build files (including the expected root build file) were not found on this branch. The index and source-validation components are isolated host-testable code; they are not linked into the original PS5 UI or payload. A passing host test would not prove PS5 SDK compatibility or firmware 13.60 operation. The newly added CI workflow has not yet returned a run or status through the available GitHub status interface.

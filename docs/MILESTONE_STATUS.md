# Native PS5 TrueNAS Game Library — milestone tracker

This is a development branch. The default branch is not modified. **Product direction: native PS5 application, not a browser-based UI.** The existing browser-based ps5-library is reference material only; do not treat it as the target application shell. **Product direction: native PS5 application, not a browser-based UI.** The existing browser-based ps5-library is reference material only; do not treat it as the target application shell.

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

## Milestone 3 — Native app foundation and source selection
- [ ] Select a source-complete native app framework and build target; do not integrate into the browser-based PS5 Library UI.
- [x] Record native app direction and target constraints.
- [x] Define the source-config boundary and mounted-root contract; full provider interface still pending.
- [ ] Confirm which existing PS5 NFS API/library is compatible with the actual payload runtime.
- [ ] Add NFS connection test, read-only browse, timeout, cancellation, and disconnect recovery.

## Milestone 4 — Native game-library UI and metadata
- [ ] Group base game, DLC, and updates only using trusted manifest/title metadata.
- [ ] Add title-ID validation and metadata precedence rules.
- [ ] Add cover/metadata cache and game detail model.
- [ ] Connect the model to the PS5 controller-friendly UI.

## Milestone 5 — Native console integration and release gate
- [ ] Restore or obtain the original PS5 Library source/build configuration in this fork.
- [ ] Integrate with existing installer/mounting entry points after inspecting actual source.
- [ ] Build the PS5 ELF with a verified PS5 toolchain.
- [ ] Test on firmware 13.60 hardware and document format-specific compatibility.
- [ ] Do not advertise untested formats as mountable or installable.

## Current blocker
The browser-based ps5-library fork lacks its original application source/build files and is not the native app target. The index and source-validation components on this branch remain portable prototypes; they are not yet integrated into a native PS5 app. A host test would not prove PS5 SDK compatibility or firmware 13.60 operation. The newly added CI workflow has not yet returned a run or status through the available GitHub status interface. The next implementation base should be a source-complete native app framework (for example, SDL2/ps5-payload-sdk or a native app boilerplate), after checking its firmware evidence, build prerequisites, licensing, and filesystem/network capabilities.

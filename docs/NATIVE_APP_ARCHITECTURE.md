# Native PS5 TrueNAS Game Library — architecture decision

## Decision

The product is a **native PS5 application**, not a browser page, WebView, or noVNC frontend. The browser-based ps5-library fork is useful only as a reference for library behavior and metadata concepts.

## Candidate foundation

Evaluate a source-complete native homebrew UI with a controller input loop and a PS5 ELF build pipeline. Candidate references include native SDL2 launcher examples built around ps5-payload-sdk, a native PS5 app boilerplate with reproducible host build, and a native UI framework with controller focus/navigation. A candidate is not accepted merely because its README says native: verify actual source files, build steps, output format, target ABI, firmware test evidence, license, and filesystem access.

## Product architecture

1. **Native shell:** fullscreen TV UI, DualSense navigation, focus/selection state, details panel, error and confirmation dialogs.
2. **Source manager:** probe /hostapp by default; permit a local folder picker if the runtime exposes directory browsing; provide an NFS connection form.
3. **Storage provider:** abstract local filesystem and NFS operations behind one read-only listing/stat/open interface. Prefer a known-compatible existing NFS stack rather than adding a browser or HTTP layer.
4. **Index/cache:** persistent JSON or SQLite index, atomic replacement, last-good cache retained on partial scans, explicit refresh and offline state.
5. **Metadata resolver:** use explicit manifest and trusted title IDs first. Keep unknown .exfat or package files provisional. Do not guess game identity from extension.
6. **Game grouping:** link base game, DLC and updates only through trusted IDs or manifest relationships.
7. **Action adapter:** mount, install, or launch only through verified interfaces supported by target firmware and payload environment. Show unsupported/unknown states honestly.

## Source selection UI

- **Default:** probe /hostapp; if it is not mounted or readable, show an empty state rather than silently switching paths.
- **PS5 folder:** browse and select a folder only if the chosen native runtime exposes the required filesystem permissions/APIs.
- **NFS:** server/IP, export path, optional port (default 2049), connection test, read-only browse, disconnect/reconnect.
- **Credentials:** do not present SMB-style username/password as universally required for NFS. NFS access usually depends on server export policy and UID/GID mapping; expose authentication only if the selected implementation supports it.

## Current state and hard boundary

The current branch contains an index/scanner prototype and source-config validation. It is not a native application and is not connected to a UI, NFS transport, mount payload, or installer. The browser-based source is not to be used as the application shell.

## Release gates

- Host unit tests pass.
- Native target builds reproducibly with documented toolchain.
- NFS/local listing, metadata, large-file stat/read, disconnect and recovery tests pass on target hardware.
- Firmware 13.60 execution is demonstrated for the exact built artifact.
- No format is advertised as mountable/installable without a successful target test.

# TrueNAS Library Browser Integration Plan

Target: PS5 Library on jailbroken PS5 firmware 13.60, using the existing PS5 Library payload and its supported integration points.

## Scope

- Preserve the existing Installed Games view and download/install workflows.
- Add a separate Library Browser for user-owned content reachable from a local path or a configured NFS export.
- Default local root: `/hostapp`, but treat it as a candidate path until runtime probing confirms it exists and is readable.
- Support the user's organization (`Game/`, `DLC/`, `Updates/`) without requiring all NAS folders to be renamed or reorganized.
- Persist a versioned index so normal launches load cached results; provide refresh and full-rescan actions.
- Group Game, DLC, and Updates using verified title/content IDs when available; use a manifest or folder-name fallback with explicit confidence status.

## Repository roles

- `ps5-library`: primary UI, payload, persistent settings/index, install workflow.
- `ShadowMountPlus`: supported image formats and existing mount/registration/status interfaces. Do not duplicate its mount logic without evidence that its public interface is insufficient.
- `ps5-ezremote-client` and `Itemzflow`: reference implementations for remote access and PS5 UI/runtime integration; reuse only code whose license and API fit.
- `libnfs`: candidate direct NFS client library, subject to PS5 SDK/ABI/build verification.
- `fuse-nfs`: optional reference only. Do not assume a FUSE mount can run in the PS5 payload environment.
- `RemotePFS`: separate Linux SBC design for presenting a NAS as virtual read-only exFAT USB storage; not a native NFS client inside PS5 Library.

## Identification order

1. Read supported image/container metadata only through a verified parser or mount API.
2. Match title ID/content ID against locally available metadata.
3. Read optional `game.json` manifest.
4. Infer a provisional title from directory/file names and label it unverified.
5. Allow manual correction and persist the user's match.

An `.exfat` suffix is not itself a game identifier. Do not guess title IDs from arbitrary filenames.

## NFS source settings

- Server hostname/IP and export path.
- NFS version and port only if supported by the chosen client.
- Username/password fields must not be shown as if standard NFS supports SMB-style user/password authentication. NFS normally relies on export and UID/GID or Kerberos policy; expose extra authentication only if the actual backend supports it.
- Test connection before saving; never log secrets.
- Keep the cached index available while disconnected, but disable operations that require live source access.

## Installation safety

- Do not claim every image/package is installable on firmware 13.60.
- Detect supported formats and route through existing ShadowMountPlus or PS5 Library installer interfaces where proven.
- Do not copy a whole NAS game to the console by default. Prefer direct access only if the existing runtime accepts it; otherwise explain and measure staging requirements.
- Install All DLC sequentially with per-file result and clear failure state.
- Treat mount/installation and launch as separate states.

## Milestones

1. Verify fork ownership, source layout, build system, runtime entry points and existing APIs.
2. Define and test the versioned library index and manifest model.
3. Implement source abstraction and local-path scan with safe file metadata handling.
4. Add NFS provider using a PS5-buildable library path; prove connect/list/read in a standalone test before UI integration.
5. Integrate UI, cache refresh, game details and installer routing; run available host tests and document hardware-only validation.

## Acceptance gates

- No writes to default branches during development.
- Index updates are atomic; interrupted scans do not destroy the last good cache.
- Disconnected NFS shares do not erase cached titles.
- Unknown file formats remain visible as unknown instead of being misidentified.
- Tests run in CI or host environment are reported separately from PS5 hardware tests.

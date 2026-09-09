# Codename Subspace

**Codename Subspace** is a native C++17/OpenGL space sandbox focused on modular ship and station construction, cockpit flight, on-foot gameplay, industry, exploration, fleet command, persistent worlds, and a large player-driven economy.

The authoritative runtime is **native C++ only**. The former C# prototype is retired from production and is not part of the normalized GitHub `main` authority.

## Current development direction

The project is being normalized into one cohesive sandbox that combines:

- modular ship/station construction with authored sockets and kitbash parts;
- cockpit-first flight plus tactical/fleet-command views;
- on-foot interiors, stations, planets, moons, caves, and industry sites;
- EVE-like fitting, economy, skills, corporations, and security regions;
- X4-like fleet command, NPC industry, and strategic simulation;
- Starbase-like modular construction and connected interiors;
- seamless or masked-seamless planetary/system travel at very large scale;
- persistent sectors and planetary hex territories;
- a server-authoritative simulation model, including local-hosted single-player.

The active Shipyard line includes semantic kitbash classification, socket editing, construction symmetry, dedicated editor-camera behavior, profile-following shield rendering, paint/decal workflows, and render-performance hardening.

## Runtime authority

| Area | Authority |
| --- | --- |
| Game/runtime | `engine/` native C++ |
| Build | CMake + Project Control Center |
| Root operations | `SubspaceTools.cmd` / `SubspaceTools.ps1` |
| Project contract | `project.control.json` |
| Runtime/game data | `GameData/` |
| Governed source/content metadata | `content/` |
| Project tools | `tools/` and `scripts/` |
| Design/architecture/history | `docs/` |

Legacy donor/prototype material may remain in local developer workspaces for reference, but it is ignored by the normalized repository authority. The pre-normalization GitHub history is preserved on an archive branch before the current `main` branch is replaced.

## Supported Windows workflow

Launch:

```text
SubspaceTools.cmd
```

Use the **Project Control Center** as the normal project entry point.

Primary paths:

```text
Build & verify
  -> Full Quality Gate

Run & play
  -> Native game
  -> Shipyard Dev Studio

Source control
  -> Repository authority audit
  -> Prepare normalized GitHub authority
  -> Publish last GREEN normalized main
```

A Full Quality Gate is the promotion authority. It verifies source continuity, supply-chain state, native C++ authority, configure/build/tests, Shipyard live authoring checks, native executable presence, runtime smoke, and a certified source snapshot.

## Manual native build

```powershell
cmake -S engine -B engine/build `
  -DSUBSPACE_HEADLESS=OFF `
  -DSUBSPACE_BUILD_OPENGL=ON `
  -DSUBSPACE_BUILD_TESTS=ON

cmake --build engine/build --config Debug
ctest --test-dir engine/build -C Debug --output-on-failure
```

The Project Control Center remains preferred because it also verifies prerequisite, content, update, rollback, diagnostics, and certification contracts.

## Repository layout

```text
Codename-Subspace/
├─ engine/                 Native C++ runtime, renderer, and tests
├─ GameData/               Runtime-authored JSON/data
├─ content/                Source registries, schemas, governed metadata
├─ scripts/                Project build/intake/audit scripts
├─ tools/                  Control Center, Blender/PCG, validators
├─ docs/                   Architecture, design, roadmap, pass history
├─ project.control.json    Universal Project Control Center contract
├─ SubspaceTools.cmd       Root launcher
└─ SubspaceTools.ps1       Root Project Control Center
```

Generated build trees, logs, debug bundles, transactional update history, runtime-derived content, and developer-local legacy archives are intentionally excluded from normalized GitHub `main`.

## Shipyard current acceptance direction

- the ship/station is the static construction subject;
- camera navigation is independent from ship flight/physics input;
- the construction frame derives from actual assembled geometry and expands with the design;
- semantic part role is primary; S/M/L/XL is only compatibility/size metadata;
- socket placement can be authored and overridden;
- symmetry preserves handedness and socket orientation;
- shield presentation follows the hull profile at roughly one foot clearance;
- the calm shield surface should read like still water/glass;
- impacts create localized damped ripples with a slight color/brightness shift;
- renderer hot paths are cached, culled, and profiled for large kitbash scenes.

## Repository normalization policy

The Project Control Center contains a guarded GitHub-normalization workflow.

Before publishing normalized `main`, it audits the remote, prepares a clean staging tree, initializes/configures local Git, requires a new GREEN Full Quality Gate with an unchanged Git fingerprint, preserves old `main` under `archive/pre-native-normalization-<timestamp>`, and publishes with `--force-with-lease`.

See [`docs/REPOSITORY_AUTHORITY.md`](docs/REPOSITORY_AUTHORITY.md).

## License and third-party content

See `LICENSE`, `CREDITS.md`, `docs/licenses/`, and project provenance records. Third-party source/content must enter through the governed intake/supply-chain workflow and retain applicable license/provenance.

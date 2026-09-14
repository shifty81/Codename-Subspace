# Codename Subspace — Parallel Rust Rewrite Lane

This directory is the Rust implementation inside the existing Codename Subspace repository.

`../engine/` remains the certified C++ donor/current-runtime lane while Rust reaches parity.

Ember owns panel rendering/docking/splitters/floating/theme. Subspace owns game/editor
domain contracts and contributes workspaces, panels and tools through `subspace_ember_bridge`.
Forge and Cortex remain external products integrated through stable contracts.

Current Rust domains include:
- stable IDs
- schema-v2 ShipBlueprint
- module definitions/instances
- sockets and MountProfile
- appearance/decals
- Shipyard document/session/selection/history/commands
- Asset Browser query and INSTANCE/DEFINITION inspector contracts
- Activity/Jobs
- deterministic PCG request/candidate/explain contracts
- interior room/aperture topology
- HP/armor/mass/power/heat/thrust/shield summaries
- donor/parity manifests
- host-neutral Ember editor contributions

Run `PROJECT_CONTROL_CENTER.cmd` from this directory for the Rust lane.

# Codename Subspace Pass1097R3 — Pass501-506 Live Gate Dependency Closure

## Failure closed

The Pass1097R2 Full Gate reached a clean main CMake build and a clean 92/92 CTest run, then failed only in the historical `scripts/subspace_pass501_506_live_gate.ps1` standalone smoke build.

`ShipyardModuleSystem.cpp` now correctly consumes the normalized Pass1023-1097 attachment authorities:

- `WorldScaleAuthoritySystem::DefaultProfile()`
- `AuthoringStandardsSystem::DiscoverFlatSnapSurfaces(...)`

The main engine target already links both implementations, so all 92 CTest targets passed. The older standalone Pass501-506 smoke CMake fragment had not been updated to link the same two source files and therefore produced two `LNK2019` unresolved externals.

## Repair

The standalone live-authoring gate now explicitly includes and verifies:

- `engine/src/world/WorldScaleAuthoritySystem.cpp`
- `engine/src/editor/AuthoringStandardsSystem.cpp`

No runtime behavior, Shipyard geometry rules, class scaling, UI behavior, planet materialization, or historical test expectation is changed by this repair. It only restores link parity between the historical standalone smoke target and the current engine dependency graph.

## Validation

A reconstructed standalone CMake target using the exact Pass501-506 smoke source plus the repaired dependency list configured, built, linked, and executed successfully. Runtime result:

`PASS497-506 ShipyardAuthoringAuthority smoke: PASS`

The authoritative Windows PCC Full Gate remains the final certification authority.

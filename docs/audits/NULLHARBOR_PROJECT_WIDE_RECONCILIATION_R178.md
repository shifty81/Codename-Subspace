# NullHarbor / Codename Subspace Project-Wide Reconciliation — R178

## Authority

This reconciliation is anchored to `shifty81/Codename-Subspace` `main` commit `8484a081f8d60be980d3aac300ba2f1f9397cd75`, certified by `QG-20260930-143924-full-53697b5b`. Uploaded Subspace snapshots are history/preimage evidence. Original NullHarbor `SourceWork` and `NovaForge_Unified_R1` are donor sources only.

Codename Subspace remains the development/repository identity. NullHarbor is the destination product identity. This milestone normalizes internal subsystem boundaries without attempting a premature global product rename.

## Reconciliation findings

The donor audit found several generations of overlapping gameplay implementations. Mining, salvage, inventory, fleet, progression, storage, player-control and related types appear in multiple legacy trees. They are not imported as parallel systems. Proven behavior is promoted into the current C++ owners only.

The original NullHarbor donor also contains a genuine first-person/interior lane: first-person control harnesses, capsule movement, hangar walkaround, live interaction execution, ship-room state, elevator/airlock transitions, camera collision and generated FPS/interior content sidecars. NovaForge contributes useful character-state ideas but does not replace Subspace's newer derived-shell collision authority.

The live project has a second overlap: the user-facing strategic-flight/autopilot path and the older ECS `FleetCommandSystem`. R178 starts convergence by making `FleetCommandSystem` an executor-driven order authority and by adding a fleet-strategy control surface that never directly changes ship physics.

## Canonical direction

- `ControlIntentRouterSystem` translates stable physical/action state into a runtime control domain.
- `PlayerController` is the player-control façade, not a second movement simulator.
- `PlayerControlSystem` retains direct ship-flight authority.
- `ShipEmbodimentSystem` owns embodied avatar locomotion state.
- `ShipInteriorShellTraversalSystem` remains the interior collision/traversal authority.
- `FleetCommandSeatSystem` authorizes physical command-terminal/seat access.
- `FleetCommandSystem` owns durable fleet orders; domain adapters execute them.
- `FleetStrategyControlSystem` owns RTS camera/selection/group/order-request behavior.
- `ModularInteriorKitSystem` owns normalized visual-module contracts.
- `InteriorKitRegistrySystem` resolves appropriate visual kits without gameplay depending on a pack name.
- `InteriorInteractionSystem` exposes doors, airlocks, consoles, elevators, helm/fleet/mining/salvage/refinery/manufacturing/storage/medical/repair fixtures as structured interaction dispatch.

## Donor kitbash harvest

`content/interiors/nullharbor_donor_kitbash_harvest_v1.json` contains 195 unique normalized donor module records recovered from the original NullHarbor generated sidecars. Twenty records are direct Quaternius Ultimate Modular Sci-Fi mesh references with CC0 provenance, authored dimensions, 0.25 m snap data, sockets, collision proxy data and visual/runtime-review requirements. The raw third-party meshes are intentionally not vendored by this patch.

The shared interior placement contract is now three-level: 2.0 m planning cells, 1.0 m structural grid and 0.25 m fine kitbash snapping. Ship, station, hangar and derelict interiors share the same module/socket/portal vocabulary while retaining separate layout grammars.

## Explicitly retired patterns

- Using W/S/A/D as an implicit universal ship-thrust contract in every runtime view.
- Generic 15-second fleet-order completion.
- Hard-coding Quaternius (or any source kit) as the only interior implementation.
- Importing entire donor gameplay trees as competing runtime authorities.
- Treating manually maintained old pass prose as newer than Git/quality-gate evidence.

## Follow-through after R178 foundation

The next integration layer should connect the live `NativeGameApplication` to the contextual control router and command-seat session, then bind fleet order executors to current navigation/mining/salvage/combat/docking/economy systems. The following large milestone should absorb the original NullHarbor mining, salvage, processing and manufacturing depth into those existing current authorities, including physical interior workstations and strategy orders.

## Portable certification evidence

- `subspace_engine` + `subspace_stabilization_tests` built successfully in the Linux portability environment.
- Stabilization suite: **95 passed / 0 failed** including the new R178 contextual-control, fleet-strategy, kit-registry and physical-workstation tests.
- Historical/master suite: **4096 passed / 0 failed** after replacing the obsolete synthetic 15-second fleet-order expectation with executor-driven evidence.
- `nullharbor_r178_convergence.cmake`: **PASS**.
- The archived September 17 staging snapshot cannot run every later historical static gate because it lacks `docs/design/SHIPYARD_BLENDER_DCC_100_PASS_ROLLUP.md`; the live GitHub baseline is documented as containing that file. Windows project-owned PCC Full Gate remains the promotion authority after application.

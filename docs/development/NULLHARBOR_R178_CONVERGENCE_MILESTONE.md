# R178 — FPS, Universal Interiors and Strategic Command Convergence

R178 is intentionally a milestone-sized convergence pass rather than a narrow feature patch. It establishes the shared contracts needed for first-person life aboard ships/stations and a true strategy-oriented Fleet Command view without creating parallel donor architectures.

## Delivered foundation

- Context-routed gameplay intent with explicit Pilot, FirstPerson, FleetStrategy, Authoring, DockedService and Transit domains.
- Backward-compatible semantic input actions appended after historical serialized action indices.
- Expanded player-control façade and FPS locomotion state with acceleration/deceleration, sprint/crouch stance, capsule/eye-height profiles and stamina state while preserving current derived-shell collision.
- Fleet strategy camera, selection, command groups, queued order requests and formation intent with no direct ship-physics writes.
- Executor-driven fleet orders; missing executors block visibly instead of silently finishing on a timer.
- Universal interior kit registry, three-level placement grid, dimensions/provenance/hydration/collision/socket metadata and removal of hard-coded source-kit selection from exterior-to-interior linking.
- Expanded physical interior fixture vocabulary for command, navigation, mining, salvage, refinery, manufacturing, cargo/storage, medical, repair, helm, elevator, door and airlock interactions.
- Governed donor kitbash metadata harvest for later Foundry hydration and certification.

## Deliberate compatibility constraints

Historical input-action numeric indices remain stable. Existing aggregate initializers for interior modules retain their old leading field order. Existing fleet order enum values and serialized fields remain compatible; new order kinds and metadata append after historical values. Existing `InteriorInteractionSystem` overloads remain supported while the structured context/action route is added.

## Next large integration slice

1. Bind `NativeGameApplication` to the control-domain router and real physical Fleet Command seat/session.
2. Route FleetStrategy W/S/A/D to camera movement only and suppress direct thrust/fire at the live application boundary.
3. Bind real executors for Move/Approach/Orbit/Hold/Patrol/Escort/Mine/Salvage/Attack/Dock/Warp/Trade/Repair/Resupply.
4. Hydrate approved interior kit sources and materialize a Pioneer-frigate ↔ station traversal proof using current shell/portal authority.
5. Connect structured FPS workstations to the exact same mining/salvage/refinery/manufacturing/storage systems exposed by strategic management UI.
6. Continue into the mining/salvage/industry donor-harvest milestone rather than adding isolated micro-passes.

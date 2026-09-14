# Pass1163-1202 — Embodied Authoring Foundation

This tranche removes several false-authority seams that made Subspace look 3D while still behaving as a planar prototype.

## Locked behavior

- `PlayerControlSystem::Full3D` now means actual six-degree-of-freedom physics input. Forward/right/up derive from the complete ship orientation; pitch/roll/yaw apply X/Y/Z torque; dampening, soft caps and emergency braking operate in three dimensions.
- `TacticalPlanar` remains available only as an explicit constraint mode and zeros Z force plus X/Y torque.
- Semantic flight actions now include vertical thrust, pitch and roll. Existing native key paths temporarily bridge PageUp/PageDown and arrow keys until the input backend publishes the semantic axes directly.
- On-foot embodiment stores FPS yaw/pitch, moves relative to view direction, exposes canonical 1.68 m eye height, and can consume generated traversal bounds instead of being permanently tied to the prototype room clamp.
- `FirstPersonViewSystem` is the camera-pose authority for on-foot and cockpit FPS views.
- `ShipyardPlaytestSystem` establishes a non-destructive Editing → Interior FPS → Cockpit 6DOF / Remote Fleet Command test loop on the same persistent ship identity.
- Construction pointer projection no longer intersects an unrelated global Z=0 plane while the 6DOF editor camera is active. It uses a real perspective ray and the camera focus plane, preventing drag previews from vanishing when orbiting around or under an assembly.
- Shipyard's primary workspace model is now Build / Interior / Systems / Appearance / Test. Model, Character, PCG Lab, World, Dev World, Project Tools and Authoring are advanced/developer workspaces.
- The default dock contract gains a narrow Tools rail plus dedicated Interior Program, Apertures & Hangars and Play / Test panels while keeping History/Validation/Console/Forge collapsed by default.
- `ApertureHangarAuthoringSystem` establishes player-sized doors/airlocks, vehicle transport envelopes, articulated hangar doors/ramps and pressure/force-field validation.
- Shield policy now distinguishes the legacy renderer-safe close gap from the future cohesive envelope. The cohesive source must be a single closed smoothed surface, bridge micro-detail, and may reserve 2.4 m clearance over intentionally walkable exterior zones.

## Deliberately not claimed complete

- Native Win32 raw mouse motion is not yet connected to cockpit mouse-look. The 6DOF physics/action authority is now real, but the final control feel still needs the dedicated input-context pass.
- The current renderer still consumes its legacy per-module shield source. This pass establishes the cohesive envelope contract; the next renderer pass must source it from the unified baked exterior.
- The topology-first ship/station generator remains unfinished. This pass makes it possible to test that generator through reliable FPS and authoring controls rather than adding another generator layer.
- Quaternius character/interior assets are not bundled in source. They remain Vault-governed external dependencies pending certified intake and source-to-meter calibration.

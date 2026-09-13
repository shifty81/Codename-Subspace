# Pass1098-1122 — Generation Foundation Emergency Normalization

## Purpose

Stop malformed procedural output before deeper player-scale/interior/station work is built on top of it. The exact regression that motivated this pass was a Battlecruiser/L candidate with roughly 17 modules being force-scaled by about 28x until it met a class-length target, collapsing the visual language into a large flat slab.

## Authority changes

- `GENERATE` is deterministic. It no longer increments the seed.
- `REROLL SEED` remains the explicit seed-changing action.
- `GeneratorParityRequest` now exposes a stable identity shared by Shipyard, PCG diagnostics and Blender parity tooling.
- Ship generation refuses non-ship generator domains instead of silently generating a ship while the UI says `Prop / Fixture`, `Station`, etc.
- `ShipClassGenerationAuthoritySystem` measures the full transformed 3D bounds of every module rather than using unrotated Y-only extents.
- Class sizing may make only a small `0.85..1.15` final correction. Anything larger requires topology/module-family regeneration.
- Generated module instances above `1.50` absolute scale fail certification.
- Below-minimum and above-maximum class module counts are hard generation failures.
- Extremely collapsed width/length or height/length candidates fail certification.
- Invalid candidates fail closed and do not replace the currently authored ship.
- The current station radial/vertical kitbash generator is explicitly marked `LEGACY_EXTERIOR_STACKING_PREVIEW`; it remains available for compatibility rendering while the replacement interior/service-program-first station generator is built.

## Deliberate limitation

This pass does **not** pretend the old `BuildShowcaseRecipes/BuildOne` path has become the final generator. It is retained temporarily as candidate synthesis behind the new fail-closed integrity boundary. Large classes may therefore reject generation until player-scale calibration and class-driven topology synthesis land. Rejection is preferable to promoting malformed geometry.

## Next dependency order

1. Canonical player model calibration and locked meter-scale profile.
2. Governed interior kitbash intake and physical classification.
3. Door/window/airlock/stair/ladder/elevator aperture/traversal definitions.
4. Rover/mech/hoverbike/shuttle transport envelopes and hangar rules.
5. Capability/interior-first ship topology generation.
6. Capability/interior-first station generation.
7. PCG proof/exemplar promotion and regression fixtures.

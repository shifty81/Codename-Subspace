# Shipyard hollow hull — Slice 1: derived geometric surfaces

**Input authority:** certified Subspace `02615ebf892fd95bd5d8d356ed7651d9800e20ed` (Slice 1A-R2). This patch does not integrate Nullharbor/NovaForge donor engines or modify PCC.

## Production code delivered

The existing `ShipInteriorCarvingSystem` continues to own walkable/excluded volumes and logical portals. The new `ShipInteriorDerivedShellSystem::Build` consumes precisely that carve plan; `ShipInteriorLayoutSystem::Plan` publishes the derived result as `InteriorLayoutPlan::shell`. The source kitbash geometry and the ship recipe are **not mutated**.

For currently supported *axis-aligned* hull envelopes it:

- Generates one set of interior ceiling, floor, and wall quads from the inset volumes.
- Computes a rectangular surface union. Overlapping/identical module volumes suppress buried faces and the corresponding static-collision quads; partially obscured faces retain only their exterior strips.
- Cuts matching human-scale openings in both facing hull walls for physically close, walkable portals. Closes the inset gap with four passage liner surfaces. Sealed/nonwalkable portals never cut holes.
- Excludes wings, exposed equipment, weapon and engine roles because the upstream carve did not admit them as walkable volumes.
- Fails **separately** when a rotation-aware mesh is required, a carve is disconnected, a passage is too far apart, or there is inadequate doorway clearance. No fake collision or invented connection is returned in those cases; the existing logical carve's validity is not silently rewritten.
- Uses one `InteriorShellSurface` quad representation with `blocksMovement` and `pressureBoundary`, intended for both visual and collision consumers. Source ownership is retained via module indices for partial regeneration later.

## Exact boundary of this pass

This is a **geometry-data implementation**, not a user-visible hollowing feature yet. The renderer, physics, navigation, damage, atmosphere, interior FPS, cutaway controls, asset publishing, and derived-output caching have **not** been connected to this geometry. Do not present `shell.ready` as gameplay certification. It means only that the geometric surface plan is safe to consume. In particular, do not leave original box colliders in place after a future shell consumer installs the derived collider set.

Rotations other than multiples of 180 degrees are rejected instead of producing incorrect axis-aligned hulls. Imported meshes and curved/concave geometry need a robust mesh/voxel boolean backend with stable source-to-derived provenance. The original exterior appearance remains untouched in this pass. Clearance is based on the existing authored inset envelope, not exact imported mesh interiors. Capped pressure and dynamic door collision need their own authored-state pipeline.

## Verified local test

`SubspaceDerivedHollowShellTests` uses real `ShipInteriorCarvingSystem` and `ShipInteriorLayoutSystem` production code, testing isolated hull six faces, coincident-volume deduplication, two inset hulls with doorway and four gap liners, positive/negative colliders at the aperture, partial/full cavity overlap, detached cavities, excessive gaps, excluded wings, rotation fail-closed, and published layout handoff.

Focused Linux test: 63 assertions PASS. `g++ -fsyntax-only` PASS for both new geometry and changed layout sources. A clean portable *full-engine* build in the provided rollup is blocked by missing `developer/build/DeveloperBuildPreflight.h` in that rollup, an unrelated pre-existing source-archive gap; Windows PCC Full Gate remains authoritative.

## Next connected implementation, in order

1. Install geometry surfaces as one real interior render + collision product, never dual colliders. Support a cutaway preview and redraw only dirty affected source modules.
2. Support rotation-aware non-box imported meshes and controlled union/subtraction, including panel IDs, source material preservation, and deterministic output caches.
3. Bridge portal nav traversability to the physical openings; build doors and articulated hinge children, pressure sealing and damage interaction.
4. Render the authored, compiled hull in first-person, with collision and direct editor TEST/stop return; save/reopen and verify source and derived recreation.

**Acceptance end-state:** two distinct real hull assets attach; inner obstructing surfaces are absent in both visual and collision meshes, exterior silhouette is preserved, a player crosses the actual generated passage, and detach/refit/restart regenerates deterministically. Slice 1 alone does NOT satisfy this end-state.

# Subspace Studio S14R2 — rotation-field repair and full-ship authoring gap audit

## Baseline and scope

GitHub main checked at `139279ca32b78cfc71f990823fc72042ff2c4883` (S12 GREEN). S13/S14 cumulative and S14R1 Win32 `near` macro repair are local pending layers, **not** in that GitHub commit. This S14R2 overlay depends on S14R1 installed, retains the previous GUI and close protection, and changes ONLY the Studio physical-rotation-axis adapter plus the focused regression test. The absence of a Windows Full Gate or interactive user acceptance in this audit must not be described as GREEN.

## Finding A — rotation defect (fixed in S14R2)

The S14 click handler set `ShipyardTransformConstraint::Y` when picking physical gizmo Y, and `::Z` for physical gizmo Z. However, the rotation delta is packed as `{pitch, yaw, roll}` while physical Y rotates roll (delta.z) and physical Z rotates yaw (delta.y). `ShipyardBuilderSystem::ApplyTransformConstraint` retains the selected *packed component*, so physical Y's `delta.z` was masked out by Y, and physical Z's `delta.y` was masked out by Z. `ShipyardTransformSystem::Rotate` never received those rotations. The existing `StudioGizmoMath::RotationFieldAxis` identity implementation was an intended adapter that did not adapt anything.

S14R2 maps X -> packed X (pitch), Y -> packed Z (roll), Z -> packed Y (yaw) **only for Rotate**, before `SetTransformConstraint`. Move and Scale retain physical axis-to-field identity. The edit is Studio-only and preserves the game's shared platform and transform implementations. Test verifies all three deltas survive the same masks that previously erased Y/Z and that cross-axes remain excluded. The existing Windows `near`-macro regression remains in the test target.

Other rotation issues remain: the shared `ShipyardTransformSystem::Rotate` snaps all three Euler components, potentially changing untouched axes on a one-axis edit; handling of local rotation axes when object already has rotation, true 3D ring/picking, numerical input, consistent Euler decomposition, and mouse/viewport zoom must be independently implemented and tested before calling all rotation cases certified. The screen-space overlay arc is not a complete 3D rotation manipulator.

## Finding B — asset drag is snap-prioritized (not fixed)

`ShipyardDragDropSystem::Begin` enumerates existing catalog socket pairs and `SelectBest` chooses the first candidate by default. `ShipyardBuilderSystem::UpdateCatalogDrag` replaces the cursor ghost position with a candidate whenever the cursor lies within a module-size-dependent snap radius. Outside that radius the current system permits free placement, but the cursor is converted using `NativeBattlefieldRenderer::ScreenToWorld`'s gameplay plane rather than arbitrary ship surfaces or a persistent depth plane. A free-placed module creates no attachment edge, and the existing certification graph can reject the resulting assembly. The observed middle-of-ship jump is consistent with that policy; no claim is made that all reported visual manifestations are confirmed without a fresh runtime capture.

Required: Studio-default free positioning with cursor-to-grab offset and stable depth; plane/surface modes; separate deliberate snap/magnet action or modifier with visible preview and threshold; placed module remains at exactly the released authoring transform. A later explicit `Attach` operation, not incidental proximity, computes and previews a connection. Floating modules remain named *drafts* until connected. Do not bypass the structural certification gate merely to allow freedom.

## Finding C — intersection-derived attachment (not fixed)

A generated socket is meaningful only after choosing two mesh surfaces, finding a supported contact patch in fully transformed world space, computing local contact points and orthonormal direction/up frames for BOTH sides, checking intersection/tolerance/clearance/collision and categorizing structure vs power vs corridor vs utility. A line or point intersection alone is ambiguous; intersecting models do not always yield a walkable or load-bearing interface. Preview candidates and require author acceptance, or an explicitly enabled auto-attach-on-save option that runs preview/validation before final save.

Persist connection IDs, per-instance sockets, source-asset IDs, local frames, parent/child module instance IDs, socket classification, surface region, authored offsets, version and undo. Generated instance sockets must not silently mutate a shared source module definition. Blueprint save must not manufacture unvalidated edges or certify a physically disconnected ship.

## Finding D — hull/interior geometry (existing approximation, not finished)

`ShipModuleInteriorLinkSystem` binds semantic walkable modules and exterior attachment metadata to portals. `ShipInteriorAuthoringSystem` generates grid-room/deck plans. `ShipInteriorCarvingSystem` insets module half-extents by shell thickness, excludes certain categories (wing, weapon, detail, drives), and joins nearby bounds when there is no explicit edge; it validates BFS graph connectivity. This is useful initial machinery but is NOT watertight mesh Booleans or collision-safe subtraction. Its overlap check is axis-aligned in world space and can propose an aperture where two actual surfaces do not meet.

Treat the hull as exterior pressure-envelope authority and interior as a derived, reviewable editable scene. Author explicit habitable masks/volumes and machinery cutouts, subtract occluders and intruding meshes using validated Boolean/voxel/SDF or equivalent geometry, retain load-bearing thickness and pressure seals, generate portals only across proven aperture geometry, validate player height/doors/navigation/egress. Preserve original exterior model and source lineage: never destructively collapse runtime geometry solely to hide clipping. Allow manual overrides with inspector warnings.

## Finding E — detach greebles and faces (feasible, not implemented)

Existing `ShipyardModelingSystem` handles a primitive recipe and primitive baking; naming Vertex/Edge/Face selection modes is not the same as actually selecting triangles in imported topology. Existing `DetachModule` works on an assembled whole module, not part of its mesh. A practical extraction hierarchy is: separate source mesh nodes or material groups (least expensive); separate connected topology islands; then face/edge paint selection and `Extract Selection` with stable triangle-to-vertex remap, source-space to new pivot transform, normals/tangents/UV/material preservation, optional boundary capping, new collision/LOD, socket policy and editable lineage. Generate a *derived* reusable canonical asset and catalog item; leave the original intact unless user explicitly requests destructive split and has recovery. Record import provenance/license and prohibit treating arbitrary third-party source material as original.

## Finding F — interior kit / characters (reference metadata, intake not certified)

The repo `ShipModuleInteriorLinkSystem::InferBinding` sets `preferredInteriorKitId = "quaternius.ultimate_modular_scifi.2021"`. This is a reference identity, NOT proof the 3D mesh kit has been downloaded, converted, imported, cataloged or rendered in Studio. The named Quaternius 2021 pack's published source formats should be verified on disk before choosing the converter and the official source/license pinned by URL/checksum. Add pack import/preflight that lists required meshes, versions, license and missing files; normalize geometry/materials/pivots/room-grid/portals to one canonical catalog; create explicit scene templates instead of spawning placeholder boxes. Do not vendor megabytes of external libraries into the source patch.

`CharacterAnimationLibrarySystem` declares a rig/animation profile, clip categorizer, and retarget plan. This does NOT establish an imported rig, playable character mesh, animation playback in Studio, or navigable interior. Character lane must verify actual source rig, scale/retarget, compatible clips, floor/portal/nav volumes and an interactive walk-through from cockpit to another room in saved blueprint, including game reload. Treat these as separate asset provenance and runtime acceptance gates.

## Finding G — document completeness (not fixed)

Existing `.subspace_ship` serialization saves assembly recipe and appearance, but S12/S13 documentation explicitly excludes editable model/interior drafts and some independent overrides. A complete Studio project/session schema must version and transactionally persist every authored dependency, selected source assets, generated per-instance sockets and attachment graph, interior geometry, kit selections, rig references, paint and undo/recovery boundary. Complete-save certification must reload a fresh process and compare exact transforms, graph, geometry/material and interior walkability against pre-save state.

## Two-audit acceptance before a full ship milestone

Pass 1 — code/math: test X/Y/Z Move, Rotate and Scale with snap off/on, Shift precision, rotated source module, local/global axes, mirrored geometry, opposite-angle wrap, camera zoom/orbit, one undo/redo transaction and stable save/reload; verify no game regression, source fingerprint and PCC invocation. Reject unsupported operations visibly instead of faking success.

Pass 2 — author workflow and assets: drag arbitrary wing and detail from browser to cursor in 3D, intentionally attach to a chosen surface, preview/generated sockets, detach a genuine greeble and publish a reusable derivative, close/save/reopen with edits intact, hydrate/import licensed interior kit, generate hull-bounded multiroom interior with door, preview actual character in first person, build/game load and run the universal PCC Full Gate. Missing source or unvalidated Boolean means this pass FAIL/PENDING, never silently GREEN.

## Evidence / code paths at verified S12 baseline

- `engine/src/studio/StudioApplication.cpp` and S14 `engine/include/studio/StudioGizmoMath.h` from S14 staging: physical axis/rotation delta mismatch (S14R2 fixes).
- `engine/src/ship_editor/ShipyardTransformSystem.cpp`: physical rotation field order, snapping.
- `engine/src/ship_editor/ShipyardDragDropSystem.cpp` and `ShipyardBuilderSystem.cpp`: snap candidate priority, free draft attachment.
- `engine/src/interior/ShipModuleInteriorLinkSystem.cpp`, `ShipInteriorAuthoringSystem.cpp`, `ShipInteriorCarvingSystem.cpp`: inferred bindings/approximate volume carver.
- `engine/src/modeling/ShipyardModelingSystem.cpp`, `engine/src/character/CharacterAnimationLibrarySystem.cpp`: primitives and metadata vs topology/character runtime.

Do not claim a full Windows PCC gate, successful final assembly, imported 2021 kit, exact mesh carve, or universal transform parity from this S14R2 mathematical repair.

# Subspace Studio S15B — true 3D inspection orbit and readable measurements

## Installation and source authority

Incremental `forge.patch.v1` repository-root drop based on exact certified
`shifty81/Codename-Subspace@0de248a10a7834ae8b00eb5b8d6c6b5f99e6688a`
(S13–S15A already committed). This patch **replaces** the previous undelivered
S15B camera/readout candidate; do not install both. Not a complete source rollup.

## Actual camera implementation

- Studio opts into native RMB-orbit / MMB-pan mouse routing via
  `SetEditorNavigationMode(false)`; no modification to the game or shared
  NativeWindow source. Win32's existing drag accumulator distinguishes a short
  RMB click from a >5px right-button drag. Context popup routing is NOT yet
  present; no fake popup or success claim.
- The initial S15B candidate used `StrategicCamera::OrbitVisual`, which ONLY
  rotates yaw and clamps a faux-3D tilt to [0,1]. It cannot orbit beneath a
  ship, so it does **not** meet the 3D requirement and has been superseded.
- Studio now uses the EXISTING `ConstructionEditorCameraSystem` to own a real
  3D eye, target/pivot, orbit distance, yaw/pitch, and roll. Each frame these
  are forwarded into the existing `StrategicCamera::SetEditorView` projection
  consumed by rendering, picking, axis gizmos, and screen-to-editor-plane rays.
  No ship transform or blueprint changes when the camera moves.
- Hold RMB and drag horizontally for continuous 360-degree azimuth; drag
  vertically to inspect above and below. The existing construction camera
  deliberately clamps latitude/pitch to -89..+89 degrees to avoid a singular
  world-up perspective basis at the exact two poles. Thus full 360 azimuth
  and near-total spherical inspection are supported, but continuous end-over-
  end *vertical* pitch crossing exactly through poles is NOT supported yet.
- Shift+RMB horizontal drag rolls the view around its own line of sight with
  wrapped 360-degree roll while retaining its orbit pivot (shared free-fly
  Roll() would otherwise incorrectly detach the pivot). MMB drag truck/pedestal
  translates eye and pivot together in 3D. Wheel dollies along the eye/pivot
  ray; F reframes the assembly without resetting orientation. These are
  camera-space actions, NOT mesh rotation or changes to the ship's orientation.
- RMB navigation is still native-client wide, including over floating panels.
  Precise viewport-only capture and native context-popup ownership require a
  later scoped pointer-intake repair. The current focus is 3D orbital math and
  its shared renderer/picking authority; do not claim panel-aware input yet.

## Readability / dimensional scope

- HUD labels POS, ROT, SCL and DIM replace P/R/S/D; physical coordinate and
  nominal dimension units display meters; scale displays percent and rotation
  degrees. Dimension labels are W/L/H rather than false XYZ labels.
- `StudioMeasurementFormat.h` owns compact label/axis/unit semantics and rejects
  invalid/out-of-display-range values instead of displaying misleading clipped
  figures. Real authoritative transforms are never rounded or clipped by UI.
- DIM remains NOMINAL LOCAL catalog bounds; not exact mesh geometry, rotated
  world bounds, carved hull, collision clearance or generated interior space.
  Editable numerical inspector, mesh-silhouette highlight and free asset
  placement are separate unfinished milestones.

## Local acceptance before GREEN / Git promotion

1. On clean committed 0de248a repository root, drop only this S15B `.patch` and
   approve via the project-owned PCC. Never force a baseline mismatch.
2. Run fresh Windows Full Quality Gate. Confirm MSVC Studio/game build, current
   CTest suite including Studio camera navigation and measurement format,
   Studio smoke and unchanged game smoke; require a new fingerprint.
3. Orbit RMB horizontally past 360 degrees: no stoppage or jumps. Drag up/down
   to inspect above and underside; expect a ±89-degree vertical pole guard.
   Shift+RMB must roll view while selected model stays fixed. Release after
   >5px drag: no click; short RMB click: no orbit, popup not yet implemented.
4. MMB pans in camera plane; wheel dolly remains stable after orbit/pan; F
   reframes without camera angle snap. Test greebling/wing from above/below:
   gizmos and object picking still line up with the actual projected mesh.
5. Verify HUD readability POS/ROT/SCL/DIM, W/L/H, meter/degree/percent units;
   move/rotate/scale independently, undo/redo and save/reopen blueprint.
6. Verify title-bar unsaved-work guard, Studio smoke and the game's controls.
   Floating-panel RMB capture remains a known follow-up; do not certify it.

## Portable evidence limitations

GCC and Clang ran focused C++17 -Wall -Wextra -Werror tests for actual
construction orbit/pan/dolly/projection/roll, readout, gizmo and close policy.
Construction camera source in portable harness comes from Sep17 archive, whose
relevant methods and signatures match the reviewed current branch, but archive
is not the full authoritative GitHub tree. Neither portable tests nor a patch
integrity audit replace an actual MSVC/Windows full gate or interactive visual
inspection. Full StudioApplication native compilation has not run here.

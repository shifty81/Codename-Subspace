# Shipyard G1 — module XYZ handle manipulation

**Base authority:** `cf0bf03aa553ec3ae00a4965572c70085f2664ba` plus the previously delivered Asset Visibility Recovery and Pass1338 gate repair. This is an incremental project-owned PCC patch; no old Hollow Hull or GUI payloads are re-applied.

## Implemented

- The existing selected-module XYZ display is upgraded to projected, pointer-selectable handles at the selected module's rendered pivot. Renderer and both standalone/runtime pointer paths consume `EditorGizmoSystem::BuildModule`, `Pick` and `DragPixels`, avoiding separate screen coordinates for draw versus clicks. X is red, Y green and Z blue.
- With Move, drag one axis along its projected screen direction. The View/Ship/Local coordinate space applies to translation: View uses camera-aligned planar directions, Ship uses assembly coordinates, and Local rotates horizontal axes by the module's authored yaw. Z remains vertical. Move remains constrained to the selected handle.
- With Scale, drag the matching axis to adjust only that dimension. With Rotate, drag the corresponding axis handle to alter the matching authored Euler component. These initial rotation handles are axis-endpoint selectors, **not** Blender-like circular rotation rings around the object.
- Gizmo picks are rejected over panel backgrounds, including floating/docked overlays. They take precedence over generic mesh picking only inside the viewport. Selection-only and Socket inspector modes do not expose a misleading module transform handle. An untouched gizmo click cancels rather than committing an undo entry.
- The existing Builder transform transaction owns the drag, including snapped/fine movement, reversible preview, commit and undo. Absolute pointer displacement is applied from the original transform each update so tiny snapped deltas accumulate instead of getting stranded by rounding. Escape cancels live handle motion in standalone and in-game workspaces.
- Model Select no longer forcibly exits the Model workspace. The primitive type cycle now derives its count from the 16-value enum rather than cycling to a nonexistent 17th type.

## Explicit remaining work

- This first pass is **only for selected assembled ship modules**. It does not render, pick or transform Model-workspace primitives, sockets, vertices, faces or door parts. Their existing unconnected buttons must not be presented as completed modeling tools.
- No 2D sketch/extrude, solid/hollow/surface conversion, exact mesh boolean, articulated hinge, animation timeline, blueprint model save/reopen or functional door cut is delivered.
- View-space axes are planar and are not free 3D camera-plane transforms. Local orientation currently uses module yaw, not a full three-axis rotation matrix. Rotations operate on the existing Euler properties; future G2 will supply proper circular handles.
- The existing click-and-drag-anywhere module path remains for compatibility. It is not a substitute for selecting the gizmo axis.

## Acceptance

1. Apply the two earlier Asset Visibility patches first (or verify they are already approved), then this `.patch` intact via `SubspaceTools.cmd`. Do not extract it or overwrite the repo manually.
2. Full Quality Gate must pass on Windows. On the supported native desktop build, open Shipyard, select a placed module, choose Move, and drag X/Y/Z arrows one by one. Verify exactly the corresponding axis moves and the gizmo follows it.
3. Repeat with Rotate and Scale, on Ship/View/Local movement, camera orbit, zoom and resized windows. Test both standalone Dev Studio and in-game Shipyard. Verify panel overlays receive clicks rather than leaking through to gizmos, and no module is added by clicking a handle.
4. Click/release an axis without moving; Escape during a drag; undo/redo after committing a move. Check catalog Assets and RESET UI remain functional and layouts persist.
5. If the Windows gate or hands-on checks fail, return its current debug bundle and do **not** commit/push as a completed gizmo release.

Focused local verification: standalone C++ gizmo regression (24 assertions), C++ syntax on changed builder/gizmo sources, static shared-source gate and CMake configure. The available September source ZIP lacks `engine/include/ship_editor/ShipyardOverlayLayoutStore.h`, so a full portable engine compilation and real Windows visual acceptance are not claimed here.

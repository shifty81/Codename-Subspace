# Hollow Shell Slice 2 — native visual and traversal consumers

**Requires:** `02615eb` + Hollow Shell Slice 1 applied unchanged. Do not install on modified overlapping source without rebase. The ship assembly remains authoritative; no donor-engine merge.

- Because the current on-foot camera is still strategic/top-down, ceiling quads are omitted **from drawing only**, not from the shared collider. Proper editable cutaway toggles and eye-level FPS remain future work.
- Native renderer's on-foot interior now consumes the **same** `InteriorDerivedShell::surfaces` used by `ShipInteriorShellTraversalSystem` for collision. Remove synthetic starter-deck geometry, and fail visibly (HUD) if shell cannot be built. Source meshes remain intact.
- Cache the production `InteriorLayoutPlan` once per player-ship rebuild; renderer receives an immutable frame pointer to the resulting shell rather than rebuilding every frame. Refit and captured-ship paths invalidate/rebuild the plan.
- Horizontal movement uses generated wall rectangles and bounded microsteps with axis sliding. Inset-generated openings remove the wall AND collider; gap liner quads provide real passage boundaries. Standing floor is derived from the carved module, not a fixed default bounding box. The engine retains on-foot view's existing strategic camera: this pass is **not first-person FPS**, no stairs, no dynamic doors, no imported-mesh booleans.
- Do not generate colliders for unready hulls: avatar traversal is disabled with an explicit HUD warning. Rotation-aware meshes need a future backend.
- Scene rendering uses existing ship position/yaw and explicit .72 ship-local-to-world scale matching camera follow. The on-foot avatar remains a top-down marker. Exterior ship geometry/physics unchanged.

## Limits / no false completion

Cavity geometry is derived from **module envelope boxes**, not exact imported hull mesh boolean subtraction. Original exterior module mesh overlaps are NOT removed or masked in exterior drawing. Existing gameplay strategic camera remains top-down, not eye-level. No GUI cutaway control, publishing, save/reopen validation, atmosphere, multiplayer/server collision authority or actual FPS test is delivered here. Unready/failing cavities are not playable.

## Acceptance on Windows

Run Full Gate, then on a supported axis-aligned connected ship enter on-foot interior via I: the prior fake starter-deck boxes are gone and the new authored cavities render. Walk against a generated wall and through the existing carved passage. Check ship rebuild/capture changes geometry; invalid rotated hull explicitly shows NOT READY and disables walking. Verify 1280x768 and 1920x1080. Do not commit until both gate and visual checks pass.

**Door threshold correction:** The previous geometric builder centered portal openings on each face's vertical midpoint, potentially leaving an invisible knee-high barrier when the player's feet are on the carved floor. Slice 2 anchors the lower opening edge to the shared deck floor, requires compatible floor levels, and rejects unimplemented vertical stair/elevator transitions. These are real geometry changes in the common surface source, exercised by two-way passage tests.

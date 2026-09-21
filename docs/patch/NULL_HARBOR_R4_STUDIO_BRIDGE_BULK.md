# Null Harbor R4 — Studio gizmo/dock/interior convergence (incremental bulk patch)

Patch ID: `null-harbor-r4-studio-dock-gizmo-interior-bridge-20260921`
Prerequisite: R3R2 All-Angle Gizmo Repair has already been applied. **Do not reapply R3, R3R1, or R3R2.** Target Git lineage: `bd59a39d7a7339a9d61f479941dc4ba01f743ea2` with uncommitted R3R2 source preimages. All modified source has SHA-256 preconditions; mismatch fails closed. Keep the package unextracted, drop into root, explicitly approve through PCC; Full Gate never applies it automatically.

## Implemented

1. **Dock-aware gizmo reflow.** The all-angle policy is now the single owner of the 2D handle geometry. The standalone `VisibleGizmoSnapshot` passes the existing floating-panel occlusion query into the policy, which tries alternate handle directions with 12 shaft and four marker samples and preserves axis identity/spacing. The existing panel compositor is materialized **once per gizmo snapshot**, not hundreds of times through `CoversFloatingPanel` during each reflow probe. It preserves the pointer system's existing floating/docked-UI occlusion semantics. The old post-projection `valid=false` loop has been removed. Rendering and pointer-down use the same reflowed snapshot; no new gizmo renderer or pick path.
2. **Interior bridge.** Standalone `StudioApplication` now supplies `frame.editorInteriorShell` from the game's existing `ShipInteriorLayoutSystem::Plan` whenever a non-exterior preview is active. The preview is derived from the live recipe and catalog; it never mutates the authored document. A deterministic cache signature invalidates after assembly pose, scale, rotation, mirror, socket, semantic, role, or attachment edits; irrelevant unused catalog records do not force rebuilds.
3. **Visible interior entry.** Entering the Interior workspace from Exterior cycles the **existing** viewport view command to X-Ray, exposing the existing ghost-hull/shell preview instead of displaying the opaque exterior unchanged. An explicitly selected Cutaway or Interior-only inspection choice is preserved.
4. **Brand safety.** Only standalone Studio's window title changes to `Null Harbor Studio`. PCC project ID, save formats, C++ namespace, Git URL, build targets, and legacy lineage are unchanged until compatibility migration is certified.
5. **Regression certification.** New CMake/CTest test covers dock occlusion/reflow and interior-preview cache invalidation. R3/R3R2 CMake gate now checks R4's actual bridge wiring and preserved render-state guard. Python checker is normalized to the R3R2 shared `PopulateHandles` helper instead of requiring two obsolete duplicate `Direction` calls. This also fixes a latent false-RED Python checker.

## Limits — not included

This is not a full GUI normalization, complete playable interior, hull boolean mesh editing, persistent authored interior document, procedural candidate browser, shader/PBR rewrite, or completed Null Harbor rename. Existing X-Ray mode uses the current renderer's wireframe/translucency presentation; a separate renderer pass is required for a properly filled, profile-exact opaque-to-ghost hull and full runtime parity. If the existing shell is not `ready` (e.g., a module is not a safe axis-aligned walkable cavity), do not fabricate geometry. The renderer's existing diagnostic should remain truthful.

## Local certification performed in preparation

`studio_gizmo_projection_policy_tests.cpp` compiled and passed with 4,940 angle/border cases; new `studio_r4_studio_bridge_tests.cpp` compiled and passed against supplied source headers; `studio_foundation_r3_authority.cmake` and `test_studio_foundation_r3.py` passed in an isolated layered fixture. Manifest archive/hash and staged preimage validation performed during packaging. **Full Windows CMake/CTest/WGL rendering and actual UI behavior have not been run here.**

## Windows acceptance sequence

1. PCC Patch status: verify R3R2 applied; drop this R4 `.patch` unextracted; approve through PCC. Refuse any preimage mismatch, don't force merge or hand-extract.
2. Run Full Quality Gate; if failed, send current debug bundle. No commit/push on RED.
3. In standalone Studio add an asymmetric box in Model. Test X/Y/Z visibility and picking from six faces, straight-on/end-on views, zoomed close, corners and resized window. Move floating panels across the **shaft** of a handle and verify it reflows to an accessible position; moving a dock over the pivot should prevent click-through.
4. Verify Move, Rotate (all three independent axes), Scale, undo/redo and Save/Reload. Open Assembly with a placed module and repeat.
5. Switch to Interior. From the default Exterior view, confirm X-Ray becomes active and that the shell appears **only** if the existing layout is ready. Change a module's position, size, angle, attachment or socket; verify regenerated shell matches the assembly. Inspect invalid layouts and verify warning rather than fake rooms. Check other viewport modes continue working.
6. Test Studio and game rendering before/after: widget must never inherit a shader, no blank UI, no material regression. Confirm branded window title; do not rename GitHub yet.

## Follow-on R5 architectural boundary

Consolidate dock compositor/layout/input into one immutable per-frame layout geometry owner, implement independent HUD placement without post-draw overlap guessing, then true filled translucent hull/cutaway renderer and complete interior document publication. Candidate generation is next after durable document snapshots; shaders after GPU test harness. Do not create parallel systems while doing so.

# Shipyard overlay Slice 1A-R2: default-anchor collision repair

Input: installed Slice 1A and R1 working tree on certified `3cfe9676b80ba283682d99a7604c3403ce965fb2` Git baseline. The debug bundle `Subspace_DebugBundle_20260917-071548.zip` reports 103/104 passing CTests; `SubspacePass506R7UniversalBuildControlsTests` fails on 1280x768: Asset Browser collapse button `-` at `[1167,525,24,24]` overlaps Properties' `SHIELD` at `[1106.5,506.9,160.5,24.3]`.

## Cause

Slice 1A made every dock tool an overlay while keeping the bottom shelf's full usable width even when the right-bottom Properties dock was occupied. At common compact viewport heights their default rectangles overlap. R1 aligned the workspace and viewport header but did not address colliding overlay anchors. Disabling the historical overlap assertion would hide a real UI problem.

## Fix and invariant

The bottom Asset Browser ends 6px before a *currently occupied* right-bottom dock anchor. The 3D canvas stays full width behind both. When the right-bottom panel hides or floats, its anchor releases that space and the shelf expands; redocking restores the separation. The left tool rail remains movable/dockable. Floating panels intentionally can overlap, with existing compositor z-order/occlusion handling; this fix addresses **only default docked placements**.

Extended the historical 1280x768 overlap test and added 1120x740, 1280x768, 1653x930, 1852x797, 1920x1080 invariant checks plus float/redock round trips to the overlay regressions. No tests were weakened, no PCC, game systems or donor source was modified.

## Release gate

Apply as an unextracted `.patch` at the repository root through the project-owned PCC. Run Full Gate and visual test at 1280x768 and the normal monitor resolution; check the Asset Browser's right edge does not cover Properties, and floating Properties releases shelf width without resizing the canvas. Commit/push only after Windows GREEN and visual acceptance. This repair is not FPS gameplay integration or full GUI normalization.

# PASS1508R2 — Shipyard floating panels: visual containment repair

**Baseline Git:** `a3221000ba127bc1969aa8f1b220ea419dd1d7c1` (PASS1507 certified); requires PASS1508 and PASS1508R1 already applied in the working tree. This is a narrow repair, not a replacement source rollup.

Reported rendered failure: after dragging Properties and Outliner into floating positions, Properties was empty while its actions and selected-object summary were drawn under Outliner; multiple thin dividers continued across the entire viewport from the detached panel edges. The visible panel windows moved, but their child controls and renderer still used old shared right-rail coordinates.

Fixes:

- Properties controls and selected-object header use the Properties rectangle, independently of Outliner position.
- Outliner/Properties panel dividers stop at their own rectangle boundaries.
- Local scissor clipping prevents text and validation from painting outside Properties, and preserves pre-existing OpenGL scissor state.
- Floating backgrounds repaint after viewport-specific construction overlays so viewport lines never draw on top of a detached panel.
- Controls that cannot fit inside a resized panel are not rendered or clickable beyond its edges; their full functionality remains available when the panel is resized/docked.
- Undocking restores a panel's usable preferred dimensions rather than shrinking it to the narrow dock leaf.
- Empty floating-panel bodies block accidental ship selection through the panel.
- A new automatically discovered static gate protects these layout invariants; extended existing CTest covers pointer geometry and viewport occlusion.

Not yet delivered: full panel-local scrolling, title-bar docking ghost/preview, arbitrary split layout creation, persistent floating z-order and true detached OS windows. This patch addresses the screenshot defect without claiming those features.

Apply the ZIP-container `.patch` directly to the project root; do **not** extract. The PCC prompts for approval; then run Full Gate and **Run & play** to inspect floating panel contents and separators. Do not commit until both automated and visual checks pass. If any modified source file has changed after PASS1508R1, stop and rebase rather than overlaying an out-of-date file.

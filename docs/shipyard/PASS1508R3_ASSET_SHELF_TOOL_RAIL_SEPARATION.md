# PASS1508R3 — Independent Asset Browser / fixed tool rail

**Input state:** PASS1508R2 visual containment installed on certified Git baseline
`a3221000ba127bc1969aa8f1b220ea419dd1d7c1`.

## Root cause reproduced from source

The previous default dock tree split the **entire window** vertically before
reserving the rail, so the Asset Browser bottom leaf spanned beneath the rail.
The renderer also computed `viewportBottom` from `assetShelfY`, even after the
shelf floated. As the shelf moved, the tool rail background/divider changed
height, creating the apparent physical connection in the reported screenshot.

## Changes

- Fixed/nonfloatable tool rail becomes its own root-column leaf (`tool_left`).
- `content` (not the rail) owns `upper` (3D View + side stack) and `bottom`
  (Asset Browser and other bottom tabs) in separate branches.
- Tool rail draws using its OWN rectangle height, never the Asset Browser Y.
- Viewport bounds come from the viewport leaf, not floating sidebar/shelf Y.
- Real default-workspace CTest exercises initial geometry, float/move/drop onto
  rail (rejected), redock into content bottom, and dock ownership validation.
- A fail-closed static gate prevents the former geometry coupling returning.

## Limits and acceptance

This fixes screen-space layout ownership. The tool rail remains represented
inside the reusable dock workspace as a fixed nonfloatable shell leaf, rather
than introducing an incompatible second layout store. If all bottom panels
are floated, their original bottom dock leaf remains reserved as a drop target;
automatic reclamation of that space is intentionally deferred to avoid a
mismatch between live 3D camera bounds, renderer, and pointer drop hit tests.

After Full Gate, float Asset Browser and move it vertically: the tool rail
must not change height or position. Redock Asset Browser into the bottom region:
its left edge must start after the rail. Outliner/Properties must remain
independent. Test at both maximized and a smaller resizable window.

This is NOT Windows visual certification. If local PASS1508R2 source differs
from the supplied R2 patch, do not overwrite it with this handoff.

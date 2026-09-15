# Pass1336 — Historical Visible-Gate Reconciliation

The Pass1335 implementation itself compiled and 95/96 CTests passed.

The only failure was `SubspaceProjectOpsStaticCertification`, caused by the historical
`pass1268_1292_visible_professional_shipyard_cutover.cmake` gate still requiring the
exact duplicate `OUTLINER / SHIP HIERARCHY` and
`PROPERTIES / INSTANCE + DEFINITION` controls that Pass1335 intentionally removed.

This pass updates that historical certification to preserve its real intent:

- the professional Shipyard workspace/tool/search/generator surface still exists;
- renderer-owned Outliner/Properties chrome still exists;
- duplicate BuildControls header projections must remain absent;
- New Seed + Generate still composes reroll + the canonical generator;
- compatibility enum ordinals remain unchanged.

No runtime, ship, PCG, transform, persistence, rendering, or editor behavior is changed.
This is a certification-only reconciliation.

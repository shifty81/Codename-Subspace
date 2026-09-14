# Pass1268-1292 R2 — Legacy Control Projection Repair

The R1 build compiled successfully, but CTest exposed five historical Shipyard tests.

## Root causes

1. The invisible migration controls were placed left-to-right using the previous control width.
   A later, wider legacy control could therefore overlap the previous off-screen rectangle.
   Historical overlap tests inspect every `BuildControls()` rectangle, including compatibility
   controls, so five test targets failed even though the new visible shell geometry itself
   was non-overlapping.

2. The new Outliner used the taxonomy display name without the certified Shipyard serial.
   `shipyard_a_wing_149_...` therefore no longer surfaced the historical human-readable
   `Wing 149` label.

## Repair

- Compatibility controls now receive a unique large diagonal off-screen coordinate.
- Asset Browser and Outliner both use the same readable `DisplayName + serial` grammar.
- No renderer rollback and no return to the obsolete workspace strip.

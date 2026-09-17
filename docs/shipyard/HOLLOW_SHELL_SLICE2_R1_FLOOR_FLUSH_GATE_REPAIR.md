# Hollow Shell Slice 2 R1 — floor-flush portal test repair

Input: certified Git parent `02615ebf892fd95bd5d8d356ed7651d9800e20ed`,
with Hollow Hull Slices 1 and 2 applied. This small update changes one C++
regression test and adds this documentation; it does NOT change runtime or
geometry algorithms.

## Failure reproduced from Windows Full Gate

`SubspaceDerivedHollowShellTests` was the sole failure (106 of 107 passed).
The assertion `both seam faces receive a four-sided door frame` was written
for Slice 1's vertically centered portal. Slice 2 deliberately lowered the
door opening to the **real deck floor** so characters do not hit a knee-high
invisible threshold. Subtracting a floor-flush opening leaves three wall
segments per facing module (left/right jambs + header), not four. The passage
still has four separate liner quads, including its floor collider.

## Repair

- Require **exactly 3** surviving wall-frame quads on each side.
- Check the opening reaches the common deck floor.
- Check the passage has four liner quads with exactly one floor liner.
- Preserve subsequent union, partial overlap, invalid gap, exclusion,
  rotation fail-closed, and layout authority test cases unchanged.

The targeted isolated Linux reproduction passed 60 assertions after this
repair, with an existing unrelated misleading-indentation warning in
`ShipModuleInteriorLinkSystem.cpp`. This does not constitute a Windows Full
Quality Gate certification or visual FPS acceptance.

## Installation / acceptance

Place the `.patch` unopened at the repository root and approve through the
project-owned PCC. Run Full Quality Gate, then Run & Play to verify actual
passage floors, wall collision, and walking between the two joined hulls.
Do not commit until both succeed. This patch does not address the root audit's
non-blocking `rust` and PASS1439 file warnings.

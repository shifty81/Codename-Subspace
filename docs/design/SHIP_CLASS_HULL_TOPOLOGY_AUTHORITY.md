# Ship Class Hull Topology Authority — Pass1440

## Locked rule

Codename Subspace separates **ship class** from **role** and **module count**.

For normal ships, the vessel has one primary hull/chassis. Modules such as armor, wings,
engines, cargo, weapons, sensors, hangars, adapters, structural braces, and surface detail
attach to that one primary hull but do not become additional hull roots.

The standard combat progression is:

- Frigate — exactly 1 primary hull
- Destroyer — exactly 1 primary hull
- Cruiser — exactly 1 primary hull
- Battlecruiser — exactly 1 primary hull
- Battleship — exactly 1 primary hull

The physical jump between each class is intentionally large enough to read immediately
in Shipyard and in space. The canonical nominal-length progression remains strongly
separated, and the first capital tier makes another substantial jump beyond Battleship.

## Capital transition

Capital construction begins beyond Battleship. Capitals are not scaled-up single hulls.
They are built by joining multiple certified primary hull roots into one capital frame.

Canonical capital topology targets:

| Class | Minimum hull roots | Target | Maximum |
|---|---:|---:|---:|
| Carrier | 2 | 3 | 4 |
| Dreadnought | 3 | 4 | 6 |
| Industrial Capital | 3 | 5 | 7 |
| Capital | 4 | 6 | 9 |

A one-hull capital is invalid and may not be surfaced as a safe draft. Likewise, a
non-capital vessel with multiple primary hull roots is invalid.

## Generator behavior

`ShipClassGenerationAuthoritySystem` counts actual placed `PrimaryHull` records.

- Non-capital: exactly one primary hull is required.
- Capital: the class-specific multi-hull envelope is required.
- Invalid hull topology forces topology regeneration.
- Invalid hull topology is never considered a safe player-visible draft.
- Whole-ship scale remains limited to minor final metric correction; class growth comes
  from proper hull/module topology instead of inflating a smaller ship.

The current showcase generator already begins from one primary hull. This is correct for
normal ships. Until a capital-specific multi-hull assembler supplies the required hull
roots, capital generation must fail closed rather than emit a fake one-hull capital.

## Capital physical envelopes

Pass1440 increases the first capital-size envelope so Carrier is no longer visually
close to Battleship. Dreadnought, Industrial Capital, and generic Capital occupy
progressively larger strategic envelopes.

## Compatibility

Existing faction hull families remain **design families/templates**, not simultaneous
hulls on one standard ship. A generated Frigate may select one of several faction hull
families, but the resulting vessel still has one primary hull root.

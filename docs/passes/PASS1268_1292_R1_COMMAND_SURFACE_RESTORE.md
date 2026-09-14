# Pass1268-1292 R1 — Shipyard command-surface restore

The visible-cutover patch accidentally carried a stale `ShipyardBuilderSystem.h` command enum while retaining the current `ShipyardBuilderSystem.cpp` implementation. MSVC therefore rejected case labels that referenced five commands present in the certified implementation but absent from the overwritten header.

Restored commands:

- `ModelPreviousPurpose`
- `ModelNextPurpose`
- `ModelAssignPurpose`
- `PreviousGeneratorDomain`
- `NextGeneratorDomain`

R1 intentionally leaves the visible-cutover implementation and renderer strategy unchanged. It restores header/source parity and adds a static gate so a future GUI/header replacement cannot silently delete command members still consumed by the implementation.

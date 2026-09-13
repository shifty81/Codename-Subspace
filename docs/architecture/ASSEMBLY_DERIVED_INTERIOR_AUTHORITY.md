# Assembly-Derived Interior Authority

Subspace interiors are derived from the final transformed ship assembly. They are not a generic list of rooms inferred from module counts.

Pipeline:

`Assembly -> transformed module envelopes -> inset pressure/walkable volumes -> machinery/weapon/wing exclusions -> attachment portal stitching -> deck bands -> connectivity certification -> runtime room/corridor materialization -> furnishing/nav/atmosphere derived products`

Rules:

- Hull/bridge/cockpit/cargo/hangar and human-scale structural connectors may contribute walkable volume.
- Engines, nozzles, RCS, weapons, hardpoints, sensors, wings/fins and surface detail are exclusion/service volumes unless explicitly authored otherwise.
- Pressure-hull inset preserves visible exterior shell thickness.
- Portal connectivity follows actual exterior attachment edges first. Overlapping/touching walkable volumes may receive a generated open passage for legacy recipes lacking attachment metadata.
- Every walkable cavity must reach the command/root interior. Disconnected cavities fail interior cohesion certification.
- Deck count derives from actual transformed vertical span, not exterior module count.
- `ShipInteriorLayoutSystem` materializes the carve result; it must not fabricate unrelated generic rooms.

Future derived products include corridor mesh booleans, floor/ceiling/wall skins, doors, ladders/elevators, navmesh, atmosphere zones, furnishing anchors, damage/breach propagation and visual cutaway preview.

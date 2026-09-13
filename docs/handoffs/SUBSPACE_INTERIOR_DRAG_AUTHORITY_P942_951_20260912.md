# P942-951 — Assembly Interior + Cursor Drag Authority

- Ship interiors now derive from transformed assembly modules instead of generic room counts.
- Walkable pressure-hull volumes, non-habitable exclusions, portal stitches, deck bands and command/root reachability are explicit runtime data.
- Disconnected walkable cavities fail cohesion certification.
- `ShipInteriorLayoutSystem` materializes the carve result.
- The live playable-interior renderer consumes the current player's carve plan and draws assembly-derived room shells and portal/corridor bridges. The old fixed starter deck is fallback-only.
- Shipyard direct manipulation now inverts the exact renderer transform, including `forwardVisualYawDegrees` and visual axis scaling.
- Pointer dragging ray-projects at the selected module's current visual depth, so catalog ghosts and placed modules track the cursor instead of moving in the opposite visual frame.

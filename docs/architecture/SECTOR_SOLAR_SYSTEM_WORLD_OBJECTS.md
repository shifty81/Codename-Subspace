# Sector Solar System World Objects

Generated celestial bodies should not be mere skybox decoration. In Subspace, a sector's star, planets, belts, and anomalies are world-space objects with gameplay implications.

## Rules

- The star/primary body is a world-space object.
- Planets and belts orbit the primary at slow deterministic rates.
- Bodies have resource tags, hazard flags, and scan output.
- Visual spacing must read as a solar system, not a cluster of icons.
- Rings should be visually attached to the planet and drawn as ellipses.
- Surface bands/details should stay within the planetary disc.

## Current interaction layer

The playable client now supports selecting and scanning bodies:

```text
N = select nearest celestial body
G = scan selected celestial body
console: bodies
console: body nearest
console: body scan
```

Future passes should connect these scans to travel, station economy, mining contracts, hazard warnings, and discovery/codex records.

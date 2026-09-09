# Sector Solar-System Celestial Pipeline

Each sector should become an actual solar system rather than a loose playfield.

```text
Sector ID + seed
  -> CelestialSystemGenerator
  -> StarSystemDefinition
  -> CelestialBodyDefinition[]
  -> RuntimeVisualProfile
  -> playable client / scanner / map / missions / resource tables
```

## Rules

- A sector owns one deterministic `StarSystemDefinition`.
- The system has a primary star or rare black-hole primary.
- Orbit bodies are tagged by role: inner system, habitable band, outer system, asteroid belt, deep space.
- Celestial type is gameplay authority, not just decoration.
- Visuals are generated from body definition data so scanner, map, economy, and client rendering can agree.

## Gameplay hooks

| Celestial type | Gameplay hooks |
|---|---|
| Star | solar hazard, station power, navigation beacon |
| Rocky planet | ore, barren outposts, surface mining |
| Dry terran | frontier settlements, salvage, organics |
| Terran / river world | settlements, trade, water, agriculture, missions |
| Ice world | volatiles, ice mining, cryo hazards |
| Lava world | rare metals, thermal hazard, high-value contracts |
| Gas giant | fuel harvesting, storm hazards, orbital stations |
| Ringed gas giant | fuel + ring mining + pirate activity |
| Asteroid belt | mining, salvage, hidden sites, pirate ambushes |
| Black hole | anomaly research, gravity hazard, rare resources |
```

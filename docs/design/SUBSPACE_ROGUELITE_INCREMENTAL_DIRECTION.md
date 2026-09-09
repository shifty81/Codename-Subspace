# Subspace Roguelite Incremental Direction Lock

## Locked direction

Subspace is now a C++ roguelite incremental space-industrial game built around two connected layers:

1. **Persistent Home Solar System** — safe by default, buildable, automatable, saved permanently unless world configuration overrides it.
2. **Expedition Systems** — dangerous generated run sectors used for mining, salvage, combat, scanning, derelicts, hazards, contracts, and rare progression materials.

The player builds and upgrades a permanent home solar system, then launches risky expeditions to feed that growth.

## Reference feel

The active target feel combines:

- top-down skill-based space flight and readable beam/action combat;
- gritty ship systems, salvage pressure, fuel/air/repair/debt/permit themes;
- Factorio-like surface and logistics construction in home build zones;
- Dyson-sphere-esque orbital/stellar infrastructure as long-term incremental goals;
- roguelite expedition risk, extraction, failure consequences, and permanent progression.

These are reference pillars only. Subspace must remain original in naming, content, systems, code, assets, and UI.

## Core loop

```text
Home system:
  Build factories, storage, power, shipyards, automation, and Dyson-like infrastructure.

Expedition:
  Launch into generated systems, scan bodies, mine, salvage, fight, trade, and extract.

Return:
  Process cargo, repair/refit, unlock blueprints, upgrade shipyard, expand automation, and push deeper.
```

## Home safety rule

Default behavior:

```text
Home solar system = safe, persistent, buildable.
Expedition systems = dangerous, generated, run-based.
```

World configuration may enable raids, home structure damage, harsher death, cargo loss, or other pressure, but the default experience keeps the home area safe.

## Conversion rule

Old C# systems are no longer copied line-for-line. They are converted into this new direction:

- port mechanics that support ship construction, mining, salvage, cargo, combat, stations, economy, factions, procedural generation, UI, and builder/editor tooling;
- redesign monolithic runtime/UI code during conversion;
- reject or defer prototype-only systems that do not serve the roguelite incremental direction.

# Home Solar System Spec

## Purpose

The Home Solar System is the player's persistent, safe, buildable base layer. It turns expedition rewards into permanent growth.

## Required components

- home star with solar intensity and long-term Dyson-style infrastructure;
- home planet or moon with surface factory zones;
- orbital shipyard grid;
- starter asteroid/resource belt;
- persistent structures and construction queues;
- automation bandwidth and drone/logistics capacity;
- shipyard progression and blueprint unlocks;
- save-game authority for every home structure and inventory item.

## Default starter layout

```text
Home Star
  Solar Collector Orbit

Foundry World
  Starter Surface Zone
  Landing Pad
  Extractor
  Refinery
  Storage Depot

Foundry Orbit
  Orbital Shipyard Grid
  Shipyard Bay

Starter Belt
  Safe low-tier mining/salvage resources
```

## Persistence rules

Home structures persist through saves by default. World configuration can alter this, but the normal mode never resets the home system when a run fails.

## Safety modes

- `Safe`: no home raids or structure damage.
- `CosmeticThreats`: events can appear but cannot destroy structures.
- `RealThreats`: raids/hazards can damage home structures.
- `Hardcore`: home system can become part of the roguelike failure model.

## Interaction milestones

1. Select/scan home bodies.
2. Open a home build zone.
3. Place starter structures.
4. Tick production networks.
5. Store outputs persistently.
6. Spend outputs on shipyard upgrades.
7. Launch expedition from shipyard/subspace array.

# Surface Factory and Automation Spec

## Goal

Surface building is the Factorio-like home layer. It starts simple and grows into persistent production chains that support expedition prep.

## First vertical slice

```text
Extractor -> Refinery -> Storage -> Shipyard Upgrade
```

Initial structures:

- Landing Pad
- Extractor
- Conveyor Hub
- Storage Depot
- Refinery
- Assembler
- Power Relay
- Drone Depot

## Starter recipes

- ore -> ingot
- scrap -> recovered parts
- ingot -> hull plate
- ice -> fuel + water
- hull plate + recovered parts -> module component

## Automation constraints

Automation should be powerful but bounded by:

- available power;
- automation bandwidth;
- drone control capacity;
- structure tier;
- logistics throughput;
- world configuration.

## Expedition relationship

Safe home production handles low-tier materials. Manual expedition runs provide rare materials, blueprints, anomaly data, military salvage, and high-value modules.

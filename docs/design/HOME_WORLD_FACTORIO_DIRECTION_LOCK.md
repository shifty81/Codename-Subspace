# Home World Factorio Direction Lock

The persistent Home Solar System should not primarily be a dashboard. It should be a safe permanent buildable space.

## Rules

1. `H` opens the habitable home-world surface view.
2. The home planet is the main production, storage, research, and shipyard hub.
3. Other home-system bodies are extraction/support outposts.
4. Outposts export to the home planet through logistics routes.
5. Ship parts are upgraded and hot-swapped at home before launching an expedition.
6. Expedition systems are risky roguelite spaces; home is safe by default.

## First vertical slice

```text
Home Surface:
  Extractor -> ConveyorHub -> Refinery -> StorageDepot -> ShipyardBay

Off-world support:
  MoonMine / BeltExtractor / SolarCollector -> ImportRoute -> Home Storage

Adventure prep:
  home ship bay -> validate fuel/slots/spares -> launch rail route -> expedition system
```

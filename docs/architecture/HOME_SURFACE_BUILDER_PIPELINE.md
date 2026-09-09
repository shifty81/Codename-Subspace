# Home Surface Builder Pipeline

The home build system is intended to be the safe, persistent, incremental half of Subspace.

```text
HomeClientView
  -> selected HomeBuildZone
  -> HomeSurfaceBuilder placement request
  -> HomeSolarSystemState structures
  -> HomeFactoryNetwork inventory/costs
  -> derived power/automation recalculation
  -> client build grid view
```

## Rules

- The home system is safe by default.
- Structures persist through saves by default.
- Construction consumes home inventory unless explicitly in developer/free-build mode.
- Starter structures are protected during the early builder pass.
- Expedition systems provide rare resources that unlock stronger home construction.

## Structure Placement

A placement request validates:

1. selected zone exists,
2. cursor is inside zone bounds,
3. tile is unoccupied,
4. structure type is allowed in the zone type,
5. inventory can pay the cost.

## Future Routing

The first builder pass stores structures as tile/grid instances. Future passes should add:

- adjacency/logistics links,
- conveyors/drones/pipes,
- per-structure recipes,
- power net routing,
- production overlays,
- blueprint stamps,
- mouse placement and drag placement.

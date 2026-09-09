# Home Surface Primary View Pipeline

The Home Solar System now has two separate presentation layers:

```text
Home Planet Surface View
  primary Factorio-like build surface
  structure placement
  production/logistics/power overlays
  ship prep bay

Home Solar System Overview
  secondary strategic map
  bodies/outposts/routes/exports
  Dyson/solar/orbital infrastructure
```

## Data flow

```text
HomeSolarSystemState
  + HomeFactoryNetworkState
  + HomeProductionPlanner
  + HomePowerGrid
  -> HomeSurfaceWorldView
  -> client home surface renderer
```

Off-world bodies feed the home planet through:

```text
HomeOffworldOutpost
  -> HomeImportExportRoute
  -> HomeFactoryNetworkState.inventory
  -> production / shipyard / upgrades
```

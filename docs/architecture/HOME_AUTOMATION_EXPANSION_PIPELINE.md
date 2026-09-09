# Home Automation Expansion Pipeline

The Home Solar System loop now flows through dedicated subsystems instead of piling more logic into the playable client.

```text
HomeSolarSystemState
  -> HomeSurfaceBuilder
  -> HomeProductionPlanner
  -> HomePowerGrid
  -> HomeLogisticsNetwork
  -> HomeAutomationScheduler
  -> ShipyardBuildQueue
  -> HomeClientViewModel
  -> Win32PlayableClient home panels
```

Expedition runs now have their own contract/reward boundary:

```text
ShipyardProgression + completed run count
  -> ExpeditionContractBoard
  -> ExpeditionRun
  -> ExpeditionRewardResolver
  -> RogueliteDirector / home inventory / shipyard progression
```

Normalization is tracked separately:

```text
ProjectNormalizationLedger
  -> active C++ authority
  -> C# source-to-port backlog
  -> assets/data needing content normalization
  -> third-party reference/provenance
```

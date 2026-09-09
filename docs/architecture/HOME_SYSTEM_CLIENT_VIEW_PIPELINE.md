# Home System Client View Pipeline

The persistent home system is the safe, incremental anchor for Subspace. Expedition runs generate risk and rewards, while the home system persists by default and grows through automation and shipyard upgrades.

## Pipeline

```text
HomeSystemSaveSnapshot
  -> HomeSolarSystemState
  -> HomeFactoryNetworkState
  -> ShipyardProgressionState
  -> RogueliteDirectorState
  -> HomeClientViewModel
  -> playable client home view
```

## Separation rules

- `HomeSolarSystem` owns persistent celestial/build-zone/structure state.
- `HomeFactoryNetwork` owns production/inventory ticking.
- `HomeShipyardProgression` owns long-term shipyard/research upgrade state.
- `RogueliteDirector` owns run offers and applies run success/failure consequences.
- `HomeClientViewModel` formats a read-only client representation.
- The Win32 client draws the current state and forwards high-level actions.

## Next implementation targets

1. Add a true surface build-grid interaction mode.
2. Persist home system snapshots to disk.
3. Add placement preview, build costs, and structure construction progress.
4. Add visible orbital/Dyson structure slots around the home star.
5. Add run selection cards with explicit required ship/fuel/risk metadata.

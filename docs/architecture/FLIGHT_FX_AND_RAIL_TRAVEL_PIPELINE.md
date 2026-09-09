# Flight FX and Rail Travel Pipeline

Inside a solar system, the player flies manually. Between solar systems, the player uses a fitted ship on an on-rails route segment.

```text
Ship part loadout
  -> flight-control profile
  -> fuel burn / thrust / RCS output
  -> thruster particle emitter
  -> manual in-system flight

Ship part loadout
  -> rail-travel fit report
  -> rail-route encounter model
  -> on-rails travel reward/risk resolution
```

Thruster cutoff / coast mode should reduce fuel burn by disabling main thrust while allowing the ship to continue drifting. RCS can remain active for low-cost maneuvering or also be disabled for full ballistic coasting.

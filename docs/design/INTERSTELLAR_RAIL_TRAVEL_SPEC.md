# Interstellar Rail Travel Spec

Subspace uses two travel modes:

1. **In-system flight**: normal manual top-down flight inside the current solar system.
2. **Inter-solar-system rail travel**: a route-based, on-rails travel segment inspired by factory-platform travel games. The player selects a route, launches with a properly fitted ship, and earns or loses resources based on fit, cargo, hazards, and route pressure.

## Rules

- Rail travel is not free teleportation.
- Ship fitting matters: drive tier, fuel, cargo capacity, scanners, defense, mass, and thrust all contribute.
- Routes have different profiles: safe lane, industrial/belt route, pirate-watched wreck lane, anomalous deep route.
- Better routes are more lucrative but require better fitting.
- Routes can collect cargo/salvage while moving, creating a lightweight incremental haul loop.
- The player flies normally once they arrive inside a solar system.

## Immediate gameplay loop

```text
Home Solar System
  -> fit ship at home shipyard
  -> choose rail route
  -> travel on rails to expedition solar system
  -> manual flight/mining/salvage/combat
  -> extract rewards
  -> return resources to home automation
```

## Future visuals

Rail travel should eventually show the ship moving on a route lane with particles, debris pickups, hazards, route-side resource chunks, incoming events, and optional player choices. The current pass adds the model and command bridge first.

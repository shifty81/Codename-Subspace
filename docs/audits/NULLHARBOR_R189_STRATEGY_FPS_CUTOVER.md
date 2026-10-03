# NullHarbor R189 — Strategy-First Runtime / Real FPS Cutover

Baseline: `22aa9510b77e94df3a8d6062505b1bde00f78d28` / `QG-20261002-090624-full-840e2fc4`.

The certified runtime still attached `PlayerControlSystem` directly to the player ship at bootstrap while `FleetStrategyControlSystem` and `FirstPersonViewSystem` were not wired into `NativeGameApplication`. The on-foot renderer also explicitly described itself as a top-down strategic preview.

R189 makes Fleet Command the initial live control domain, clears direct ship authority at boot, routes WASD through `FleetStrategyControlSystem`, makes `I` board the player ship into on-foot embodiment, and only reattaches `PlayerControlSystem` after the physical helm transition. A world-space `FirstPersonViewPose` now drives native perspective projection; the authored interior ceiling is restored in FPS and the local avatar proxy is not rendered into its own camera. `StrategicFlightSystem` remains ship-autopilot authority, not the player's RTS camera.

The first materialization is allowed only from exact Git HEAD `22aa9510b77e94df3a8d6062505b1bde00f78d28` with target files equal to `git show`. Once R189 semantic postconditions exist, future governed descendants are verification-only and are not hash-pinned. Windows Full Gate and visual runtime acceptance remain required.

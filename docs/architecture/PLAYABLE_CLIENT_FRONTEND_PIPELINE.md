# Playable Client Frontend Pipeline

## Current pass

Pass 44 introduces a Windows-native client target:

```text
subspace_client.exe
```

It provides the first visible and testable game window while keeping `subspace_game.exe` available as a console/runtime diagnostic executable.

## Runtime ownership

```text
Win32PlayableClient
  owns local playable slice state
  owns GameRuntime
    owns RuntimeSession
    owns RuntimeWorld
    owns DeveloperModeController
      owns DeveloperCommandBridge
      owns DeveloperEditingLayer
```

The playable slice currently keeps movement, asteroids, cargo, station, and HUD state locally inside the client target. Runtime developer commands are forwarded to `GameRuntime::ExecuteDeveloperCommandLine` so the developer-mode path is exercised inside the visible client.

## Why this is deliberately simple

The project needed a real client window before more systems were added. A Win32/GDI slice is intentionally low-dependency and easy to compile on a Windows developer PC with Visual Studio. It is not the final rendering architecture.

## Next architecture step

The next client milestone should move from local placeholder state to canonical runtime state:

```text
Player input
  -> PlayerController
  -> RuntimeWorld entities
  -> Simulation systems
  -> Render backend draw model
  -> Developer overlay / console surface
```

Recommended next passes:

1. Add a renderer-neutral client draw model.
2. Move player/asteroid/cargo/station into `RuntimeWorld` or ECS-backed runtime objects.
3. Route viewport picking into `DeveloperViewportBridge`.
4. Consume `DeveloperOverlay` draw lists in the visual client.
5. Replace placeholder GDI primitives with the chosen production render backend.

# Runtime Visual Profile Pipeline

`RuntimeVisualProfile` is the active bridge between gameplay/content definitions and rendered test-client visuals.

## Goals

- keep visual authority outside `Win32PlayableClient.cpp`
- render ships from converted C++ module data
- let the developer client, future editor, and future renderer share a common visual draw model
- prepare for PixelPlanets-style celestial generation without carrying Godot runtime dependencies

## Current flow

```text
ShipModuleLibrary
  -> ModularShipFactory
  -> ModularGeneratedShip
  -> BuildShipVisualProfileFromModules
  -> RuntimeVisualProfile primitives
  -> Win32/GDI draw adapter in playable client
```

## Renderer-neutral primitive model

Profiles are composed of primitives:

- polygon
- box
- diamond
- circle
- ring
- line/text placeholders for later

Each primitive includes:

- semantic role
- source module id
- visual layer
- fill/stroke colors
- local-space points or bounds

## Next integrations

1. Use this same profile model for NPC ships.
2. Add visual IDs to runtime entities.
3. Add a station/module visual factory instead of default station hardcoding.
4. Add PixelPlanets-derived C++ celestial profiles for stars, planets, belts, and black holes.
5. Add developer overlay inspection for visual source authority and primitive counts.

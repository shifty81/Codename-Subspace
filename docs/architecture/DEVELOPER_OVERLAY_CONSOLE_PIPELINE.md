# Developer Overlay Console Pipeline

## Goal

The developer overlay is the first visible surface for Development Play mode. It should let a developer inspect and issue live runtime edit commands while the game continues running.

## Architecture

```text
Keyboard / UI input
  -> DeveloperOverlay
  -> DeveloperConsole
  -> DeveloperCommandBridge::ExecuteLine
  -> DeveloperModeController
  -> DeveloperEditingLayer
  -> Runtime edit adapters
  -> Runtime session history
```

## Renderer-neutral UI contract

`DeveloperOverlay` does not draw directly. It produces `DeveloperOverlayDrawList` primitives. This keeps the overlay independent of a specific backend.

Future backend targets:

- OpenGL debug overlay renderer;
- D3D renderer;
- native Windows development UI;
- future standalone editor surface;
- future AI/editor command panel.

## Development mode states

The overlay should be available in development builds and hidden in release builds unless explicitly enabled by a developer flag.

Recommended future states:

```text
Hidden
VisibleConsole
VisibleInspector
VisibleAssetReloadFeed
VisibleCommandPalette
```

Pass 07 only implements the console-ready foundation.

## Safety rule

The overlay must never directly mutate gameplay or assets. It routes text commands through the command bridge. This preserves one audit trail, one undo/redo path, and one future permissions gate.

## Future wiring

Next passes should connect:

1. input/keybind toggle;
2. renderer backend for draw-list primitives;
3. concrete asset reload handlers;
4. concrete entity/component reflection;
5. concrete ship editor mutation handler;
6. in-world visual gizmos.

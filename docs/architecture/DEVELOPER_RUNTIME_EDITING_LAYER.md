# Developer Runtime Editing Layer

## Purpose

Codename Subspace needs a development editing layer that can run while gameplay is active. The goal is not a separate offline-only editor. The goal is an active development mode where designers and developers can inspect, edit, reload, validate, and promote changes while the game simulation continues.

This layer should support:

- live ship/module editing during gameplay;
- runtime entity/component inspection;
- real-time asset reload for models, textures, materials, data, scripts, and shaders;
- undo/redo for edit commands;
- dirty state and reviewable change history;
- validation before changes are saved back to source content;
- future native editor panels, console commands, and node-graph tools using the same command bus.

## Non-goals for the first pass

The first pass is not the full visual editor. It should not become a giant monolithic UI. It should only establish the runtime-safe backend contract that future editor panels can call.

Do not bake editor-only assumptions into gameplay systems. Gameplay systems should receive commands through stable adapters and remain usable in release builds without editor UI.

## Required architecture

```text
Developer UI / Console / Native Editor / Node Graph
                  |
                  v
        DeveloperEditingLayer
                  |
      RuntimeEditSession + Command History
                  |
       RuntimeEditCommand dispatch
                  |
   System-specific Runtime Edit Adapters
                  |
   Gameplay Systems / ECS / Assets / Renderer
```

## Core components added in this pass

| Component | Role |
|---|---|
| `RuntimeEditCommand` | Transportable command format for live edits. |
| `RuntimeEditSession` | Queue, applied history, undo/redo, dirty state. |
| `AssetHotReloadService` | Polling file watcher for asset changes. |
| `DeveloperEditingLayer` | Runtime command router and asset-reload bridge. |

## Runtime edit command rules

All live editing operations should be expressed as commands.

Examples:

```text
Update entity://player_ship/components/Transform position from A to B
Reload content://models/ships/starter.glb
Paint ship/player/blocks/12,0,3 material from Steel to Titanium
Update tuning://economy/miningYield from 1.0 to 1.25
Validate content://ships/starter_frigate
```

Commands should include before/after values whenever possible. This allows undo, redo, save promotion, and clean diff review.

## Asset hot reload rules

The hot reload service should begin as polling-based for reliability and portability. Later it can be upgraded to platform-specific watchers such as Win32 `ReadDirectoryChangesW`.

Reloaded assets must not be blindly applied to all runtime instances. The flow should be:

1. detect asset changed;
2. queue reload command;
3. load asset into a staging object;
4. validate asset schema and dependencies;
5. swap runtime handles atomically;
6. update dependent entities/materials/UI previews;
7. report success/failure in the dev notification/log panel.

## Live asset editing workflows

### Model or module asset changed

```text
Blender/exporter writes GLB/OBJ
AssetHotReloadService detects timestamp change
DeveloperEditingLayer queues RuntimeEditCommand(asset reload)
Model/Asset adapter reloads into staging cache
Validation checks scale, bounds, sockets, materials, collision tags
Renderer swaps mesh handle
Ship/module instances receive refreshed render data
If gameplay collision changed, physics proxy rebuild is requested
```

### Ship edited during gameplay

```text
Ship editor places/removes/paints block
ShipEditorController mutates ship runtime structure
RuntimeEditCommand records operation and before/after delta
Ship validation runs incrementally
Stats/collision/render proxy rebuild is requested
Dirty state marks ship asset/session as unsaved
Developer can apply to source blueprint or discard
```

### Tuning value changed during gameplay

```text
Inspector edits economy/mining/combat value
RuntimeEditCommand updates tuning registry
Affected systems receive config-changed event
HUD/dev overlay shows changed value
Dirty state marks tuning data as unsaved
Developer can save to content/data/tuning/*.json later
```

## Development mode boundaries

The runtime editing layer should be compiled behind a development flag and guarded at runtime. Release builds can keep the command types available if needed for mods/tools, but privileged editing features must be disabled by default.

Recommended flags:

```text
SUBSPACE_ENABLE_DEVTOOLS=1
SUBSPACE_DEVELOPMENT_BUILD=1
```

## Next adapters to implement

| Adapter | First responsibility |
|---|---|
| `ShipRuntimeEditAdapter` | Bridge `ShipEditorController` operations into `RuntimeEditCommand` history and validation. |
| `AssetRuntimeEditAdapter` | Reload models/textures/materials/data through a real asset registry. |
| `EntityInspectorEditAdapter` | Read/write ECS component properties safely. |
| `WorldRuntimeEditAdapter` | Spawn/move/delete stations, asteroids, fields, encounters, and POIs. |
| `TuningRuntimeEditAdapter` | Live-edit gameplay settings and save them back to data. |
| `RendererHotReloadAdapter` | Rebuild materials/shaders/mesh handles without restarting. |

## Important refactor requirement

The editing layer must not directly own every gameplay system. It should use narrow adapters. Otherwise the developer layer will become another monolith.

Correct:

```text
DeveloperEditingLayer -> CommandHandler -> ShipRuntimeEditAdapter -> Ship system
```

Avoid:

```text
DeveloperEditingLayer directly includes every gameplay header and mutates everything
```


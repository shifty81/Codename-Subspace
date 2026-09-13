# Shipyard Editor / Game Client Parity Authority

## Decision

Codename Subspace has **one native game client** and **one native Shipyard editor application**.

Shipyard is the project editor/engine identity. It is not only the in-game ship builder and it is not a collection of separate standalone authoring applications. It is a Blender-like authoring environment composed from task-oriented workspaces inside one shell.

Primary workspaces:

- Layout
- Ship
- Character
- World
- Interior
- Modeling
- Materials
- Animation
- PCG
- VFX
- Audio
- Logic
- Diagnostics

Station, turret, planet-sector and blueprint contexts remain specialized document contexts inside that one editor rather than separate applications.

## Parity rule

The editor is an **authoring superset**. The game client consumes the exact same canonical data contracts.

The client may expose a restricted player-facing editor when gameplay allows it, but that surface must mutate the same canonical type through gameplay permissions rather than inventing a second runtime format.

Examples:

| Canonical type | Shipyard editor | Game client |
|---|---|---|
| `Assembly` | full ship/module/socket/definition authoring | hangar Shipyard/refit under inventory, facility, skill, cost and validation rules |
| `CharacterDefinition` | morph/source/rig/apparel/portrait authoring | character creation/customization through approved channels and owned cosmetics |
| `InteriorDefinition` | portals/rooms/clearance/source authoring | safe-zone furniture/service customization where gameplay permits |
| `MaterialDefinition` | shader/surface/livery authoring | approved paint/pattern/decal controls |
| `WorldDefinition` | full world/system/planet/settlement authoring | consumed by runtime; player changes happen through gameplay construction/simulation |

## Hard parity requirements

1. Anything the client can render or simulate must be inspectable in Shipyard through the same asset/definition identity.
2. Anything Shipyard certifies as runtime-valid must be interpreted identically by the game client.
3. Player-facing editors are policy-constrained views over canonical definitions, never shadow formats.
4. Editor preview and game runtime must use the same compilers/validators wherever practical.
5. Runtime captures/saves should eventually be inspectable by Shipyard without rebuilding a fake editor copy of the entity.
6. Workspace changes only rearrange tools/panels; they do not switch to a different editor application.

## Editor shell direction

The existing `EditorDockSystem`, `EditorSelectionService`, `EditorCommandStack`, `EditorAssetBrowserModel`, `EditorPlacementResolver`, inspector schemas and context actions are the shared shell primitives.

Visible rendering/hit-testing should converge on those authorities. Workspace presets select which panels are open and how they are arranged; users may still rearrange and save layouts.

## In-game Shipyard naming

The project may continue using **Shipyard** as the editor/engine name. The player-facing ship workflow can use contextual labels such as Shipyard, Hangar Engineering, Refit or Construction without changing the editor architecture.

# Codename Subspace Shipyard — Blender Extension

`SubspaceShipyard` is the Blender-side authoring and procedural-generation front end for Codename Subspace's certified modular ship system.

## Target

- Blender 5.2 LTS is the current production target.
- The extension keeps compatibility with Blender 4.5+ APIs used by the project.
- The game remains authoritative for final gameplay stats, fitting validation, manufacturing modifiers, rarity rolls, inventory and runtime behavior.
- Blender is authoritative for visual module placement, socket authoring/inspection, equipment-slot markers, paint/decal intent, procedural visual generation, preview and exported ship-design layout.

## What it does

### Certified module catalog

Loads the project's current certified Greyoxide Shipyard v0.7 corpus directly from:

`content/derived/greyoxide_shipyard_v07/certified/certified_module_catalog.csv`

Only Grade-A certified modules are offered. The add-on reads the actual OBJ bounds and mirrors `ShipyardModuleSystem` semantic, size and socket rules.

### Manual authoring

- Add a certified module at the 3D cursor.
- Generate visible runtime-compatible socket empties.
- Select a parent and make the child active, then use **Snap Active Module to Selected Parent**.
- Add equipment-slot markers to any module at the 3D cursor.
- Use Blender's native transform, duplicate, undo/redo, collections and outliner normally.

### Procedural generator

Deterministic seed-based presets:

- Industrial
- Combat
- Mining
- Hauler
- Exploration

The baseline generator mirrors the current native C++ showcase grammar:

1. role/size-biased hull selection;
2. forward/aft socket hull chain;
3. command section on the first hull;
4. engines inserted into paired aft-hull engine cavities;
5. role-aware hardpoints;
6. restrained surface detail.

**Designer Enhanced** optionally adds wings and role-specific sensor silhouettes while still using certified module pieces and explicit exported transforms.

### Paint and decals

The extension stores primary, secondary and accent colors and a decal code in the authoritative design JSON. A simple Blender material is applied for immediate preview without destroying the source module geometry.

### Equipment slots

Equipment-slot empties export the same conceptual fields used by native `EquipmentSlot`:

- id
- allowed type
- max size
- mount name
- position
- rotation

Supported types include weapons, turrets, missiles, mining/salvage/tractor tools, shields, countermeasures, scanners, drones and repair bots.

### Validation

Current validation checks:

- ship has modules;
- instance IDs are present and unique;
- referenced module IDs still exist in the loaded certified catalog;
- structural pieces have uniform XYZ scale;
- command section exists;
- propulsion exists;
- equipment slots are parented to real module placements;
- connection parent IDs resolve.

Validation details are written to a Blender Text block named `Subspace Shipyard Validation` when issues exist.

### Preview and icons

**Setup Real-Time Preview** creates a dark space-style world, camera and three-point lights around the current ship. Use Material Preview/Rendered viewport for live inspection.

**Render Blueprint / Item Icon** renders the current assembled ship to a transparent 512x512 PNG. This is the basis for the same render-from-item pipeline needed by blueprint/module inventory icons in game.

### Export

The authoritative output is `*.subspace_shipyard.json` using schema `subspace.shipyard_design` version 1.

It contains:

- ship identity, author, role and deterministic seed;
- generator settings;
- paint colors and decal code;
- every module ID and instance ID;
- module class, semantic and size;
- position/rotation/scale;
- parent/child socket connection metadata;
- equipment-slot markers.

The extension can import its own design JSON for round-trip editing.

**Export Preview GLB** is deliberately marked preview/reference-only. Runtime should continue to assemble the certified module assets from the JSON rather than baking every ship into a unique mesh.

## Installation

### Direct extension ZIP

Install `subspace_shipyard-0.1.3.zip` through Blender's **Preferences > Get Extensions > Install from Disk**.

### Project source

The extension source is stored at:

`tools/blender/SubspaceShipyard/`

Blender 5.2 can build/validate the extension with:

```text
blender --command extension validate --source-dir tools/blender/SubspaceShipyard
blender --command extension build --source-dir tools/blender/SubspaceShipyard --output-dir tools/blender/dist
```

The repository helper `scripts/subspace_build_blender_shipyard_extension.ps1` wraps that flow on Windows.

## Coordinate contract

- +X = starboard/right
- -X = port/left
- +Y = ship forward
- -Y = aft
- +Z = dorsal/up
- -Z = ventral/down
- units = meters

This matches the current native Shipyard socket grammar.

## Important pipeline rule

Do not merge/destructively remodel the certified source corpus during normal assembly. The current runtime certification policy preserves each authored source object intact and treats connected islands as diagnostics only.

## 0.1.1 classification correction

Greyoxide's engine layer is an engine-builder parts layer, not a list of complete drives. 0.1.1 reads the certified catalog `semantic` field when present and otherwise performs filename-first classification on the original leaf name. Historical `shipyard_a_propulsion_*` text is treated as provenance only. Engine struts are structural frames, `engineTrussworkWing` is a wing, `engineBody*`/bracket pieces are housings, and the generator selects only `MAIN_ENGINE` modules for drive anchors.


### 0.1.2 path-resolution normalization

The Project field now accepts the repository root, `content/derived/greyoxide_shipyard_v07`, the `certified` directory, or any path inside the repository. A direct `certified_module_catalog.csv` override is also available. The extension also checks `SUBSPACE_PROJECT_ROOT`, the current `.blend` location, the working directory, and common local checkout paths before reporting a missing catalog.


## 0.1.3 axis + attachment-face normalization

The Greyoxide OBJ corpus is authored/exported as X-right, Y-up, +Z-forward while the Subspace Shipyard authoring contract is X-right, +Y-forward, Z-up. The extension now canonicalizes every imported vertex as `(X, Z, Y)` before creating Blender mesh data. Bounds use the same mapping. This matches the native renderer's `RemapObjVertex` contract and prevents long hulls from standing vertically in Blender.

The catalog loader also inspects OBJ boundary edges and records a conservative geometry-derived attachment-face hint. A strong open ventral/bottom face is used for surface-mounted pieces such as command/detail assets where appropriate. Socket snapping now aligns socket normals as well as positions, so side/bottom mounts can rotate into the parent instead of being translated without orientation correction.


## 0.1.4 — Shipyard R5 authority

- Compatible with the R5 certified Shipyard Strikes Back geometry refresh while retaining stable Subspace module IDs.
- Keeps the Pass453 canonical `(X,Y,Z) -> (X,Z,Y)` axis normalization and open-face mount hints.
- The three authored Scout/Battleship/Cruiser ships remain reference/universe vessels; procedural generation does not copy their topology.


## 0.1.5
Preserves Shipyard Strikes Back OBJ `usemtl` face regions and source MTL material properties during template import. Hull paint remaps Mat_Main/Mat_Seco without flattening metal, glass or emissive slots.

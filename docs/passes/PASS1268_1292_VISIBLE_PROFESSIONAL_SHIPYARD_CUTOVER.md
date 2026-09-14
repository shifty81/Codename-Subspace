# Pass1268-1292 — Visible Professional Shipyard Cutover

This is the first tranche whose success criterion is deliberately visual. The professional Shipyard architecture from Pass1203-1267 now becomes the public `Layout`, `BuildControls`, `HitTest`, and `Activate` surface consumed by the existing native renderer and input path.

## Visible shell

- Primary strip: **BUILD / INTERIOR / SYSTEMS / APPEARANCE / TEST / DEV**.
- DEV expands specialist workspaces instead of permanently consuming the header.
- Left construction surface is presented as an Asset Browser with search affordance, categories, compatibility/favorite/certification affordances, visual cards and placement actions.
- A readable Blender-style tool rail exposes Select, Move, Rotate, Scale, Snap and Frame while preserving Q/W/E/R authority.
- The right surface is visibly separated into **OUTLINER / SHIP HIERARCHY** and **PROPERTIES / INSTANCE + DEFINITION**.
- Systems exposes **GENERATE / NEW SEED + GENERATE / EXPLAIN**.
- TEST is a real primary shell state for validation, framing, draft/save/apply preparation rather than another permanent developer tab.

## Behavior preservation

The existing certified `ShipyardBuilderSystem.cpp` remains the implementation authority for authored mutations during this cutover. Its previous public layout/control/action symbols are compiled as temporary `Legacy*` entrypoints and delegated to by the new public shell where required. This avoids replacing the large certified implementation in the same pass as the visible UI cutover.

No alternate generator, undo stack, drag/drop implementation, socket system or refit path is introduced.

## Temporary compatibility boundary

`ShipyardDefinitionOverrideSystem.h` contains a narrowly-scoped preprocessor strangler shim that activates only when included after `ShipyardBuilderSystem.h`, which is the include order of the legacy builder implementation translation unit. This compatibility mechanism must be removed once renderer/input migration no longer needs the old public method definitions.

## Manual acceptance

Launch Shipyard from the main menu. The primary header must be visually different within two seconds: five task workspaces plus DEV, readable left Asset Browser and tool rail, and separate right Outliner/Properties regions. Then verify selecting modules, dragging/staging a part, transforms, symmetry, Undo/Redo, Generate, New Seed + Generate, Validate, save, and docked Apply/refit.

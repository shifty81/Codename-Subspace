# Pass1203-1222 — Professional Shipyard Core Normalization

This pass starts the structural Shipyard refactor before the visible Blender/Forge-style shell is switched over.

## Locked foundations

- `ShipyardDocumentSystem` separates authored ship content from temporary editor/session state.
- `ShipyardStableIdSystem` introduces stable typed IDs for documents, modules, attachments and future authored subobjects. Storage index is no longer the intended durable identity.
- `ShipyardSelectionSystem` provides one shared, multi-select-capable selection model for viewport, Outliner and Properties synchronization.
- `ShipyardHistorySystem` records document transactions rather than snapshots of the entire runtime/editor model. One gesture is one transaction.
- `ShipyardCommandSystem` establishes command metadata, search, shortcut ownership and automatic transaction wrapping. UI controls, hotkeys, future Cortex actions and menus converge on the same command IDs.
- `ShipyardSessionSystem` owns temporary workspace/tool/selection/layout/candidate state separately from authored data.
- `ShipyardMountProfileSystem` converts loose preferred-face hints into an explicit geometry-backed primary root surface plus alternate surfaces and provenance.
- Structural feet such as `enginestrutfoot` use their broad geometry-supported flat root surface as primary attachment authority when their old classification says `auto`.
- `ShipyardProfessionalUiSystem` defines the professional panel registry, layout presets, five primary workspaces, progressive-disclosure advanced workspaces, and the primary viewport tool vocabulary.

## Compatibility

This pass deliberately does not replace the current runtime builder yet. New C++ source files are compiled automatically by the existing `src/ship_editor/*.cpp` CMake glob, while the current GREEN `ShipyardBuilderSystem` remains behavior authority until the next migration tranche wires it to these systems.

This order prevents a UI rewrite from simultaneously changing save/runtime behavior.

## Next migration tranche

1. Bind current builder recipe/appearance to `ShipyardDocument`.
2. Bind workspace/tool/selection state to `ShipyardSession`.
3. Route existing buttons/hotkeys through `ShipyardCommandSystem`.
4. Retire builder-owned full-runtime-model history after parity tests are green.
5. Replace the visible workspace strip with `Build / Interior / Systems / Appearance / Test / Dev`.
6. Split Outliner and Properties into independently dockable panels.
7. Add `Generate` and `New Seed + Generate` over one deterministic generation authority.
8. Replace detached Project Tools process launching with managed jobs/output/cancellation.

# Shipyard Blender-Derived DCC UI Contract

## Status

Active visible corrective tranche: **Pass1338 / BLENDER-DCC-001**.

This contract governs the native standalone Shipyard surface. It intentionally follows Blender's dense DCC interaction and layout model without copying Blender branding or becoming a general-purpose modeling package.

## Primary UX rule

The ship is the dominant object. At 1920x1080 the default center 3D viewport must retain at least 1000 logical pixels of width while the Asset Browser and Outliner/Properties editors remain visible.

## Window hierarchy

1. **Application menu bar** — File, Edit, View, Ship, Select, Add, Help.
2. **Workspace strip** — Build/Model/Interior/Sockets/Appearance/Systems and capability-driven development workspaces.
3. **3D View header** — Assembly Mode, View/Select/Add Module, transform orientation, pivot, snap.
4. **Editor areas** — Asset Browser | 3D Ship View | Outliner over Properties.
5. **Status bar** — operation status, contextual help, and permanent visible build/UI identity.

## Asset Browser

The left editor is a visual module inventory, not a debug text list. It keeps the existing certified geometry thumbnails, categories, size/compatibility metadata, socket metadata, propulsion metadata, drag staging, and placement controls. The panel is compact enough that the viewport remains the primary surface.

## 3D Ship View

The center area owns camera orbit/pan/dolly, selection, staged geometry placement, socket/snap previews, transform tools, symmetry, forward marker, framing, shield preview, and validation overlays.

The current Q/W/E/R transform authority is preserved in this pass. A later context-aware keymap pass may add Blender-style G/R/S only after live gameplay bindings are explicitly isolated from Shipyard input.

## Outliner

The upper-right editor shows the ship root and current module hierarchy. Pass1338 gives the hierarchy its own visually separate region above Properties instead of the previous combined `OUTLINER / PROPERTIES` heading.

Follow-up work will add grouping by structural assembly/system, visibility and selection locks, drag parenting, collections, and direct bidirectional Outliner hit-testing.

## Properties

The lower-right editor remains backed by the current command/model authority, but its content starts below the Outliner. Future contextual vertical property tabs will include Ship, Transform, Attachment, Structure, Power, Propulsion, Defense, Weapons, Interior, Materials, Paint, Decals, Damage, Cargo, Crew, Validation, and Advanced Authoring.

## Visual language

- dark neutral charcoal editor regions;
- thin separators instead of decorative sci-fi frames;
- dense controls and restrained padding;
- compact headers embedded into editor areas;
- viewport grid subordinate to ship geometry;
- clear selected/hovered/snap/invalid states;
- no giant dashboard cards;
- no debug wall as the primary workflow.

## Runtime certification

A green Full Gate is not sufficient unless the actual Shipyard render path executes. Pass1338 adds:

- `--shipyard-smoke`;
- 1280x768 real standalone Shipyard initialization;
- multiple rendered frames;
- Full Gate execution after the generic runtime smoke;
- permanent visible `BLENDER-DCC-001 | PASS1338` identity in the Shipyard status bar.

## Pass1338 acceptance

1. Native C++ configure/build succeeds.
2. All registered CTest/static gates pass.
3. Application menu, workspace strip and 3D View header are visibly distinct.
4. Asset Browser is visibly distinct on the left.
5. Outliner is visibly above Properties on the right.
6. Central viewport is materially wider than the previous dashboard proportions.
7. Viewport tool rail remains visible and functional.
8. Existing staged placement/drag/symmetry workflows remain available.
9. Status bar visibly displays `BLENDER-DCC-001 | PASS1338`.
10. Full Gate runs `--runtime-smoke` and `--shipyard-smoke`.
11. Windows PCC Full Gate is required before this tranche may be called GREEN.

## Next visual tranche

After Pass1338 is visually accepted:

1. interactive hierarchical Outliner with visibility/lock controls;
2. switchable bottom Asset Shelf and search/filtering;
3. contextual vertical Properties icon tabs;
4. context-aware Blender G/R/S input;
5. resizable/splittable editor areas and Ctrl+Space maximize;
6. stronger socket editing overlays and snap visualization;
7. layered material/paint/decals workflow;
8. blueprint browser and variants;
9. systems graph;
10. visual capture evidence tied to certification.

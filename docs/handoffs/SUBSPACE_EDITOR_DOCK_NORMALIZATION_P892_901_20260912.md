# Subspace Pass892-901 Editor Dock Normalization Handoff

Date: 2026-09-12
Baseline Git authority: `1f714de6ea5c86ae45d38e01cd6167ca3dc6e5d3`
Patch authority: internal Subspace PCC root-drop `.patch`

## What changed

Pass892-901 establishes `EditorDockSystem` as the single project-wide editor dock authority.

The dock model now supports:

- open/close/toggle registered panels;
- protected central Viewport;
- tab groups with active-tab state;
- moving panels between groups;
- horizontal/vertical split regions;
- floating panels and redocking;
- per-workspace persisted dock state;
- renderer-facing materialized rectangles;
- duplicate/missing dock ownership validation.

`SubspaceEditorLayoutSystem` is retained for compatibility but now projects from the dock system instead of owning a separate rigid left/right/bottom layout.

Default panel registry:

- Left: Assets, Outliner
- Center: Viewport
- Right: Inspector, Selection, Properties
- Bottom: Console, Validation, Diagnostics, Jobs, History, PCG, Runtime Diff, AI Commands

Assets is explicitly a panel/tab and is not allowed to become an exclusive editor mode.

## Verification performed here

Linux/CMake headless compile completed for the shared engine and dedicated tests.

Passed:

- `SubspacePass892To901EditorDockNormalizationTests`
- `SubspacePass555To584ProjectWideEditorStationUpgradeTests`

The authoritative Windows/MSVC project Full Gate must still be run from internal PCC option 1 after application.

## Next visual/runtime dock work

This pass establishes the canonical state/layout authority. The next UI integration pass should make the native rendered editor consume the dock tree directly for:

- dock chrome and tab bars;
- drag-to-dock target previews;
- splitter dragging;
- close/open panel menu;
- floating native/editor windows;
- pointer hit-testing from the same materialized geometry;
- saved/restored user layout files;
- Shipyard/Station/PCG workspace-specific default presets.

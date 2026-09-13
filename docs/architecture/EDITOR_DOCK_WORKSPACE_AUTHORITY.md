# Codename Subspace Editor Dock Workspace Authority

Status: active architecture authority for the native editor shell.

## Rule

Subspace has one project-wide docking authority: `EditorDockSystem`.

The editor must not treat Assets, Inspector, Outliner, Console, Validation, PCG, History or other tools as mutually exclusive top-level modes. They are panels that may be opened simultaneously and arranged by the user.

## Required dock behavior

The canonical workspace supports:

- open / close / toggle any registered optional panel;
- protected non-closable central Viewport;
- tab groups with one active tab per leaf;
- moving a panel between tab groups;
- horizontal and vertical split regions;
- floating windows;
- redocking floating panels;
- per-editor-workspace persisted arrangements;
- default-layout reset;
- panel and node validation that rejects duplicate ownership;
- renderer-facing materialization into bounded rectangles.

## Default workspace

The default native authoring workspace is deliberately viewport-first:

- Left dock: Assets, Outliner
- Center: Viewport
- Right dock: Inspector, Selection, Properties
- Bottom dock: Console, Validation, Diagnostics, Jobs, History, PCG, Runtime Diff, AI Commands

The bottom dock starts collapsed until a tool is opened. Assets is a dock tab, not an editor state that can lock the rest of the application.

## Compatibility

`SubspaceEditorLayoutSystem` remains as a compatibility projection for existing renderer/tests, but it now materializes a dock workspace instead of owning a parallel fixed layout.

`DeveloperPanelModel` and `DeveloperWorkspaceState` are legacy/developer-facing panel metadata/state surfaces. They must converge onto or adapt to `EditorDockSystem`; they are not independent layout authorities.

## Shipyard

The Shipyard may retain specialized internal inspector tabs (Transform, Sockets, Authoring, Assembly, Appearance) inside the Inspector panel. Those are local inspector workflows, not substitutes for the global editor dock system.

Future rendered dock chrome should consume `EditorDockWorkspace` directly so panel movement and visibility affect both drawing and pointer hit-testing from the same geometry.

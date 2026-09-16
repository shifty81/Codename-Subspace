# Subspace DCC Asset Workbench

## PASS1444-1453 visible normalization

The standalone Shipyard is the first working consumer of a reusable Subspace DCC asset-workbench shell. The visual target is a conventional desktop 3D content-creation application: compact global chrome, a dominant 3D viewport, a thin tool rail, an Outliner over Properties on the right, and an Asset Browser shelf below the viewport. The goal is familiar spatial organization and interaction hierarchy, not a pixel-for-pixel copy of Blender.

The previous PASS1439 shell had Blender vocabulary but still behaved visually like a debug dashboard: the viewport remained black/game-like, Asset Browser consumed a large left column, cyan borders dominated the hierarchy, the ship framed too small, and permanent construction/symmetry diagnostics competed with the model. PASS1444-1453 changes the composition itself.

### Visible defaults

- Neutral charcoal editor shell and neutral gray 3D viewport.
- Perspective floor grid instead of the full-screen engineering graph-paper treatment.
- Gameplay starfield and travel backdrop are suppressed in standalone Shipyard authoring.
- Bottom Asset Browser shelf replaces the permanent large left library.
- Compact vertical tool rail sits directly against the 3D View.
- Outliner occupies the upper-right editor area; Properties owns the lower-right.
- Socket overlay and conformal shield preview default off and remain explicitly toggleable.
- Construction/symmetry frame appears only during transform/socket work instead of permanently.
- A compact XYZ viewport gizmo replaces the large center-screen forward arrow.
- Ship framing uses authored extents and renderer scale so the active asset occupies the viewport.
- Existing G/R/S, F3, Ctrl+Space, workspace, asset filtering, Outliner, Properties, undo/redo, socket and authoring systems remain the underlying authority.

## Tranche map

- **Pass1444** — shared DCC editor-area layout authority.
- **Pass1445** — neutral standalone authoring canvas.
- **Pass1446** — perspective floor grid and gameplay-backdrop separation.
- **Pass1447** — bottom Asset Browser shelf.
- **Pass1448** — Outliner / Properties right-area split.
- **Pass1449** — compact icon-oriented viewport tool rail.
- **Pass1450** — authored-bounds viewport framing and larger asset presentation.
- **Pass1451** — contextual construction diagnostics with clean socket/shield defaults.
- **Pass1452** — reusable asset-domain workbench catalog.
- **Pass1453** — historical-gate reconciliation, build identity and certification gate.

## Universal asset-shell direction

`EditorDccShellLayoutSystem` owns reusable editor-area geometry. `EditorAssetWorkbenchSystem` defines the asset-domain catalog and makes migration state explicit.

The currently live adapter is **Ship Modules / Shipyard**. Station modules, interiors, characters, props, materials, celestial/world assets, VFX and UI are registered as shell domains but are **not** falsely advertised as fully wired editors yet. They should migrate into the same shell through domain adapters that bind their existing authoritative systems to Asset Browser, viewport, Outliner and Properties surfaces.

This keeps one normalized authoring language across the game instead of creating a separate bespoke GUI for each asset type.

## Acceptance

PASS1444-1453 is source-complete when:

1. `subspace_game` builds and links.
2. all historical CTest targets pass after successor-authority reconciliation;
3. ProjectOps static certification passes;
4. Windows PCC Full Quality Gate passes;
5. rendered Shipyard smoke succeeds; and
6. visual inspection confirms the status identity `DCC-ASSET-SHELL | PASS1453` and the viewport-first DCC composition.

# Pass1243-1267 — GameMaker GUI Normalization

This tranche converts the GameMaker GUI research into native Shipyard runtime authority rather than visual mockups.

## Added

- `ShipyardAssetBrowserSystem`
  - text search, semantic/category/size tags, favorites, recents, saved filters, thumbnail scale/density, generator eligibility and selection-compatibility filtering.
- `ShipyardUniversalSearchSystem`
  - one F3 search surface spanning commands, assets, ship objects, properties, panels, validation and developer results.
- `ShipyardWorkspaceLayoutSystem`
  - saved/resettable layout state, panel visibility, auxiliary-dock collapse and hovered-panel maximization.
- `ShipyardActivityDockSystem`
  - consolidated Activity / Validation / History / Console / Search / Build / PCC dock state.
- `ShipyardConstructionGuideSystem`
  - temporary authoring guides for root mount planes, normals, clearances, player scale and distance.
- `ShipyardProfessionalShellSystem`
  - one renderer-facing projection composed from document, session, commands, Asset Browser, Outliner, Properties, layout and search authority.

## Existing authorities upgraded

- Properties explicitly distinguishes **INSTANCE** state from reusable **DEFINITION** state and exposes `Open Definition` as a deliberate action.
- Professional UI now owns Shipyard/Edit/View/Add menu descriptors and quick actions.
- Generator is a normal Build/Systems/PCG panel rather than an advanced-only concept.
- `F3`, `F12`, and `Ctrl+Space` have stable command IDs in the shared command registry.
- `ShipyardProfessionalController` now owns and rebuilds the professional shell projection and can open normalized panels through command IDs.

## Migration boundary

The visible legacy `ShipyardBuilderSystem::BuildControls()` renderer is intentionally not deleted in this tranche. It remains compatibility authority while the shell projection becomes complete enough for a one-way renderer cutover. No second asset/search/layout state system should be added to the legacy path.

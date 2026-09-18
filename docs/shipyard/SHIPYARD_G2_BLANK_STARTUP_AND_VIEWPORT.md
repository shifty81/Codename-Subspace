# Subspace Shipyard G2 — empty studio document and viewport grounding

This incremental PCC payload targets the **applied** Asset Visibility fixes and G1
unified gizmo over Git baseline `cf0bf03aa553ec3ae00a4965572c70085f2664ba`.
It does not replace the PCC, does not commit, and must not be installed on a
checkout with differing preflight source hashes.

## Implemented

- Launching standalone/main-menu Shipyard or `--shipyard` opens a blank
  `shipyard.untitled` recipe: zero modules/attachments/details/anchors/
  hardpoints/articulations, no playable certification, no selected cockpit.
  In-game docked refit still chooses the existing player ship/gameplay fallback.
- The initial studio camera switches to free-fly after neutral framing rather
  than tracking a non-existent vessel. This reuses the existing camera service;
  it does **not** create a new camera architecture.
- Always-visible **NEW EMPTY** starts another blank document in standalone
  mode; refuses to destroy dirty work, clears cross-document undo and transient
  authoring state. **GENERATE** is optional and only shown when there is room
  in the header and the recipe has no modules; it remains accessible from
  Assembly properties on narrower widths.
- The renderer suppresses the player's gameplay ship while the standalone
  workbench is open and displays actual authoring recipe contents instead.
  Its blank-document banner provides first actions. The first module's drag
  ghost is rendered even when no module has yet been committed. No automatic
  hull or visual shield is emitted by an empty recipe.
- Perspective guide lines span the visible viewport's left/right edges
  instead of ending at the old 72-percent fraction. This is **not** yet a
  world-space/infinite construction grid.
- Former inert File/Edit/View/Help text is relabeled as a non-menu document
  title. The historical static gate now checks honest current chrome and
  actual NEW EMPTY command instead of demanding that fake menu label.

## Windows acceptance (NOT yet certified here)

1. Run project-owned PCC Full Gate after intake. A previous G1 and Asset
   Visibility repair must be applied first, as checked by source hashes.
2. Start standalone Shipyard from the main menu and `--shipyard` smoke: no
   premade ship, floating shield, stray NPC/player ship, or selected module.
3. Open ASSETS, drag the *first* module into the viewport: ghost is visible,
   then commit and drag its X/Y/Z handles. Generate optional example only if
   requested. Check 1280x768 and 1852x797 for toolbar collisions.
4. Edit a ship so it is dirty; NEW EMPTY must refuse until the user saves or
   explicitly discards. Open another document after clearing dirty state;
   Undo must not resurrect the old document.
5. Verify a normal gameplay/docked ship remains loaded in the restricted
   in-game Shipyard; empty standalone startup must not overwrite game state.

## Remaining before GUI certification

Real File/Edit/View/Help commands, Save/Open of an *empty* document, confirmation
for discarding unsaved work, actual panel scrollbars and asset view modes,
complete paint workflow, editable model/door geometry, world-anchored grid,
interior-only/cutaway authoring, first-person camera, and independent
`subspace_studio.exe`. A passing G2 test does not imply any of these.

Portable Linux CI on the reconstructed source tree can build the standalone G2
header/test but cannot compile the entire app because that source archive omits
`engine/include/ship_editor/ShipyardOverlayLayoutStore.h`; use Windows PCC
as authority for the complete native build and UI.

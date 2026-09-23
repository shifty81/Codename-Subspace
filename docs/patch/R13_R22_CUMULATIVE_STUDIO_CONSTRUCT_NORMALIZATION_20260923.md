# Codename Subspace R13-R22 — cumulative Studio Construct normalization

Baseline: GitHub `main` commit `351469aaf3954981c74990399f5a517798f3df9b` (`Certified QG-20260922-165907-full-9ce473fb`).

This cumulative package carries R12 forward and adds ten bounded passes. It does **not** claim a Windows full application build or final visual acceptance. The project-owned PCC remains the certification authority.

## Why this tranche exists

The reported scale behavior was not safely fixable by swapping X/Y globally. Assembly scaling edits local `scaleX/scaleY/scaleZ`, while the visible Studio gizmo was drawn primarily from the ship/root basis. Rotated modules could therefore stretch along an axis different from the handle the user followed. Native modeling also contained one wedge generator whose comment/geometry used the opposite Y-forward convention from the rest of the project. Separately, Assembly and Model were exposed as peer workspaces despite sharing selection/transform intent, and the Asset Browser header had a real mid-width search/button collision.

## Ten passes

| Pass | Implemented slice |
| --- | --- |
| R13 | Context-neutral X/Y/Z authoring vocabulary (`WIDTH/LENGTH/HEIGHT`); legacy serialized ship-oriented symmetry enums remain compatible. |
| R14 | Studio scale gizmo follows the selected model/module's actual rotated/mirrored local basis. Assembly local vectors include the renderer's anisotropic root width/length scale instead of assuming unrotated X/Y. |
| R15 | Constrained X/Y/Z scaling uses opposite-face anchoring. The dragged positive face moves while the opposite face stays fixed. Free/uniform scale remains center-based. Fixed/discrete kitbash families do not receive axis-anchor translation. |
| R16 | Visible nudge/symmetry wording becomes X/Y/Z rather than Port/Starboard/Fore/Aft/Dorsal/Ventral. Existing command IDs and save values are retained. |
| R17 | Assembly and Model become one public **CONSTRUCT** workspace. Internal Build/Model modes remain for compatibility. Geometry is entered from Construct and returns to Assembly through contextual controls. |
| R18 | Assembly and Geometry share the Select/Move/Rotate/Scale tool rail; only lower context actions differ. |
| R19 | GUI overlap cleanup: Asset Browser search reserves the panel-management button strip before sizing; the fixed tool rail becomes non-closable/non-floatable/non-resizable. |
| R20 | Native wedge primitive is normalized to the project coordinate contract: +X width/right, +Y forward, +Z up. No global axis swap is performed. |
| R21 | Adds `Subspace Kitbash Foundry`: a deterministic recipe compiler that inventories donor capabilities read-only and emits a standalone Blender worker without importing/executing archived donor scripts. |
| R22 | Adds fail-closed source migration, recovery/rollback receipts, source assertions, C++ transform-contract tests, Python tool tests and a focused pre-build gate. |

## Transaction model

The PCC payload installs new helpers, the R12 dependency repair, the updated `StudioAxisGizmo.cpp`, tests and tooling. Four large active source files are changed by an explicit transactional migration after patch intake:

- `engine/src/ship_editor/ShipyardBuilderSystem.cpp`
- `engine/src/ship_editor/ShipyardProfessionalVisibleCutover.cpp`
- `engine/src/ship_editor/ShipyardWorkspaceSystem.cpp`
- `engine/src/modeling/ShipyardModelingSystem.cpp`

The migrator checks the exact published Git blob preimages from `351469a`, preflights every edit before writing, creates recovery copies under `.subspace/recovery/`, writes files atomically, verifies post-write SHA-256, and records a receipt under `.subspace/receipts/`. Unknown drift blocks the migration. Rollback is refused if a migrated file has been edited afterwards.

## Apply before the next build

1. Place the cumulative `.patch` unextracted at the repository root and approve it through the project-owned PCC.
2. Run `scripts\subspace_studio_r22_apply.ps1` once. It is idempotent and fail-closed.
3. Run `scripts\subspace_studio_r22_gate.ps1`.
4. Only after the focused gate passes, run PCC option **1 — FULL QUALITY GATE / CERTIFY GREEN**.
5. Perform live Studio checks before publishing: rotated wing/hull scaling on X/Y/Z, mirrored piece scaling, Construct Assembly↔Geometry switching, Assets panel at narrow/mid widths, floating Outliner/Properties, maximize/restore, undo/redo and save/reopen.

## Scale acceptance

For a rotated part, the handle labeled X must alter local width (`scaleX`), Y local length (`scaleY`), and Z local height (`scaleZ`) while the drawn handle follows that actual rotated local dimension. Dragging a positive axis handle in constrained Scale keeps the opposite face stationary. Uniform/free scale remains center-based.

This change intentionally does **not** reinterpret saved X/Y coordinates. Existing sockets, propulsion rules, blueprint transforms and mirror semantics keep their serialized coordinate contract.

## Construct workspace acceptance

The permanent top workflow is Construct / Interior / Systems / Appearance / Test. Construct shows Assembly or Geometry as a context/submode, not a second top-level editor. Both submodes use the same core transform tool rail. Legacy Build/Model enums remain internal until document migration proves they can be retired safely.

## Kitbash Foundry

Example recipe: `docs/shipyard/KITBASH_FOUNDRY_RECIPE_EXAMPLE_R21.json`.

Commands:

```powershell
python tools\blender\subspace_kitbash_foundry.py inventory docs\shipyard\BLENDER_DONOR_INVENTORY_20260922.json Builds\kitbash_foundry_capabilities.json
python tools\blender\subspace_kitbash_foundry.py validate docs\shipyard\KITBASH_FOUNDRY_RECIPE_EXAMPLE_R21.json
python tools\blender\subspace_kitbash_foundry.py emit-worker docs\shipyard\KITBASH_FOUNDRY_RECIPE_EXAMPLE_R21.json Builds\kitbash_worker.py
blender --background --python Builds\kitbash_worker.py -- Builds\example.frigate.engine_pod.glb
```

The worker writes Subspace part/socket metadata as Blender custom properties and emits GLB. Blender remains an optional authoring worker; the game/runtime does not depend on Blender.

## Still intentionally pending

- Windows/GPU visual proof of the revised scale gizmo.
- A true per-frame immutable layout snapshot shared by every renderer/picker/panel path; this tranche removes concrete overlap sources but does not claim all GUI geometry duplication is gone.
- Face/edge/vertex production topology tools, Boolean/UV production backends and socket-surface derivation.
- Full unified Studio document schema replacing separate `.subspace_ship` and `.subspace_studio` authorities.
- Live exemplar promotion into the PCG runtime; R10/R11 learning artifacts remain review-only.
- Blender execution certification against the user's installed Blender and final visual/material acceptance.

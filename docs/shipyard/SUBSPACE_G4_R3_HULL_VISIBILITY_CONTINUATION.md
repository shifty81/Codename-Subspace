# Subspace G4 R3 — cumulative hull visibility + view routing

**Patch transport:** direct root-drop `.patch`, ZIP container with `PATCH_MANIFEST.json`; no extraction, helper program or PowerShell parameters. Exact Git baseline `cf5107e4b36881ebfbcb46325a31558e84979597`. This contains **the entire G4 R2 hull-visibility fix plus the R3 changes** from that baseline, not an entire source rollup or the previously committed G4/G5 foundation. Apply **this patch alone** in place of R2 if neither was applied. If R2 is applied but uncommitted, PCC must approve the overlay; if local HEAD is different or PCC refuses, do not force, reset or overwrite.

## Audit defects and remedies

- Previously, the clicked TEST workspace command (`WorkspaceDevWorld`, -1268) returned early without the R2 view transition. It now restores Exterior from Interior-only, while preserving Cutaway and X-Ray.
- Keyboard workspace cycling and ordinary workspace tab commands retain the R2 transition logic.
- Model/assembly user actions that should reveal an exterior ship (AddModule, ConfirmPlacement, SelectPlaced, FrameSelected, FrameShip, GenerateVariant, NewEmptyDocument) now use the shared view policy after **successful** actions in non-Interior workspaces; failures do not switch views.
- Active Build drag/staged placement cannot be hidden by a viewport mode cycle into Interior-only. Status explains why the requested hidden mode was refused; view changes remain possible otherwise.
- Direct PCG reroll branch uses the visibility policy after successful generation. No generated ship is inserted on failure.
- R2 policy is retained: transitioning out of Interior-only reveals hulls without overwriting deliberate Cutaway/X-Ray.

## Remaining limitation, do not mark fixed

`BeginCatalogDrag()` is implemented in the separate builder translation unit and can be called directly from mouse input. This source-only pass does not intercept the very first drag initiation when the user had *already* manually selected Interior-only in Build; EXTERIOR is still the immediate recovery. The end-to-end viewport must be tested before calling the reported blank-viewport incident closed. Missing certified mesh assets can independently suppress geometry: check logs for `SHIPYARD_RUNTIME_NOT_READY` and `Could not load certified Shipyard module` if EXTERIOR is still blank. No renderer/asset bypass is introduced.

## Certification and next pass

Run PCC Full Quality Gate (Windows) and then visually test: INTERIOR -> BUILD -> first hull ghost and commit, INTERIOR -> TEST tab, keyboard cycling, Outliner select/frame, EXTERIOR/CUTAWAY/X-RAY persistence, mouse drag while toggling views, NEW EMPTY and GENERATE, and in-game docked Shipyard / cockpit input. Do not commit until visible result and gate pass. Focused C++ tests here check view-policy only; GUI source dispatch has been checked via exact source assertions but full Windows build/visual tests have not been run here. Physical command-seat integration, Blender ortho/gizmo controls and full document save remain outstanding.

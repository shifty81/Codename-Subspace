# Codename Subspace R23-R32 — cumulative Studio Construct cutover

Target certified baseline: `206976365c3bc7364f06315a086d9bf31753b50f` (`Certified QG-20260923-081311-full-cd95dc2c`).

The previous R22 commit contains the transform-basis/gizmo foundation and the migration tooling, but the four deferred large-source rewrites were not present in the committed Git tree. This tranche closes that gap and makes the cutover mandatory for ProjectOps certification.

## Passes

| Pass | Implemented change |
| --- | --- |
| R23 | Absorb the deferred R13-R22 source migration directly into the new transaction. Exact baseline/source drift fails closed. |
| R24 | Generic transform-space presentation: `VIEW / PARENT / OBJECT` replaces `CAMERA / SHIP / LOCAL` in authoring UI/status without changing serialized enums. |
| R25 | Primary Studio workflow is one row: `CONSTRUCT / INTERIOR / SYSTEMS / APPEARANCE / TEST / DEV`. Assembly and Geometry remain Construct submodes. |
| R26 | DEV no longer creates a second row over the viewport. DEV temporarily replaces the workspace strip and provides `BACK` on the same row. |
| R27 | Construct keeps one Select/Move/Rotate/Scale rail with contextual `GEOMETRY` / `ASSEMBLY` transitions. |
| R28 | Asset Browser secondary actions become responsive and disappear before they collide with preset/density controls. Search continues to reserve panel chrome. |
| R29 | Tool rail remains fixed shell chrome: non-closable, non-floatable, non-resizable. Public workspace list no longer advertises Model as a peer. |
| R30 | +Y-forward primitive normalization and opposite-face constrained scaling become mandatory source assertions rather than optional migration intent. |
| R31 | Adds one-command authored-ship exemplar review bundles: native candidate + audit + observational family grammar + provenance manifest. Runtime promotion remains prohibited. |
| R32 | Adds ProjectOps-discovered static certification and focused C++/Python tests. Full Gate cannot go GREEN while the old second-row DEV / peer MODEL paths remain. |

## Apply and certify

1. Apply the `.patch` through the project-owned PCC.
2. Run `scripts\subspace_studio_r32_gate.ps1`. The focused gate **applies the transactional source cutover automatically**; there is no separate apply step to forget.
3. Run PCC option **1 — FULL QUALITY GATE / CERTIFY GREEN**.
4. If GREEN, visually test at 1120x740, 1600x900, maximized, and at the Windows DPI values you normally use.
5. Commit/push only after visual acceptance.

The new static gate lives under `tools/control/static-gates`, so normal ProjectOps static discovery will fail the Full Gate if the source cutover has not actually happened.

## Visual acceptance

- Top-level Assembly and Model tabs are gone; `CONSTRUCT` remains visible while switching Assembly/Geometry submodes.
- DEV does not add a second row or move viewport header content downward/under it.
- Scale X changes local width, Y local length, Z local height on rotated and mirrored modules.
- Constrained axis scale keeps the opposite face stationary; free/uniform scaling remains center-based.
- Transform space reads `VIEW`, `PARENT`, `OBJECT`.
- Narrow Asset Browser does not overlap density/favorite/clear/place/check or panel-management controls.
- Fixed tool rail cannot float across the viewport.
- Undo/redo and save/reopen retain transforms.

## Authored ship learning review bundle

Example:

```powershell
python tools\shipyard\exemplar_review_pipeline.py `
  --ship content\ships\blueprints\reference_frigate.subspace_ship `
  --out-dir Builds\ExemplarReview\reference_frigate `
  --grammar-id reference.frigate.family.v1
```

The output contains `intake/exemplar_candidate.json`, `intake/exemplar_audit.json`, `grammar/family_grammar.json`, and `review_manifest.json`. The manifest explicitly records `promotionAllowed: false` and `runtimeInstallAllowed: false`; physical geometry/socket/interior/function certification and explicit designer approval remain required before live PCG use.

## Intentionally still pending

This tranche does not claim production vertex/edge/face editing, Boolean/UV completion, a single all-data Studio document format, runtime exemplar promotion, or Blender visual certification. Those should follow only after this Construct/GUI/transform cutover is visually stable.

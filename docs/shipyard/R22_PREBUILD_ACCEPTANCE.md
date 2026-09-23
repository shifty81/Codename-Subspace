# R22 pre-build acceptance

Do not call this tranche GREEN until all items below pass on Windows.

1. PCC patch intake succeeds without force/overwrite warnings.
2. `scripts\subspace_studio_r22_apply.ps1` reports APPLIED or already present and writes/retains a migration receipt.
3. `scripts\subspace_studio_r22_gate.ps1` passes Python and C++ focused checks.
4. PCC Full Quality Gate passes from the migrated checkout.
5. Studio: select an unrotated module; constrained X/Y/Z scale edits width/length/height respectively.
6. Studio: rotate a module 90 degrees; X/Y scale handles follow the rotated object, not the ship/root axes.
7. Studio: mirror the module; the displayed scale axis follows the mirrored visual basis.
8. Constrained scale keeps the opposite face stationary; free/uniform scale stays center-based.
9. Top workflow shows CONSTRUCT rather than separate Assembly+Model peer tabs; Geometry remains reachable and returns to Assembly.
10. At 1120x740, 1280x768, 1600x900, 1920x1080 and at least 150% Windows scale, Asset search/clear does not overlap Collapse/Float/Pin/Close.
11. Tool rail cannot be floated/closed into an overlapping state.
12. Save/reopen and undo/redo preserve the authored transform; no X/Y serialization swap occurred.

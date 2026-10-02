# NullHarbor / Codename Subspace — R181 semantic overlay reconciliation

## Trigger

Windows Full Gate after R180 stopped before CMake with:

`R33-R42 NORMALIZATION BLOCKED: three-way merge conflict for engine/include/input/InputState.h`

This is an expected fail-closed outcome from R180's generic textual merge. The conflict is semantic-free: certified September 30 Studio source appended `DccCycleTransformSpace` and `DccDuplicateSelection`, while R178/R179 appended context-specific Pilot/FPS/Fleet actions and `PlanetaryCommandCycleOverlay` at the same enum tail.

## Correction

R181 replaces the generic `git merge-file` strategy with deterministic semantic reconstruction. The seven affected files are rebuilt from the exact certified GitHub `8484a08` blobs, then only the intended R178/R179 deltas are applied at exact anchors. This preserves all later Studio source by construction rather than asking a line-oriented merge algorithm to infer intent.

The repair remains transactional, backs up every current postimage, validates exact pre-state blob identities, validates Studio plus gameplay postconditions, and fails closed on mixed or unknown local states. The existing six Studio source gates remain unchanged and run after reconciliation.

## Seven repaired files

- `engine/include/input/InputState.h`
- `engine/include/interior/ShipEmbodimentSystem.h`
- `engine/include/ui/RuntimeControlContextSystem.h`
- `engine/src/ui/RuntimeControlContextSystem.cpp`
- `engine/src/application/NativeBattlefieldRenderer.cpp`
- `engine/src/application/NativeGameApplication.cpp`
- `engine/src/platform/NativeWindow.cpp`

No rollback of R178/R179, no Git reset/checkout/stash/force, and no weakening of Studio certification are permitted.

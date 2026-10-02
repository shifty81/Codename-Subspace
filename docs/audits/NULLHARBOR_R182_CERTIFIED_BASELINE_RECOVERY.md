# NullHarbor / Codename Subspace R182 — Certified Baseline Recovery

## Purpose
R182 supersedes the temporary R180/R181 repair strategy and closes the R178/R179 source-generation regression.

R178/R179 correctly targeted Git `8484a081f8d60be980d3aac300ba2f1f9397cd75`, but seven full-file overlays were authored from older certified September source bodies. The Git HEAD remained correct and the PCC gates prevented certification. R180 then failed closed on a legitimate text merge conflict in `InputState.h`. R181 switched to semantic reconstruction, but incorrectly verified standalone Studio transform tokens in `NativeGameApplication.cpp` instead of their real current owner, `engine/src/studio/StudioApplication.cpp` and Shipyard transform systems.

## R182 authority
- Forward source authority: GitHub `shifty81/Codename-Subspace` main @ `8484a081f8d60be980d3aac300ba2f1f9397cd75`.
- Development identity: Codename Subspace.
- Destination product identity: NullHarbor.
- September donor/snapshot sources are provenance only and are never promoted over the certified Git baseline.

## Recovery behavior
Before any write R182 proves the current Studio R82 transform authority in its actual owner files:
- `engine/src/studio/StudioApplication.cpp`
- `engine/include/ship_editor/ShipyardBuilderSystem.h`
- `engine/src/ship_editor/ShipyardBuilderSystem.cpp`
- `engine/src/ship_editor/ShipyardTransformSystem.cpp`

It then recognizes only the exact known R178/R179 postimages of seven affected files, reads their certified September 30 versions with read-only `git show`, verifies the certified blob SHA, applies the explicit R178/R179 semantic forward-port, validates gameplay postconditions, backs up every current file, and writes the recovered set transactionally.

Affected files:
1. `engine/include/input/InputState.h`
2. `engine/include/interior/ShipEmbodimentSystem.h`
3. `engine/include/ui/RuntimeControlContextSystem.h`
4. `engine/src/ui/RuntimeControlContextSystem.cpp`
5. `engine/src/application/NativeBattlefieldRenderer.cpp`
6. `engine/src/application/NativeGameApplication.cpp`
7. `engine/src/platform/NativeWindow.cpp`

The application postcondition deliberately does **not** claim ownership of `StudioTransformMoveDelta::AssemblyAuthored`, `StudioTransformMoveDelta::ModelAuthored`, or `TranslateSelectedResolvedParent`. Those are certified where they actually live. The existing historical Studio gates remain unchanged and run after recovery.

## Safety
R182 performs no reset, checkout, stash, force operation, broad source replacement, or historical-gate bypass. Unknown or partial working states fail closed. Recovery receipts and preimages are retained under `artifacts/gates/migrations/r182_certified_baseline_recovery/`.

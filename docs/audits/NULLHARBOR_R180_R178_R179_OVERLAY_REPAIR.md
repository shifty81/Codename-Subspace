# NullHarbor / Codename Subspace — R180 R178/R179 overlay reconciliation

## Why R180 exists

The authoritative repository remained `shifty81/Codename-Subspace` at certified Git commit
`8484a081f8d60be980d3aac300ba2f1f9397cd75`. R178 and R179 added valid gameplay work, but seven
existing files in those patch payloads were authored from the earlier September 17 source export.
Those full-file overlays could therefore replace later Studio/input/runtime changes that existed on
GitHub main even though the gameplay additions themselves were valid.

The first Windows symptom was the R33-R42 prebuild proof identifying R82R1 final source while
`pass1521_1530_studio_transform_authority.cmake` rejected the overlaid application/renderer state.
Weakening that gate would certify missing source and is explicitly rejected.

## Reconciliation model

R180 adds a narrowly bounded three-way repair. For each affected file:

1. `git show 8484a08:<path>` supplies the certified September 30 baseline.
2. The exact certified historical source revision supplies the merge ancestor.
3. The current working file supplies the intended R178/R179 delta.
4. `git merge-file` combines both histories.
5. Studio R82 and R178/R179 gameplay markers must all survive.
6. Every source file is backed up before transactional replacement.
7. Unknown or partial postimages fail closed rather than being guessed or overwritten.

Historical merge bases are `a3221000ba127bc1969aa8f1b220ea419dd1d7c1` for the input/runtime headers and
`3cfe9676b80ba283682d99a7604c3403ce965fb2` for the application/renderer pair.

## Affected files

- `engine/include/input/InputState.h`
- `engine/include/interior/ShipEmbodimentSystem.h`
- `engine/include/ui/RuntimeControlContextSystem.h`
- `engine/src/ui/RuntimeControlContextSystem.cpp`
- `engine/src/application/NativeBattlefieldRenderer.cpp`
- `engine/src/application/NativeGameApplication.cpp`
- `engine/src/platform/NativeWindow.cpp`

No Studio gate is disabled. No Git reset, checkout, stash, or force operation is used. Git is read as
certified source provenance only; the working files are updated transactionally by the repair tool.

## Expected Windows flow

Apply R180 on top of the uncommitted R178+R179 working state at Git HEAD `8484a08`, then run the
normal PCC Full Quality Gate. During `Studio bulk polish normalization`, the R180 hook reconciles the
seven exact known overlays before the six historical Studio source gates execute. A successful run
writes an idempotent receipt under `artifacts/gates/migrations/r180_overlay_repair/`.

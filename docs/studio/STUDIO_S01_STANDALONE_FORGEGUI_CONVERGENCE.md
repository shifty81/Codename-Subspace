# Subspace Studio S01 — native executable and ForgeGUI shell convergence

## Exact input and preservation

Baseline: certified `15758e1672ad0d592f10623852e006d0cc3b4957` on Codename-Subspace `main`, which already **contains all R2–R4 hull visibility changes**. This incremental source handoff is cumulative in effect from the current committed source, but does not re-overlay R4 files. Do not apply old R2/R3/R4 patches again. If Git HEAD advances or source files are edited locally, do not force a mismatch; rebase against the new exact source.

## Implemented this pass

- CMake produces `subspace_game` **and** `subspace_studio`, each with a different `main()`. The Studio executable always selects standalone Shipyard from its own entry point; it rejects gameplay-only switches. `--studio-smoke` exercises the existing native eight-frame Shipyard smoke. `--help` is GPU-independent and registered as a CTest.
- The existing C++ NativeGameApplication, OpenGL renderer and NativeWindow translation units are compiled **once** in `subspace_native_host` (CMake OBJECT library) and linked into both executables. The simulation, renderer, module catalog and blueprint contracts remain single-authority; no game fork, copied gameplay source or dependency on ForgeGUI's Rust Cargo workspace.
- `tools/studio/LaunchSubspaceStudio.cmd` launches the output from the correct working directory once the PCC has built it. Missing binaries fail clearly; launcher does not auto-build, auto-patch or overwrite a source tree.
- Certified base `15758e` already contains R2, R3 and R4 viewport visibility corrections, including first hull catalog press/drag. S01 touches no ship-builder implementation, gameplay camera, PCC or blueprint schema.

## ForgeGUI reference and practical UI direction

Reference: independent `shifty81/ForgeGUI_Core` at Git `532f7e1a0dfe4d7eefc46680a1567242059640a7`, `docs/forge_gui/FORGEGUI_CANVAS_FIRST_SHELL_STANDARD.md` and README at version 0.4.8. This pass adopts *concepts*, not runtime Rust integration; no C ABI or render-surface interop has been certified.

Target Studio composition (one owning shell, no nested editor shell):

1. Dedicated native title bar, ordinary window move/resize/maximize controls and clearly separate File/Edit/View/Help menu bar. The window must be independently launchable; project file title and dirty marker should be visible.
2. One compact workflow navigator: **Build → Model → Interior → Systems → Paint → Test**. On default launch, show Build, make advanced developer tools discoverable from one explicit menu (not an always-open cluster of duplicate tabs). Restore the last *approved* layout per Studio profile; RESET UI changes only layout, not ship data.
3. One large persistent 3D assembly viewport with shared projection/picking, orientation controls, frame selection, grid/snap and unobstructed drag ghost. No viewport resizing when docking/floating tool panels; body occlusion must prevent click-through.
4. Left rail: Select / Move / Rotate / Scale, grouped with contextual tools and tooltips; commands must have real behavior and undo/redo. Right contextual surface: selection summary, transform/socket/appearance details. Outliner is the document hierarchy, not another asset catalog. Bottom Assets tray is searchable, filterable and scrollable; pressing a card stages a ghost rather than silently committing a part.
5. Minimal always-visible document/status footer: unsaved changes, validation/readiness, selected module, active input profile and actionable errors. Clear NOT READY explanations rather than placeholder UI.

## Authority and staged migration

`subspace_studio.exe` is a **distinct native executable**, but S01 still delegates to `NativeGameApplication` internally. It is not yet a dedicated independent Studio app host, not a wholesale ForgeGUI port, not a certified improved GUI, and not a completed File Open/Save As round trip. This first migration creates the build/run boundary safely. Follow-up work extracts Studio-owned window/menu/navigation/compositor orchestration behind a shared native application interface; the game retains a thin refit interface. An optional Rust/C++ FFI bridge may be designed *only* if a versioned stable ABI and viewport texture/input/picking handshake are validated; do not vendor the ForgeGUI repo or run two independently competing GUI/render loops.

## Certification and acceptance

Source-only checks in the packaging environment are not Windows certification. On the user's machine: approve this S01 patch through project-owned PCC; run Full Gate to build both binaries and execute the new help test; launch `tools\studio\LaunchSubspaceStudio.cmd` and verify an empty Shipyard without starting gameplay. In Studio test Interior → Build → first hull drag → ghost/Place → Outliner → Frame; check Cutaway/X-Ray, save blueprint and undo. Then run `subspace_game.exe` and confirm normal game boot, cockpit input and in-game docked refit are unchanged. Do not commit/push without a fresh GREEN fingerprint **and** visual acceptance. If PCC refuses baseline, stop; do not force/restore older source.

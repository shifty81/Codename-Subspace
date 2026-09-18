# Subspace Studio — S02 through S10 implementation and two-pass audit

**Baseline:** `b6bf8378cdf3af591c5164424fb2b58b1237fda9` (S01 GREEN, already contains R2–R4). This is an incremental root-drop PCC update against S01. Do not reapply S01, R2, R3 or R4.

## Exactly what changed

- S02 **implemented, awaiting Windows certification**: `studio_main.cpp` constructs `StudioApplication`, never `NativeGameApplication`. Studio owns a native window, pointer and keyboard routing, renderer lifecycle and an empty non-gameplay scene. Its authoring renderer, materialized catalog and builder remain the shared native C++ production systems. No duplicated gameplay implementation or Rust/egui dependency.
- S02 input **implemented, awaiting Windows visual confirmation**: Esc is consumed by a Studio-only policy, cancelling socket edits / catalog staging / transforms before dismissing the command palette or menu. Idle Esc does not navigate to the game main menu. Game input handling is untouched.
- S03 **partial**: retains the C++ ForgeGUI-inspired shell already implemented in Subspace, with the existing real panel compositor, hit testing, dock drag, asset search, viewport/camera routing and contextual builder controls. This pass does not falsely claim the interface was fully redesigned or ForgeGUI Rust controls embedded.
- S04 **partial**: document owner uses the existing `.subspace_ship` blueprint schema, `--open <file>`, protected New, unique-name draft Save into repository `dist/blueprints`, transactional temporary-file verification and recovery on failed replacement, plus ordinary builder controls/hotkeys. Open/Save As **file dialogs**, configurable recent documents and editable modeling/interior serialization remain outstanding. Document New/Open retains dock layout.
- S05 **partial**: Studio routes catalog press through select-before-threshold drag, authoring placement, viewport selection, module/socket transforms, undo/redo, mouse orbit/pan/zoom and dock occlusion to the shared editor systems. It does NOT implement vertex/edge/face topology editing or the full 6DOF camera/gizmo parity from the in-game editor.
- S06 **implemented geometry subset, not a complete modeler**: canonical Wing is a five-point finite-thickness planform; HullSegment is a tapered closed volume; Pipe/Tube/Ring/TurretRing have inner/outer walls and end surfaces. Mirror and LinearArray evaluate to real additional canonical geometry and node transforms in modifier order, capped at 128 shapes. Non-evaluated Bevel/Boolean/Inset/Extrude/etc. fail validation instead of silently publishing unchanged geometry; invalid geometry bakes to a rejected empty asset. No fake geometry fallback.
- S07 **partial**: canonical blueprint Save/Load round trip exists. `ModelPublishCanonical` still only creates an in-memory baked asset: the model recipe persistence, content catalog publication, imported asset registration, reload and runtime use remain incomplete. Never claim a modeled module is published merely from that command's status string.
- S08 **partial**: Studio loads persistent socket/definition overrides from the same `content/authoring` files as the game, handles existing editor save requests and refreshes the authoritative renderer catalog. Interior regeneration, clearance, collision and walkable traversal certification are still required.
- S09 **not implemented in this patch**: painting/decals remain in the existing builder; no new per-face UV paint editor or final screenshot certification.
- S10 **two source-only double-check audits in this handoff; local Windows acceptance pending**. This heading is a milestone boundary, not a completion claim for every subsystem above.

## Audit pass A — source and build compatibility

1. The two changed pre-existing source targets are `engine/CMakeLists.txt` and `engine/src/studio_main.cpp`; `ShipyardModelingSystem.cpp` is additionally changed and is known to match the GitHub baseline input Git blob before edits. All other production code is new `engine/include/studio` or `engine/src/studio`; game application, game main, game renderer and gameplay source files are unchanged.
2. All three newly introduced Studio C++ production translation units and the modeling implementation received C++17 `-Wall -Wextra -Werror -fsyntax-only` checks (newer interface declarations were projected into a local test-only overlay because the attached archive is older than GitHub; those test overlay files are NOT shipped).
3. Focused tests: Studio escape/discard/publish policy 15 assertions; geometry, reflection, array, winding and invalid-geometry refusal 21 assertions; native blueprint serialization 7 assertions. Counts are local isolated tests, not a PCC Full Gate.
4. CMake configure with tests OFF succeeds against the available older source snapshot. Full configure with tests ON is inconclusive in this environment because the older source ZIP lacks current upstream test files referenced by the certified CMakeLists. Do not delete those current CMake targets to silence an older-snapshot error.
5. Windows compile, live rendering, existing full suite, asset hydration, PCC source fingerprint and local visual acceptance still require the user's real repository build.

## Audit pass B — workflow, source authority and data safety

- Studio executable has no path back to `NativeGameApplication` or the gameplay frontend. It renders a shared certified catalog but does not spawn a sector/player or enable live Apply/refit; idle Esc stays in Studio.
- The existing game entrypoint and game controller are not changed. Shared source files are modified only where needed for model shape fidelity, ensuring game runtime uses the same canonical geometry and validation rules.
- The same upstream blueprint codec is used to load and write, and a save is verified by loading its staged bytes before moving it over the target. Existing documents receive a recovery rename, with rollback attempted if final promotion fails; refused/recovery files are never auto-deleted on failure.
- Opening an unavailable-module blueprint fails with a specific module ID rather than loading an invisible ship silently. New and Open refuse dirty-document replacement. Creating the first Save path fails closed if project root cannot be established.
- Shared catalog authoring overrides load at launch and are reapplied to the renderer when saved. Input on floating panel body must not pick hulls below it; catalog click does not silently add a part.
- Intentional fail-closed limitations: no animated interior editor certification, no topology mode, no automatic bevel/Boolean registration, no model recipe Save/Open bridge, no real File Open dialog, no module collision rebuild from newly modeled geometry, no Windows visual certification.

## Local acceptance sequence (one patch only)

1. Root-drop this `.patch` unextracted into the S01 GREEN repository and approve it through project-owned PCC. Stop for a baseline/checksum mismatch; do not extract or overwrite files manually.
2. Run PCC option 1 to complete the FULL QUALITY GATE on Windows. If GREEN, launch `tools\\studio\\LaunchSubspaceStudio.cmd` and verify the title, blank draft, asset catalog and visible 3D viewport.
3. In Studio, press Esc at idle, then with a catalog drag active, then with a popup open; none may take you to the game main menu. Click and drag a part, stage/confirm, move/undo/redo, orbit/pan/zoom, float a panel and click its body to verify no hull click-through.
4. Save a blueprint; verify it appears at repository `dist/blueprints`, exit and reopen via `LaunchSubspaceStudio.cmd --open "C:\\path\\to\\blueprint.subspace_ship"`. Verify module IDs, transforms and appearance survive. Save a second time and confirm the previous good file is preserved on failure.
5. In Model, review Wing/HullSegment/Tube canonical shapes; Mirror and Array now bake, while unsupported Bevel/Boolean must explicitly fail validation. Existing source model status is NOT equivalent to catalog publication.
6. Launch the regular game and perform original main-menu, cockpit, docking and in-game refit checks. Only after Windows build AND visual acceptance is a commit/push appropriate.

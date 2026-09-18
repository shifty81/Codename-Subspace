# Subspace Studio S12 — document workflow / safe blueprint recovery

Baseline: `shifty81/Codename-Subspace@4f6e42f3fd0836c21a132c528d8975fd6aeb71a8`, certified QG-20260918-173002-full-93e92fd0. This baseline already includes S01–S11. This is an **incremental** patch from that source, cumulative by retention, not an older-baseline rollup. Do not reapply S11 or S02–S10.

## Implemented

- `Ctrl+O` opens a real Windows blueprint file chooser. A document with unsaved blueprint, socket/definition, model or interior edits blocks opening *before* the chooser. Existing module availability verification remains in `StudioDocumentStore::Open`; failed load cannot replace the active document.
- `Ctrl+N` creates an empty document only when no unsaved blueprint or authoring edits would be discarded. `Ctrl+S` saves the active document; the first save opens Save As, requiring a chosen path rather than silently generating a timestamped draft. `Ctrl+Shift+S` opens a native Save As chooser. Windows native common dialogs are linked to `subspace_studio` only through `comdlg32`; the game and ForgePY source are unchanged.
- Distinct shortcut routing lives in Studio. `S` without Ctrl retains the existing Scale tool; the shared game `NativeWindow` key mapping is not changed. File chooser cancellation is a no-op. Errors show a user-visible native warning and builder error status without marking edits clean.
- Closing Studio with an unsaved ship assembly attempts to write a **separate, uniquely named**, canonical `.subspace_ship` blueprint under `dist/blueprints/recovery/`. Existing blueprint path is restored after the recovery write; neither the original document nor any previous recovery is overwritten. The established pending-file/verification/rollback protocol is reused. Failed recovery returns process code 6. Unpersisted socket/definition overrides or editable model/interior drafts return code 7 and print an explicit warning.
- If an ordinary document is saved successfully but its previous-version recovery backup could not be removed, it now keeps the successful document path, reports a cleanup warning, and preserves the old backup for review instead of reporting the saved document as unsaved.
- Two small C++17 tests exercise document shortcut priority and recovery path isolation. CMake registers them with CTest; the universal project-owned PCC Full Gate continues to own test/build certification.

## Known limitations — NOT fixed or certified

- The existing drawn File popup still has New/Save/Generate only. Open and Save As are **keyboard + native dialog** interactions until Studio's shared UI control schema/renderer is migrated. Do not describe these as visible File-menu buttons.
- The `.subspace_ship` format stores the ship recipe/appearance. **It does not serialize socket/definition override edits, editable primitive/model recipes or interior structural drafts.** Exit recovery is blueprint-only, not complete authoring-state recovery. Native window WM_CLOSE still cannot be canceled by Studio and an unexpected process crash cannot run this close handler. S13 should introduce a studio-owned close veto plus independent autosaved authoring-session format, then File menu actions and document dirty/title surfaces.
- A Windows MSBuild/native dialog interaction/visual acceptance and full PCC gate were not available here; exact source header projections were used only for local C++ syntax checks. No unpublished-source changes, no weakening the PCC's source-fingerprint gate or patch guard.

## Acceptance

1. On Git `4f6e42f` with clean baseline, place only the S12 `.patch` at the root, approve using the project-owned PCC or ForgePY native-provider action. Run Full Quality Gate. Do not force any baseline mismatch.
2. Launch `subspace_studio.exe`; test Ctrl+O cancel, Ctrl+N, Ctrl+S, Ctrl+Shift+S, Ctrl+O of a saved blueprint; ensure S alone still activates Scale and the game's regular boot/menu remains unchanged.
3. Place a hull, leave changes unsaved, close Studio, verify a new file in `dist/blueprints/recovery/` and that the original blueprint was not overwritten. Reopen the recovered ship using Ctrl+O. Test the incomplete model/interior recovery warning separately and do **not** rely on this file to preserve model primitives.
4. Only commit/push after a new GREEN record, visual file-dialog inspection, and gameplay regression smoke. On failure upload the PCC debug bundle.

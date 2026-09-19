# Subspace Studio S13 — independent window close guard

## Exact baseline and delivery

Input: `shifty81/Codename-Subspace@139279ca32b78cfc71f990823fc72042ff2c4883`, certified commit `QG-20260918-185237-full-ae195fec`. S01–S12 are already committed on this baseline. S13 is an **incremental** forge.patch.v1 root-drop patch; it retains all earlier changes by building on the exact committed tree and does not overlay or reapply prior passes. Do not apply S12 again. The game, ForgePY, Rust Forge, canonical blueprint schema, NativeWindow implementation, and the PCC patch engine are untouched.

## Implemented: real Studio-owned Windows WM_CLOSE interception

- `StudioNativeCloseGuard` finds exactly the Studio window on the current GUI thread by its configured title and attaches a Win32 common-controls subclass to that HWND. The Studio executable links `comctl32`; the normal game target and shared platform source do not. It removes the callback on normal shutdown and on WM_NCDESTROY; installation failure stops Studio rather than silently proceeding with unprotected close behavior. An unsupported platform fails closed.
- A close request via the title-bar X or ordinary Alt+F4 now reaches `StudioApplication::ConfirmClose()` **before** `NativeWindow` can process WM_CLOSE and post WM_QUIT. A reentrant close request while a confirmation dialog is open is vetoed.
- A clean session closes directly. A session with unsaved blueprint, socket/definition overrides, model primitives, or interior structural draft displays **Yes / No / Cancel**: Yes saves the blueprint and closes only if no other unsaved authoring state remains; No enters the recovery-close path; Cancel keeps the application and viewport open.
- Before a recovery-close containing unsupported authoring work, a **second** dialog names the model/interior/override data that is not recoverable in a `.subspace_ship` blueprint and requires explicit acknowledgement. Cancel in either dialog vetoes window close. There is no silent model/interior discard.
- The blueprint recovery file is saved and verified **before** allowing WM_CLOSE to continue. If recovery fails, the window remains open and displays the error. Once verified, the normal post-loop close path does not generate a duplicate recovery file. Ordinary clean app shutdown, game window handling, PCC/full gate and unchanged blueprint codec remain intact.

## Audit A — source and build

Original bytes for `engine/CMakeLists.txt`, `engine/include/studio/StudioApplication.h` and `engine/src/studio/StudioApplication.cpp` were reconstructed directly from the already-issued S12 package and Git SHA matched the current GitHub `main` source. No older snapshot version of these three files is overlaid.

The two new standalone focused CTest targets compile and pass using the C++17 host toolchain. Policy coverage includes clean/dirty state, failed blueprint recovery veto, separate unsupported-data acknowledgement and save-path closure. The native guard test covers non-Windows fail-closed behavior and idempotent detach. Linux native-close-guard syntax passed under `-Wall -Wextra -Werror`. CMake configuration with tests disabled succeeds on an archive-plus-overlay fixture. Full CMake configuration **with all tests enabled fails on the archived source fixture** because the September 17 archive lacks unrelated newer test source files that exist in the current GitHub branch. That fixture failure must not be called a failure of the actual current Windows repo; a current-source Full Gate remains required.

## Audit B — safety and rollout

Project ID, patch schema, exact Git baseline, each payload SHA256 and size, archive CRC, path confinement and duplicates are checked on the finished artifact. No Git operations are performed by the patch. No modifications to the game's menu, shared native window class, canonical asset/blueprint format, internal patch intake, ForgePY executable or Rust Forge repository.

**Not implemented / not certified:** Windows MSBuild, actual WM_CLOSE/title-bar/Alt+F4 behavior, native dialog visual interaction and complete PCC gate need local testing. Model geometry, modifier recipes, interiors and unsaved socket/definition overrides are **not** serialized by a blueprint or by S13. Explicit acceptance of partial data loss still permits closing; do not treat that as complete recovery. Ctrl+O/Ctrl+S from S12 remain; File menu still does not have clickable Open/Save As. There is no crash-proof autosave or complete editor-native document format yet.

## Local acceptance (before commit)

1. At clean Git `139279c`, root-drop **only S13** and approve through the existing project-owned PCC or ForgePY project-native provider. Run Full Quality Gate and require a **new** GREEN fingerprint. On baseline mismatch, do not force.
2. Open Studio, place a hull without saving and click the title-bar X. Cancel must retain the editor. Yes must open Save As for the first save and only close after a successful save. Cancel the chooser to confirm the window stays open. No must write a separate verified recovery file before the window disappears. Reopen that `.subspace_ship`.
3. Test Alt+F4 with an unsaved document, and a Studio session with a model primitive: the latter must require a second explicit warning and refuse Save/Close as complete. Use Cancel to retain work. Confirm a clean window closes immediately. Run `subspace_game.exe` to check unchanged game close behavior.
4. Commit/push only if Windows PCC is GREEN and the actual viewport/dialog tests pass. If the gate fails, send its new debug bundle. Do not reset or reapply older patches.

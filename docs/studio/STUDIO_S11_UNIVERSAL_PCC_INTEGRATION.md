# Studio S11 — universal PCC / independent Studio build integration

Exact Git baseline: `4e246069cde7227db93146f2bfeb0213823436cb` (previous Studio S02–S10 already certified and committed). This is an incremental patch, not a source rollup, and contains no older modeling payloads.

## Project-owned authority

Subspace remains independently buildable and owns its `forge.project.v1` contract, `forge.patch.v1` transactional root-drop updates, approval, rollback, Full Gate, source fingerprint, Git certification and all Windows commands. ForgePY (current operational universal frontend) discovers these commands and calls the declared interpreter; this patch neither changes nor vendors ForgePY/Rust Forge.

`project.control.json` exposes `build.studio`, `test.studio`, and `run.studio`, plus the legacy `run.shipyard` key pointing to the **independent** executable. The game stays on `run.game`. Windows command providers use `powershell.exe`, matching the existing project-owned Windows PowerShell 5.1 host; they no longer require PowerShell 7 `pwsh` to be installed. New provider keys all use native `SubspaceTools.ps1 -Action ...` dispatch, not generic CMake guesses.

## Build and runtime

- `build.studio` reuses the existing project-owned render CMake configure/build and CTest suite, then verifies `subspace_studio.exe` exists in the active render build. No second build graph.
- `run.studio`, `run.shipyard` and the root PCC Run & play submenu launch `subspace_studio.exe` from `engine/build` (or its named configuration child). No stale recursive executable search or game `--shipyard` fallback.
- `test.studio` runs `subspace_studio.exe --studio-smoke` with existing logging and nonzero exit propagation.
- The authoritative Full Gate keeps the original **game** runtime and in-game Shipyard smokes, adds an independent Studio binary check and eight-frame Studio smoke, and runs a non-mutating provider contract test before source certification.
- The root PCC menu now offers independent Studio, game and Studio smoke as distinct selections; the original advanced menu also offers Studio build and run.

## Safety

No source, patch, gate, branch, snapshot, or rollback policy is relaxed. All operations still invalidate the previous GREEN source fingerprint after a change. Approve patch explicitly, then run Full Gate, inspect native Studio and game, then commit/push only once newly certified.

## Verification scope

This package is produced from source whose Git blob hashes were verified against the exact online commit, and checked for JSON/commands, PowerShell structural token matching and ZIP payload hashes. Windows PowerShell execution, Windows CMake build, live Studio smoke and visual acceptance must run on the user's machine before claiming GREEN.

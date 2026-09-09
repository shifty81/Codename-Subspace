# Subspace Root Utility

`SubspaceTools.ps1` is the root-level project control script for the Subspace source rollup.

## Launch

From PowerShell in the repo root:

```powershell
.\SubspaceTools.ps1
```

Or double-click/run the command shim:

```cmd
SubspaceTools.cmd
```

## Main menu

The menu intentionally stays compact:

1. Status / preflight
2. Clean headless build + tests
3. Clean render build + tests
4. Incremental render build
5. Run tests only
6. Clean build outputs
7. Package debug bundle
8. Package full source rollup
9. Root audit + content normalization dry-run
10. Advanced tools

The build entries clean the logs folder before starting and write a fresh session log under `logs/sessions/`.

## Direct actions

```powershell
.\SubspaceTools.ps1 -Action status
.\SubspaceTools.ps1 -Action build-headless -Clean
.\SubspaceTools.ps1 -Action build-render -Clean
.\SubspaceTools.ps1 -Action test
.\SubspaceTools.ps1 -Action debug-bundle
.\SubspaceTools.ps1 -Action source-rollup
.\SubspaceTools.ps1 -Action root-audit
```

## Debug bundles

Use this after a failed local build:

```powershell
.\SubspaceTools.ps1 -Action debug-bundle
```

The bundle is written to:

```text
dist/debug/Subspace_DebugBundle_<timestamp>.zip
```

It collects logs, CMake caches, CTest logs, build docs, scripts, and the root utility itself.

## Source rollups

Use this when you want a clean shareable source snapshot:

```powershell
.\SubspaceTools.ps1 -Action source-rollup
```

The source zip is written to:

```text
dist/source/Codename_Subspace_Source_<timestamp>.zip
```

It excludes generated folders such as `.git`, `.vs`, `build`, `out`, `dist`, `logs`, `engine/build`, and `engine/build-headless`.

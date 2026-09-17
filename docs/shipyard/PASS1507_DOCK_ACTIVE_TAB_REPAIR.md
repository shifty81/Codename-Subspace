# PASS1507: Shipyard dock active-tab overlap — direct patch

**Baseline:** `31442f2502a89f81086aacdd136a586753bcc2d7`,
Windows PCC gate `QG-20260916-150602-full-2332d9f7`.
**Input:** user source rollup `Codename_Subspace_Source_20260916-225757.zip`.
**Scope:** one native dock implementation, engine CMake registration, one native
regression test, and this document/static gate. No project-control executable,
patch intake, imported assets, or unrelated game systems are changed.

## Behavior changed
A dock leaf is a tab *stack*, not a simultaneous overlay of every open tab.
`LayoutNode` materializes the single active visible tab. If the selected tab
is closed, floated, or auto-hidden, it uses the first available tab as a
non-mutating fallback. Closing the active tab also repairs its stored
active-tab reference. Floating remains in the existing workspace model.

## Installation — no generator, no extraction
1. Copy the `.patch` file **as-is** to the repository root beside `SubspaceTools.cmd`.
2. Launch the Project Control Center. Accept the detected patch when prompted.
3. Check option 4 for the successful patch receipt, then run option 1 Full
   Quality Gate and option 3 Shipyard. Only commit/push after both pass.
4. If no prompt appears, do not infer success: check that the `.patch` file
   is directly in the repository root, or inspect `updates/transactions` and
   `updates/failed` via PCC option 4.

The PCC validates patch manifest SHA-256 and `baseline.gitCommit` but its
existing patch engine does not enforce the `sourcePreflight` hashes below.
This package was constructed against the exact uploaded baseline versions
of both modified files. If those files were independently edited after the
source rollup, stop rather than overwriting them and rebase the repair.

## Regression evidence
`SubspacePass1507DockActiveTabTests` is part of the engine's CTest suite and
runs under the ordinary Full Quality Gate. It covers tab activation, closing,
reopening, float/redock, persistence, auto-hide, hover reveal, collapse, and
the empty dock. The ProjectOps static gate checks source and CMake wiring.

Portable focused test (developer-only, optional):

    c++ -std=c++17 -Iengine/include engine/src/ui/SubspaceUiFramework.cpp engine/tests/pass1507_dock_active_tab_tests.cpp -o pass1507_dock_test
    ./pass1507_dock_test

The test confirms native dock state/layout behavior. Actual Windows pointer
dragging and rendered panel appearance require separate interactive acceptance.
PASS1508–1515 and genuine arbitrary drag-to-dock are NOT claimed complete.

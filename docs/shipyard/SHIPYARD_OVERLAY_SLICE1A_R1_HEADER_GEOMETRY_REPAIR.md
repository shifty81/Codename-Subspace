# Shipyard Overlay Slice 1A-R1 — shared viewport toolbar/header repair

Baseline: certified `3cfe9676b80ba283682d99a7604c3403ce965fb2` plus the **already applied** Slice 1A patch. Applies only when its changes are still present; do not apply against unmodified 3cfe or locally edited versions of the two changed C++ files.

## Observed Windows failure
`Subspace_DebugBundle_20260917-033952.zip`: 103/104 CTests passed. `SubspacePass506R7UniversalBuildControlsTests` failed 2 overlap checks (20/22 assertions). DEV tab [y 24.3, h 25.2] intersects STATS [y 48.8, h 25] at 1852x797 and 1280x768. Slice 1A itself compiled and its 44-assertion regression passed, but the full quality gate is FAIL.

## Actual repair, not test weakening
`BuildControls` now anchors the viewport toolbar to the actual workspace-strip bottom and clamps button height within the view-header strip. The renderer paints the same strip based on shared metrics instead of guessing 26 pixels above the viewport. The existing PASS506R7 regression keeps both non-overlap assertions and adds explicit DEV-vs-STATS boundary checks at both resolutions.

## Install and verify
Place this `.patch` unextracted beside `SubspaceTools.cmd`; restart the project-owned PCC and approve it. Run option 1 Full Quality Gate and option 3 interactive GUI; verify no workspace/viewport toolbar overlap at 1280x768 and 1852x797. Only after success use option 2 commit + push. If the PCC refuses a source or baseline match, stop and rebase rather than bypassing policy. No donor gameplay is included in this certification repair.

## Verification boundaries
`c++ -std=c++17 -fsyntax-only` passed for all 3 modified C++ units. Shared header geometry standalone executable passed checks for 1852x797, 1280x768 and 1920x1080. Local full engine build timed out during compilation (~29%); it is not represented as a successful complete build or as Windows GREEN. PCC on Windows is final authority.

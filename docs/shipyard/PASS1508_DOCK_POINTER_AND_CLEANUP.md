# PASS1508 — Native dock pointer interaction and repository hygiene

Baseline: certified `a3221000ba127bc1969aa8f1b220ea419dd1d7c1`.

## What works in this pass

- LMB drag the TITLE AREA of Outliner, Properties, or Assets to detach into the shared floating dock model. Dragging a floating title moves the real panel; its bottom-right 18-pixel corner resizes. A five-pixel movement threshold avoids turning a click into a float.
- Release over a DIFFERENT dock leaf to dock there. Dropping over the viewport leaves a panel floating; the existing `[]` header button also toggles floating/docked.
- A dock-header gesture captures mouse input before ship/asset selection, and never steals the last ~103px of header management controls, the tool rail, or the viewport. Both standalone and in-game Shipyard use the same pointer implementation.
- Inactive panels are excluded from generated pointer controls when no rectangle is materialized; this prevents hidden tabs from stealing clicks after redocking into a shared leaf.
- Current tests exercise real shared workspace mutation, ownership validation, serialization and click-versus-drag behavior. No graphics/render certification is claimed for Linux headless builds.

## Scope and explicit limits

- This is native **in-window** floating (not separate operating-system windows). The current Shipyard renderer still owns foreground draw order and viewport clipping. More general arbitrary split creation, edge previews, arbitrary panel drawing adapters, persistent drag ghost visuals, and OS detached windows remain future work.
- Remove accidentally committed root source handoff artifacts `README_START_HERE.txt`, `Subspace_PASS1507_DockFix_Generator.zip`, and `test_generator.py`; keep the real PASS1507 test/docs/source.
- Update visible editor identity to PASS1508 while retaining PASS1505 historical baseline constants for existing gates.
- The standalone PCC owns patch approval and final Windows Full Quality Gate; do not certify or commit until it passes and pointer behavior has been inspected visually.

## Acceptance

1. From certified PASS1507, leave this .patch unextracted beside SubspaceTools.cmd; relaunch PCC and approve.
2. Run Full Quality Gate; new CTest name is `SubspacePass1508DockPointerTests`.
3. Run standalone Shipyard. Drag an Outliner TITLE from the right into the viewport. Resize its lower-right corner; drag its TITLE over the bottom shelf and release. Confirm neither movement selects a ship module nor the four tiny header buttons lose click behavior.
4. Commit/push only after gate plus manual run pass; then verify repo root contains none of the three old generator artifacts.

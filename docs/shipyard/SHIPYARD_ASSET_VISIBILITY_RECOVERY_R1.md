# Shipyard Asset Browser visibility recovery R1

**Base:** certified `cf0bf03aa553ec3ae00a4965572c70085f2664ba` (cumulative Hollow Hull + GUI recovery). This is an incremental GUI repair; do not reapply the older `02615eb` cumulative package.

## Regression and repair

The Asset Browser is closable; its visibility, collapsed state, active dock tab and floating position are persisted in `dist/user/shipyard_overlay_layout_v1.txt`. The cumulative GUI patch added scroll/search *within* Assets but failed to supply any visible control **outside** Assets to reopen it after the header X was clicked or an old saved layout hid it. A bottom leaf may also have a different active tab while `asset_browser.visible=true`. Maximized viewport hides the entire overlay.

This patch adds always-accessible **ASSETS** and **RESET UI** controls to the common viewport header (and to maximized viewport recovery). ASSETS is a reveal action, not a toggle: it opens the canonical panel, expands it, disables auto-hide, selects the proper dock tab or raises its floating window, and exits maximize. RESET UI reinstates the default workspace without modifying the ship. Both commands save layout through the existing project-owned `ShipyardOverlayLayoutStore`. The inert `View / Select / Add / Object` header text is repositioned to prevent painting over interactive recovery controls. No new GUI authority, PCC, runtime or asset catalog is introduced.

## Immediate recovery without the patch

Close the game. At the repository root, **rename** (do not delete) `dist/user/shipyard_overlay_layout_v1.txt` to a unique backup name, if that file exists, then relaunch. The missing/obsolete layout causes the runtime to retain the canonical default layout. If the viewport is maximized, first use RESTORE or Ctrl+Space. This changes only your UI layout, not ship blueprints or installed assets.

## Acceptance

1. Verify Assets displays on a fresh start; close using X and recover via ASSETS without restarting.
2. Collapse Assets, switch to another bottom tab, float Assets, and maximize viewport. ASSETS must restore the browser in every case.
3. Scroll through the full filtered catalog using the mouse wheel/arrow controls. Verify the last card appears. Restart and verify restored panel visibility persists.
4. RESET UI restores defaults and does not change ship recipe, cargo, saved blueprints or other game content.
5. Run PCC Full Quality Gate, then Run & Play. Native Windows rendering/interaction requires manual sign-off; Linux builds and focused CTests are not Windows certification.

**Limit:** this repair restores access to the existing Asset Browser. Draggable scrollbar, complete modeling controls and full workflow redesign remain separate unfinished milestones.

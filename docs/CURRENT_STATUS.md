# Current Project Status

`docs/STATUS.md` is the current status authority. This compatibility entry exists so historical links do not point at stale Pass746-era promotion instructions.

Current line: **Pass902–911 deep project alignment normalization**, based on standalone PCC R6R2 + Pass892–901 editor dock foundation.

Certification state: portable C++ configure/build and all registered CTest targets pass in the audit environment; Windows internal-PCC Full Gate is still required before the resulting tree may be called GREEN.


Active visible corrective tranche: **Pass1338 / BLENDER-DCC-001** rebases the standalone Shipyard onto a Blender-derived DCC shell and adds a real `--shipyard-smoke` Full-Gate stage. State: **IMPLEMENTED_UNCERTIFIED** until Windows PCC Full Gate and visual inspection pass.

PCC intake corrective tranche: **Pass1339 / LEGACY-ROOTDROP-RETIREMENT** teaches the standalone PCC to surface legacy ZIP handoffs, automatically archive only provably superseded Pass-numbered ZIPs outside the certification path, and continue to fail closed on ambiguous/newer legacy transports. State: **IMPLEMENTED_UNCERTIFIED** until Windows PCC startup + Full Gate verify the old Pass891 ZIP is retired and certification proceeds.

PCC strict-mode corrective revision: **Pass1339R1 / PENDING-NAME-STRICTMODE-FIX** removes unsafe collection `.Name` member-enumeration from the standalone PCC, normalizes handoff name/path resolution for PowerShell 5.1, and preserves fail-closed legacy ZIP retirement. State: **IMPLEMENTED** and retained by Pass1441 recovery.

PCC source-artifact classification revision: **Pass1339R2 / SOURCE-ARTIFACT-HANDOFF-CLASSIFICATION** classifies legacy ZIPs as update handoffs only when they contain a root `PATCH_MANIFEST.json`, preventing cumulative source rollups, snapshots, and debug bundles from blocking certification. State: **IMPLEMENTED** and retained by Pass1441 recovery.


## Pass1340–1439 cumulative Blender DCC tranche

- Portable implementation target: **BLENDER-DCC-100 | PASS1439**.
- Dedicated DCC state, Asset Browser, Outliner, Properties contexts, Blender-style hotkeys, viewport overlays/shading, command search, and maximized viewport are implemented in source.
- Portable C++ engine/game link, CTest 96/96, and ProjectOps 31/31 are GREEN in the handoff environment; Windows PCC Full Gate and rendered `--shipyard-smoke` remain final visual authority.


## Pass1440 — single-hull standard ships / multi-hull capitals

Implemented pending Windows PCC certification. Frigate, Destroyer, Cruiser, Battlecruiser,
and Battleship are locked to one certified primary hull root with substantial physical
class jumps. Carrier and larger capital classes now require multiple primary hull roots.
One-hull capitals and multi-root non-capitals fail class-generation certification and
cannot surface as safe drafts. Pass1440 also carries forward the PASS1439R1 MSVC
`ShipClassRoleSystem::ClassName` string-return fix.


## Pass1441–1442 PCC recovery and authority reconciliation

- **Pass1441 / PCC-POST-ROLLUP-RECOVERY** restored the actual Pass1339R1/R2 PCC implementation after the PASS1439 cumulative source overlay reverted those implementation files.
- **Pass1442 / PCC-STATUS-AUTHORITY-RECONCILIATION** restores the historical status markers required by the retained certification gates without changing runtime/gameplay behavior.

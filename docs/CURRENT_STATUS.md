# Current Project Status

`docs/STATUS.md` is the current status authority. This compatibility entry exists so historical links do not point at stale Pass746-era promotion instructions.

Current line: **Pass1444–1453 universal DCC asset-workbench normalization**, based on certified commit `cf3c620` / `QG-20260915-194324-full-28b90446`.

Certification state: the Pass1443 baseline is Windows PCC-certified GREEN and pushed to `origin/main`. Pass1444–1453 passes portable C++ build/link, CTest, and ProjectOps static certification in the handoff environment and requires the next Windows PCC Full Gate + rendered Shipyard visual review before promotion.


Active visible corrective tranche: **Pass1338 / BLENDER-DCC-001** rebases the standalone Shipyard onto a Blender-derived DCC shell and adds a real `--shipyard-smoke` Full-Gate stage. State: **CERTIFIED** in baseline `cf3c620`.

PCC intake corrective tranche: **Pass1339 / LEGACY-ROOTDROP-RETIREMENT** teaches the standalone PCC to surface legacy ZIP handoffs, automatically archive only provably superseded Pass-numbered ZIPs outside the certification path, and continue to fail closed on ambiguous/newer legacy transports. State: **CERTIFIED** in baseline `cf3c620`; legacy ZIP handling was subsequently hardened through Pass1339R1/R2 and Pass1441–1443.

PCC strict-mode corrective revision: **Pass1339R1 / PENDING-NAME-STRICTMODE-FIX** removes unsafe collection `.Name` member-enumeration from the standalone PCC, normalizes handoff name/path resolution for PowerShell 5.1, and preserves fail-closed legacy ZIP retirement. State: **IMPLEMENTED** and retained by Pass1441 recovery.

PCC source-artifact classification revision: **Pass1339R2 / SOURCE-ARTIFACT-HANDOFF-CLASSIFICATION** classifies legacy ZIPs as update handoffs only when they contain a root `PATCH_MANIFEST.json`, preventing cumulative source rollups, snapshots, and debug bundles from blocking certification. State: **IMPLEMENTED** and retained by Pass1441 recovery.


## Pass1340–1439 cumulative Blender DCC tranche

- Portable implementation target: **BLENDER-DCC-100 | PASS1439**.
- Dedicated DCC state, Asset Browser, Outliner, Properties contexts, Blender-style hotkeys, viewport overlays/shading, command search, and maximized viewport are implemented in source.
- Portable C++ engine/game link and CTest were GREEN in the handoff environment; the cumulative line was subsequently Windows PCC-certified and committed in baseline `cf3c620`.


## Pass1440 — single-hull standard ships / multi-hull capitals

Certified in baseline `cf3c620`. Frigate, Destroyer, Cruiser, Battlecruiser,
and Battleship are locked to one certified primary hull root with substantial physical
class jumps. Carrier and larger capital classes now require multiple primary hull roots.
One-hull capitals and multi-root non-capitals fail class-generation certification and
cannot surface as safe drafts. Pass1440 also carries forward the PASS1439R1 MSVC
`ShipClassRoleSystem::ClassName` string-return fix.


## Pass1441–1442 PCC recovery and authority reconciliation

- **Pass1441 / PCC-POST-ROLLUP-RECOVERY** restored the actual Pass1339R1/R2 PCC implementation after the PASS1439 cumulative source overlay reverted those implementation files.
- **Pass1442 / PCC-STATUS-AUTHORITY-RECONCILIATION** restores the historical status markers required by the retained certification gates without changing runtime/gameplay behavior.


## Pass1444-1453 — universal DCC asset-workbench visual normalization

Implemented pending Windows PCC certification. The standalone Shipyard is now the first live consumer of the reusable `EditorDccShellLayoutSystem` / `EditorAssetWorkbenchSystem` shell. The visible composition is viewport-first: neutral gray 3D canvas and perspective floor grid, compact left tool rail, bottom Asset Browser shelf, Outliner over Properties at right, contextual diagnostics, compact XYZ gizmo, and tighter authored-asset framing. Standalone authoring no longer paints the gameplay starfield by default; socket and shield overlays default off but remain toggleable. Current visible identity: **DCC-ASSET-SHELL | PASS1453**.

The shell defines future domains for stations, interiors, characters, props, materials, celestial/world assets, VFX, and UI. Only Ship Modules / Shipyard claims a live adapter in this tranche; other domains remain explicit adapter work rather than being reported as complete.

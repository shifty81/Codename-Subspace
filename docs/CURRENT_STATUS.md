# Current Project Status

## Current authority — 2026-10-01

The current source authority is GitHub `shifty81/Codename-Subspace` `main`, anchored for the R178 convergence milestone at commit `8484a081f8d60be980d3aac300ba2f1f9397cd75` / `QG-20260930-143924-full-53697b5b`. **Codename Subspace** remains the development/repository identity; **NullHarbor** is the destination product identity.

Active development is now the milestone-sized **R178 FPS + Universal Interiors + Strategic Command Convergence**. Its architecture authority is `content/architecture/nullharbor_convergence_authority_v1.json`; its audit is `docs/audits/NULLHARBOR_PROJECT_WIDE_RECONCILIATION_R178.md`. Donor SourceWork/NovaForge packages are reference/behavior sources only and never outrank GitHub main.

Older pass notes below are retained as lineage and must not be interpreted as newer than the Git commit/quality-gate authority above.

## R179 — Planetary Command preimage reconciliation and gameplay convergence

R179 supersedes the pending R158-R177 text-transform materializer after R178 legitimately changed `InputState.h`. Planetary Command is now materialized directly in current native source: stable planet+axial sector identities, surveyed/claimed/developed territory, contiguous owner claims, governed construction, globe/sector projections, six strategic overlays, shared render/hit-test projection, and live P/LMB/RMB/F5/Enter/Shift+Enter command flow. The historical migration entrypoint is retained only as a semantic verifier so future input/control milestones no longer need an expanding list of textual preimages.

R179 preserves R178 FPS/fleet/interior authorities and keeps the internal `PlanetaryManufacturing` workspace enum as a compatibility ID while presenting **PLANETARY COMMAND** to the player. See `docs/audits/NULLHARBOR_R179_PLANETARY_COMMAND_PREIMAGE_RECONCILIATION.md`.


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

First-class editor polish tranche: **PASS1454-1465 / FIRST-CLASS-DCC** normalizes Shipyard against the latest certified ForgeGUI_Core Creator Studio contracts (`532f7e1`) without embedding its Rust/egui runtime. It fixes PASS1453 panel/header overlap, introduces a contextual Shipyard Inspector, projects ForgeGUI-derived compact metrics and semantic surfaces into native C++, improves asset-card hierarchy/truncation, and requests dark rounded Windows non-client chrome. State: **IMPLEMENTED_UNCERTIFIED** until Windows PCC Full Gate and visual inspection pass.


## Pass1466-1505 — Shipyard Foundry

Cumulative first-class authoring tranche built on PASS1454-1465. The visible target is **SHIPYARD-FOUNDRY | PASS1505**. The tranche fixes camera pan/orbit continuity, introduces X/Y/Z transform constraints, native dock/float/pin/collapse/auto-hide/layout persistence contracts, guided workflow authority, detachable/articulated module pivots, class-scaled generated interiors, removable/modelable doors/airlocks/floors and related interior structures, expanded native primitive modeling, advanced metal/pearlescent/iridescent paint finishes, faction-aware markings, source-material suppression/fallback, and hydrated-corpus material-health auditing. State: **IMPLEMENTED_UNCERTIFIED** until the next Windows PCC Full Quality Gate and rendered workflow review.

Production Boolean execution and automatic UV unwrap remain explicitly pending backend integrations; their contracts are present but this status does not advertise them as complete. External kitbash/material packs remain governed intake sources and are not blindly vendored.

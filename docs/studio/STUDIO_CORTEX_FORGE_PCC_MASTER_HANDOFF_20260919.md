# CODENAME SUBSPACE — Studio + PCC + Forge + Cortex integrated master handoff

**Date:** 2026-09-19. **Status:** audited requirements and migration specification, not a completed Studio/PCC/Cortex/3D-generation implementation or a Windows certification. **Authoritative published Subspace SHA:** `cee9993c48f7b0d3c854096c763a77577760b63b`; user-reported S15B GREEN and screenshots are interactive feedback, not acceptance of S15C–S19. Do not overwrite a local repo whose HEAD, dirty tree, PCC fingerprint or applied packages differ. Source-bound evidence is indexed at the end. This document extends, not replaces, `Subspace_Studio_S15C_Integrated_Audit_20260919.md`.

## A. What all Subspace conversations are asking for — single product objective

A standalone, first-class **Subspace Studio** and the *same authoring capabilities* exposed within the game's shipyard: one original, cohesive sandbox/campaign where ship design, saving and discovering blueprints, modules/equipment, stations, salvage, mining, manufacturing, fittings, factions, the continuously evolving orbital universe, ship interiors and boarding eventually interoperate. The combat prototype remains a game subsystem, not the full game. Design inspiration from other games is reference-only; no copying protected visual or gameplay content. Studio must author rather than simply decorate ship models; its saved structures govern runtime and PCG.

Studio should be **one C++ authoring product**, not six executables. The primary workspaces are Assembly, Model (including Kitbash Workbench), Interior, Systems, Paint/Materials, and Test. A selected domain can supply contextual presets for Ship, Shuttle, Rover, Station, Interior, Prop, Character and later World; only implemented domains should advertise working commands. In-game and standalone editors use the same assembly, asset, transaction, physical validation, serializer and PCG authorities, with separate camera/input profiles and permissions. A studio document must preserve complete edits; `StudioDocumentStore` currently explicitly says model/interior drafts are not all persisted. The entire experience should be normal and useful **without Cortex, Forge, the internet or a generative model**.

Outside Studio: Subspace's own PCC remains its standalone authoritative build/patch/test/certification provider. The universal **Forge** operation service/CLI is the shared external multi-project coordinator; **Cortex** is the user-facing AI/project intelligence and action planner; **ForgeGUI_Core** supplies common desktop presentation and docking for the external multi-project host. This does NOT imply embedding a Rust/egui app's source or an entire AI service in the native C++ Studio. It means a versioned adapter from Studio and Subspace PCC to shared project context, typed commands, jobs, events and artifacts. A Cortex-first desktop can expose a Subspace editor integration workspace without merging Subspace runtime into Ember or making Ember a Subspace dependency. Avoid duplicate registries, command executors, project identities, model engines, independent job queues or PCCs.

## B. Source-of-truth snapshot and discrepancies to resolve first

| Component | Published evidence on 19 Sep | What is NOT established |
|---|---|---|
| Subspace | `cee9993c48f7b0d3c854096c763a77577760b63b`; commit text says certified QG-20260919-090747; `project.control.json` identifies `codename-subspace`, `forge.project.v1`, `SubspaceTools.cmd` / `.ps1`; read-only `ProjectOpsCli.py`, `ProjectCommandRegistry.ps1` exist. | Any unpublished local change, exact active running GUI binary, Windows acceptance of next passes. |
| Cortex | Published `5e5e113d6168acac721bb722d169be04b4ec1c3f`, A05 after A04. A04 gives **read-only source-anchor ForgePY parity inventory**, not behavioral parity. A05 provides guarded preview-and-approve contract migration/backups. | A06–A12 full provider, operations, GUI and Windows parity; installed GUI/session actual readiness; local unpushed versions. |
| ForgeGUI_Core | Published `eafa8e78efd54142a19e66d8be7b7d3985af23d2`. `docs/INTERNAL_PCC_STANDARD.md` calls its own provider `forge.internal_pcc.v1/0.4.13`; source mutation invalidates GREEN and provider self-update needs transaction/relaunch. | That its Rust GUI and PCC provider can be copied byte-for-byte into native Subspace or that the local app equals the published commit. |
| Forge repository | `shifty81/Forge` published `f00ca0fea2dfa29dc930fcc18ddcedc347a7c6ad` with ForgePY F797 commit title. Separate Forge Pass08C source archive has a Rust host and U9/ForgePY donor. | That published Forge equals newest local ForgePY 9.19.1 or that Rust Forge is replacement-certified. The Pass08C file audit found no Windows Rust compilation and its GUI parity false/incomplete. |
| Local archives | Sep17 Subspace source ZIP predates Sep19 SHA. `SourceWork.zip` contains a different large `SourceWork` tree; `NovaForge_Unified_R1.zip` is a NovaForge donor. | Those are NOT latest Cortex/ForgeGUI source, and must not be treated as Subspace overwrite sources. |

The latest Subspace `SubspaceTools.ps1` has `Invoke-AutoApplyUpdateInbox` as a *pending-update guard only*. Its explicit startup prompt may apply user-approved patches and restart the PCC when its own source changes; Full Gate refuses pending patches, and ordinary build can continue without auto-consuming them. An older `content/architecture/project_control_center_v2.json` still calls patch intake a full-gate stage. Resolve the stale declaration through a separate documentation/contract migration; **the live script is authority and may not be changed to reinstate silent auto-apply**. ForgeGUI's `docs/INTERNAL_PCC_STANDARD.md` models approved root intake and `PATCH_APPLIED_NEEDS_GATE`; its cargo fmt/Cargo.lock rules are Rust-project-only, not rules to import into Subspace CMake/MSVC source. The former suggestion to create a new `universal.pcc.v1` root schema is superseded by compatibility with `forge.project.v1` plus a separately versioned operation protocol.

## C. Studio input and visual correctness: prerequisite of every new tool

**Observed bug:** `StudioApplication::HandleInput()` picks and selects another module after a gizmo miss, immediately begins its transform under Move/Rotate/Scale; equivalent in-game path exists. Transform tools must pin the selected **stable instance ID**; clicking a different model or empty space is a no-op by default, no reselect and no mutation. Explicit Select mode or Outliner changes target; optional discoverable `click-to-switch-target` setting is user-controlled. Convert active gesture states (Idle → PressPending → Dragging → Validate/Commit/Cancel), not one ad-hoc condition per input handler. On press, resolve UI focus/occlusion and viewport ownership, latch target/axis/start transform/snap mode and transaction, only cross threshold begins edit. On release, record one history entry if real change. Esc, capture loss, focus loss, doc close, tool switch, stale ID => restore before state and release pointer. Win32 `SetCapture` / `WM_CAPTURECHANGED` are platform mechanisms, not substitutes for editor transaction logic. Fix `ShipyardBuilderMigrationSystem` stable IDs: exact transform equality must not be used as a durable *instance identity* policy. Restore a meaningful no-selection state instead of clamping `size_t selectedPlacedModule` to 0. Object selection, hover, multi-selection, object lock, gizmo targeting, and the Outliner/Inspector should share one source of truth.

**Gizmos:** `StudioAxisGizmo` currently uses three screen-space linear shafts even in Rotate; it is not three 3D rotation rings. Separate tool-specific handles: Move axes/planes/center; Rotate circular X/Y/Z rings/free rotation; Scale axis cubes/uniform; pivot, measure, align, sockets, symmetry and section/isolate tools follow. Renderer, hit test, screen-to-ray, hover and selection outline must use the *same subrectangle viewport* and frontmost/occlusion info. Mesh-following selected outline replaces giant cyan box. Inspector owns editable exact transforms; nominal catalog dimensions, mesh bounds, ship extents and interior clearance must be separately labeled.

**Camera:** existing S15B has RMB orbit/MMB pan and a genuine eye/pivot, but ±89° turntable vertical limit, close dolly floor of ~0.35 units, ship-wide F, no implemented short-RMB popup, and right-drag can capture over floats. Implement viewport-only navigation; cursor/surface focus and selectable pivot; F selected vs Home ship; scale-aware zoom-to-cursor, precision dolly, adaptive near clip, orbit collision and explicit cutaway for interior; preserve pose and correct picking at all layouts/DPI; optional quaternion/trackball profile is a separate certified enhancement (do not claim full six-DOF just by lifting pitch clamp).

**Attachment safety:** current socket-pair candidate code defaults `collisionRisk` without computing actual hull overlap, and free build can flag preview valid. Semantic surface positions and insertion depths alone cannot distinguish a legitimate exterior hatch/engine cavity from an embedded arbitrary component. Introduce one *shared geometry + semantic* placement gate for drag/confirm/manual transform/reattach/mirror/PCG/import/saving; verify transformed mesh or conservative shape containment/penetration, allowed contact patch, outward normals, legal insertion depth, neighboring occupancy, reserved habitable/pressure volumes and frame connectivity. Preview invalid candidate red and block confirmation; free drafts remain drafts, not runtime-certified connected parts. Doors/airlocks are only exceptions with specifically authored aperture, pressure seal, attachment portals, motion envelope and nav/traversal evidence. Don't classify the screenshot's structural part as a door solely by appearance; inspect source mesh in Workbench first. The interior generator runs after a validated assembled envelope: load-bearing shell, walkable headroom, verified portals, floors, decks, machinery exclusions, sealed door actions and connected cockpit-to-exit route. Current AABB-based carve is a stepping stone, not mesh-perfect booleans.

## D. Studio workflow/UI intended end state

| Workspace | Primary actions | Persistent panels | Contractual test |
|---|---|---|---|
| Assembly | Browse, stage, place freely, deliberate snap/attach, align, socket, symmetry, detach/reattach, hierarchy, prefab | 3D viewport + Outliner + Inspector + collapsible Assets + bottom Activity | build/save/reopen same IDs/transforms and no embedded mounts |
| Model / Kitbash | Governed import, isolate, connected-island extraction, cut/slice, primitive modeling, modifiers, pivot, sockets, collision, LOD, metadata, animated component | dedicated contextual tools + mesh hierarchy + editable operations + timeline as needed | source retained, derived module published, animated hatch opens in runtime |
| Interior | hull cutaway, decks, openings, stairs/ladders/elevators, rooms, cargo/hangars, airlocks, structure, furnishing | shared viewport + scene hierarchy + interior inspector | walk from cockpit to second room/airlock and save/reload |
| Systems | fitting, HP/damage, generators/shield, power/heat, thrusters, weapons, RCS, drones, automation | module topology + graph + inspector + diagnostics | every functional module connected and simulation validator passes |
| Paint/Materials | material library, PBR/UV audit, zone masks, multi-layer patterns, decals, faction livery, weathering, emissive | material slots, brush/color zones, preview | correct textures and masks in Studio + game, no silent fallback |
| Test | generated ship preview, stress/collision/doors, player-scale camera/FPS, runtime/campaign import, simulator results | viewport + results + diagnostics + launch | playtest authoring document end-to-end, gate receipts |

General shape: one File/Edit/View/Tools/Window/Help menu; contextual workspace tabs and left rail (distinct accurate icon, text tooltip, shortcut), central viewport, right Outliner and editable Inspector, bottom collapsible Assets/Problems/Validation/Jobs/Console. One command registry and action permission model for menu, hotkey, palette, tool rail, inspector, Cortex invocation. Inactive controls must be disabled with reason, never dead placeholders. Consistent layout/min/max/float/resize, opacity, scissor, focus and high-DPI. Don't import ForgeGUI Rust UI into C++: share presentation tokens/semantics via a native adapter or use ForgeGUI in the external Cortex/Forge desktop that hosts/references Studio as a separate application with an IPC project/document bridge.

## E. Kitbash Workbench actual pipeline

**Governed source link → download or local access → license/hash/format verify → immutable original retained → import scene/node/material hierarchy → source-inspection → mesh/material/connected-island extraction → author derived non-destructive modeling recipe → source-node animation → sockets/pivots/functional metadata → material zones and PBR → collision/LOD/interior masks → interactive preview → structural validation → human review → immutable, versioned catalog publish → runtime test.** Start with screenshot structural attachment, classify from actual source geometry, then make a deliberately authored exterior hatch. Important accepted functionality: crop/split/select actual mesh topology rather than just advertising Vertex/Edge/Face enum; extract greebles with UVs/tangents/normals/mats intact, cap cuts, preserve provenance; source kit version/licensing and derived revision; author local hinge pivot/clip/events and physical aperture/pressure/nav, damage/HP and socket interface. First small vertical slice is one reusable hatch, not arbitrary generation of a giant asset pack.

GLB/glTF is the preferred runtime/interchange derivative (hierarchy, node TRS, morph, animation, PBR); it is *not* complete editable source, so preserve `.blend`/authored procedural recipe and versioned Subspace sidecar for semantic sockets, pressure, clearance, interior carve, licensing, origin and animation action contracts. Blender automation is an optional bounded geometry worker for slicing, bevels/booleans, topology analysis, bake/UV/LOD; it should use version-pinned, reviewed templates and sandboxed paths, never execute arbitrary AI-emitted Python without inspection/approval. Reuse existing `ShipyardModelingSystem`, `UniversalKitbashAuthority`, `ShipArticulationSystem`, material auditors and asset catalogs where real behavior passes tests rather than building parallel systems.

## F. AI generation in Studio: precise semantics, not promises of arbitrary text-to-3D

Cortex is the **only visible AI broker** in the external console or optional Studio-linked contextual panel; provider identities do not become separate Studio workflows. Current Cortex published tree has service/RPC/agent/project tooling and a ComfyUI *image* provider. An image provider is **not evidence of a production text-to-mesh engine**. Treat 3D model intelligence as staged new integration; the first immediately practical capability is Cortex authoring structured `SubspaceGeometrySpec` for Subspace's own parametric/kitbash generator. Optional Blender headless/DCC converts reviewed plans into geometry. Dedicated external text/image-to-3D providers later compete behind Cortex's capability broker and receive the same preflight+validation+human-approval gates.

**Primary source-authoritative generation path**:

```
User / selected Studio module + instruction
  -> Studio immutable context snapshot [ProjectId, DocumentId, stable PartIds,
     selected mesh/sockets, units, location, existing materials, license, version]
  -> Cortex authorized request + model/provider selection + typed GenerationSpec
  -> Forge operation reservation/job/event identity [no direct project mutation]
  -> Subspace procedural builder and/or pinned Blender worker executes in staging
  -> immutable candidate assets + operation logs + progress + provenance
  -> Studio preview (before/after, placement, sockets, opening/animation, material)
  -> geometry/material/physics/interior/animation/license security certification
  -> user explicitly accepts candidate; Subspace single transaction imports/publishes
  -> document becomes dirty; source fingerprint invalidated; Full Gate later runs
  -> Cortex receives result/artifact/diagnostics tied to original request
```

**Shape-generation taxonomy:** (1) parameterized native primitives and assemblies (wedge, hull segment, plate, beam, cylinder, cutout, door frame; deterministic seed and exact units); (2) kitbash retrieval/recombination from licensed, certified Vault assets with semantic sockets; (3) scripted template-based Blender geometry/UV/LOD as worker; (4) external AI mesh as *untrusted draft* requiring cleanup; (5) reference image/concept via ComfyUI as a distinct candidate, never inferred production mesh. Give a geometry authoring action a bounded shape domain, goal dimensions, positive/negative volume constraints, symmetry, polygon and memory budgets, UV/material zones, sockets/mount normals, placement role, animation and intended runtime use. Return multiple reviewable candidates with seed, source hashes, metadata and reasons for failed validation; never auto-publish or auto-edit the current ship when inference finishes.

**Illustrative use case:** “Make a 1.2 m wide exterior access hatch on this hull and animate it.” Cortex gathers selected hull exterior normal, wall thickness, designated permission to cut, minimum walkable/pressure/egress constraints, nearby sockets. It proposes a cutter volume, frame and hinged/sliding panel with axis/pivot and motion clip; engine generates a staging-only candidate; validation checks material occupancy, pressure seal, collision sweep, aperture, avatar passage, clearance and no other module intrusion. A highlighted preview explains all failures. Only acceptance creates an authored derivative+attachment and regenerates relevant interior nav/portal data. Runtime opens/closes the door. A plain picture or loose plate attached inside hull is *not* success.

### GenerationSpec V1 proposed data contract

The companion `content/architecture/subspace_studio_generation_spec_v1.json` is a **non-executable DESIGN DRAFT** with requested fields, not a registered or implemented runtime schema. Contract: schema/version, project/document/candidate/request identities, scope and selected stable IDs + source revision, source provenance/license, category and dimensions/units/coordinate basis, external-mount and interior masks, deterministic seed/constraints/functional tags, material slots/texture authority/paint zones, animation/door/pressure specifications, staging target, provider execution metadata, validation diagnostics, human approval, publish intent and result artifact hashes. All references must be stable, all output paths stay in a staged sandbox and cannot use path traversal/symlink escapes, dimensions finite and positive, category whitelist, bounded complexity, no secrets in logs, and no silent overwrite. If selected document revision changes during long generation, candidate stays review-only and must rebase; do not attach to a different selection by index.

Cortex could also run **asset corpus audits and guided remediation**: explain why 51 components lack certified textures, choose a material zone rule from approved finishes, offer side-by-side before/after, generate masks/material derivatives, batch validate with receipts, request approval once for a bounded group. For UI, use one contextual “Cortex” surface attached to existing project/document/selection, with request, plan, live phase, candidates, diff/preview, safety, approve/reject and retry. Preserve conversation/context and show model/provider phases, tool activity, job and artifact IDs, elapsed time, failures and retry, but never hidden chain-of-thought. External Cortex offline must not stop manual Studio functions or PCC operations.

## G. Universal PCC + ForgeGUI + Forge + Cortex integration authority

The **canonical live Subspace PCC** is already registered through `project.control.json` and actual `SubspaceTools.ps1`, and offers `ProjectOpsCli.py` (`inspect`, `commands`, `describe`, `execute`, `gate`, `artifacts`, `validate`) and `ProjectCommandRegistry.ps1` (List/Describe/Validate/Json). **Extend or wrap those instead of inventing another PCC or replacing PowerShell 5.1 with unrelated Rust Cargo rules.** First gate is read-only: Cortex/Forge/GUI must all list exact `projectId`, project path/provider/version, declared and verified capabilities, `run.studio` / `build.studio` / `gate.full`, and pending update status without even constructing an executable command or touching source. Only then add normalized typed operation records and streaming execution. Maintain direct `SubspaceTools.cmd` operation offline.

```
Cortex Desktop (AI + persistent chat/project context)
  |                   Subspace Studio native C++ UI / optional Cortex panel
  |                         |
  +--- ForgeGUI views -----+---- adapters, typed API, ONE ProjectId
                |
      Forge Operation API/CLI (installed version/version negotiation)
          [command catalog, jobs, leases, events, receipts]
                |
    Subspace project-owned PCC adapter (project.control.json)
       SubspaceTools.ps1 + ProjectOpsCli.py + dedicated CLI spine
          [build, native tests, Studio smoke, Full Gate,
           patch review/apply/recover, Git, diagnostics, artifacts]
                |
          Subspace engine/editor/certification
```

**Command ownership:** Forge coordinates exact invocation/cross-app single-flight and persistently stores OperationId; Subspace owns its exact CMake/MSVC build commands, source/asset gate fingerprint, patch rules and runtime smoke; Cortex proposes and receives results; ForgeGUI renders. Neither Forge nor Cortex may synthesize a fake `gate.full` from generic CMake if an exact project provider is present. Duplicate `build.full`/`gate.full` commands should alias a single invocation, not create two gate authorities. `run.shipyard` is a compatibility alias for `run.studio`. User-triggered Build in Cortex must appear as same OperationId/log/receipt in Forge GUI/CLI and PCC. Commands require declared permissions/risk/cancel/rollback semantics. Reuse `forge.project.v1` with a separately negotiated `project_control_protocol.v2`; translate Cortex's richer typed command schema through strict dual read, no silent rewrite. A declared but unresolved command is not a working GUI button. Explicit capabilities statuses `DECLARED / RESOLVED / VERIFIED / UNAVAILABLE / FAILED` and disabled reason.

**Patch intake:** root `.patch` or ZIP containing one `.patch` → scan/check project ID/schema/baseline hashes and changed paths → stage before any write → dry-run preview and user approval → project PCC applies transaction with preimages, hashes, no symlink/path escape/duplicate entries → archive only after verified outcome → source becomes `PATCH_APPLIED_NEEDS_GATE` → Full Gate explicit → persistent GREEN receipt for exact source/asset/deps fingerprint → independently approved Git commit/push. Pending packages *block certification* and must not silently auto-apply in Full Gate. A PCC self-update must restart exact new provider only after safe handoff; all viewers subscribe to same job ID rather than executing again. Failed midapply or rollback retains recovery evidence and debug bundle. ForgePY remains reference/current universal authority until native Forge behavioral parity and explicit approval; **do not backport Rust Forge wholesale into Subspace**. Local embedded Project PCC keeps working when any external service is unavailable.

**GUI-driven development:** the Forge/Cortex integrated PCC tab displays actual project selection/browse, exact source root and HEAD/dirty status, provider selection, verified capability cards, Build/Test/Studio Run/Full Gate, patch review, jobs with streamed stdout/stderr, issue/test results, source/asset/fingerprint freshness, Git push approvals, debug bundle/artifact browser and status of Cortex model service. Clicking “Build Studio” dispatches only `build.studio` from Subspace; “Run Studio” `run.studio`; “Certify GREEN” `gate.full` with no silent intake; “Commit + push” only a separate appropriately gated command. UI restart must not lose OperationId or stream continuity. GUI installation is never a prerequisite to Subspace standalone build.

**Storage and Vault:** large third-party kitbash, ComfyUI checkpoints, Blender source and derived materials live in external governed Vault roots with SHA256/license/version and lazy on-demand hydration, not vendored into Subspace source handoff ZIP. Blueprint/material sidecars contain stable, resolvable asset IDs + content hashes. File system scan and inventory is read-only first; duplicates, uncertain provenance or destructive moves go to Review Queue. Keep source and asset fingerprints separate and combine for release certification. Log project/session/operation/build/candidate IDs with redaction and retention, not bulk embedding of every log line.

## H. Cross-core compatibility and acceptance matrix

| ID | Test scenario | PASS condition |
|---|---|---|
| PCC-01 | Subspace root offline | `SubspaceTools.cmd` status/build/test/run/gate/patch remains independently functional; prior GREEN never faked. |
| PCC-02 | Cortex/Forge read-only onboarding | Same project ID/root/declared command keys without source writes; nested project lineage and path relocation reviewed. |
| PCC-03 | GUI/CLI same operation | One OperationId, identical live logs, terminal result and gate receipt; no duplicate process from two clients. |
| PCC-04 | Concurrent source mutation | One writer lease, other queues/denied with reason; cancellation kills subprocess tree safely. |
| PCC-05 | Patch disruption | corrupt hash, wrong project/stale baseline, path traversal, duplicate entries, fail midapply, failed rollback, provider self-update all safe and evidenced. |
| PCC-06 | Certified receipt | a changed source, asset, dependency or accepted generated asset invalidates associated GREEN; Full Gate produces new fingerprint; separate guarded push. |
| PCC-07 | Provider failure | Cortex offline, Forge absent or GUI crash do not prevent PCC or local Studio; recovery state persists. |
| AI-01 | Agent model shape | typed spec -> staging geometry -> preview -> validate -> human accept -> catalog publish, stable IDs and one undo/save. |
| AI-02 | Stale selection | selection/document changed while AI runs: candidate not silently applied to new target. |
| AI-03 | Untrusted geometry | malformed units/invalid manifold/missing license/texture/penetration/huge mesh => fails with specific diagnostic, no promotion. |
| AI-04 | Door from screenshot | actual imported geometry judged by source, physically valid exterior aperture, animation clip, pressure/nav/collision runtime. |
| ST-01 | Selection | Rotate click another part or empty view preserves target; gizmo crossing parts cannot retarget; Escape/focus/capture loss rolls back. |
| ST-02 | Physical geometry | no embedded exteriors; valid purposeful apertures and engines pass; placement/reattach/symmetry/PCG share validator. |
| ST-03 | Camera and GUI | close focus, cursor zoom, correct docked viewport edges, model-following outline, readable inspector and real tool icons. |
| ST-04 | Save/runtime | all source edits, derived assets, socket/animation/material/interior metadata survive restart; cockpit-to-airlock playthrough. |

Run staged smoke tests first (read-only contract, dummy command, Studio smoke), then error-injected PCC smoke, then same-job Cortex + Forge UI trace, then native Studio/generator runtime tests and finally Windows Full Gate and interactive screenshots. **GREEN in Subspace at S15B is not evidence of Forge/Cortex service parity or 3D inference.** Exact local branch/dirty/commit/source/asset fingerprints are a precondition to any code migration.

## I. Implementation order and bounded patch deliverables

1. **P0 inventory/handoff (this artifact):** published SHA + verify local source, compare Cortex A05, ForgeGUI 0.4.13 and actual installed ForgePY/current Rust Forge with exact manifests. Preserve old source and current PCC; reconcile outdated architecture JSON only after tests. Deliver version/command/feature parity table.
2. **P1 PCC compatibility adapter:** build thin version-negotiated read-only introspection on *existing* ProjectOps CLI, normalized typed command/action/capability status and project identity. Add golden fixtures for absent/malformed/untrusted contract, read-only behavior and aliases. No GUI dependency or alternate dispatcher.
3. **P2 Forge operation integration:** persistent OperationId and writer lease, stream stdout/stderr via existing project PCC, cancel/timeouts/diagnostics and same receipt across CLI/Cortex/ForgeGUI. PCC still works standalone; executable command discovered once. Do not assume Cortex A04 inventory is parity.
4. **P3 S15C0A / S15C0B Studio integrity:** stable selection/gesture + shared collision/semantic placement; tests across Studio/in-game; saved blueprint integrity.
5. **P4 S15C1 / S16 polish:** coordinate rect camera/picking/close-up, real gizmos/outline/measurements and unified GUI/commands/tooltips/icons; independent interactive smoke.
6. **P5 Studio↔Cortex optional bridge:** live version negotiation, selection snapshots, capability permissions, plan previews, Forge background job adapter, untrusted candidate staging, user accept as one local editor transaction. First generator **parametric hatch** with deterministic native geometry, not an undocumented 3D black-box.
7. **P6 S17 / S18:** source kit import/extract and scripted Blender worker; animation of hatch, material corpus PBR/paint zones, licensed Vault; ready and validated GLB/sidecar pipeline.
8. **P7 S19 / full 3D candidate lane:** PCG exterior/interior validation, animated connected doors, FPS playtest; then optional text/image-to-mesh provider under same contract and quality envelope; cross-product Windows certification.
9. **Promotion:** compare ForgePY/current PCC to Rust Forge against behavioral fixtures, build current native Cortex GUI with exact ForgeGUI commit pins and real Windows smoke, explicit user-approved operational takeover only then. Retain direct Subspace PCC for recovery.

**Never bundle all external source trees or heavy models into a Subspace patch; do not report this handoff as implemented PCC, GUI, Cortex connection, AI mesh generation, or production certification.** A docs/contract-only patch may safely add specifications as a reviewable source update, but requires normal PCC approval, gate and commit and is not a feature implementation.

## J. Source references, dated evidence, and open checks

1. Subspace SHA: https://github.com/shifty81/Codename-Subspace/commit/cee9993c48f7b0d3c854096c763a77577760b63b
2. Current Subspace provider: https://github.com/shifty81/Codename-Subspace/blob/cee9993c48f7b0d3c854096c763a77577760b63b/project.control.json
3. Subspace CLI: https://github.com/shifty81/Codename-Subspace/blob/cee9993c48f7b0d3c854096c763a77577760b63b/tools/control/ProjectOpsCli.py
4. Subspace command registry: https://github.com/shifty81/Codename-Subspace/blob/cee9993c48f7b0d3c854096c763a77577760b63b/tools/control/ProjectCommandRegistry.ps1
5. Subspace patch pending guard: https://github.com/shifty81/Codename-Subspace/blob/cee9993c48f7b0d3c854096c763a77577760b63b/SubspaceTools.ps1
6. Subspace Studio/Gizmo: https://github.com/shifty81/Codename-Subspace/blob/cee9993c48f7b0d3c854096c763a77577760b63b/engine/src/studio/StudioApplication.cpp
7. Studio controller migration: https://github.com/shifty81/Codename-Subspace/blob/cee9993c48f7b0d3c854096c763a77577760b63b/engine/include/ship_editor/ShipyardProfessionalController.h
8. Subspace existing authority rule: https://github.com/shifty81/Codename-Subspace/blob/cee9993c48f7b0d3c854096c763a77577760b63b/docs/ARCHITECTURE_AUTHORITY.md
9. Cortex A05: https://github.com/shifty81/Cortex/commit/5e5e113d6168acac721bb722d169be04b4ec1c3f
10. ForgeGUI provider 0.4.13: https://github.com/shifty81/ForgeGUI_Core/blob/eafa8e78efd54142a19e66d8be7b7d3985af23d2/docs/INTERNAL_PCC_STANDARD.md
11. Forge/F797 Git: https://github.com/shifty81/Forge/commit/f00ca0fea2dfa29dc930fcc18ddcedc347a7c6ad
12. Cortex ComfyUI image provider: https://github.com/shifty81/Cortex/blob/5e5e113d6168acac721bb722d169be04b4ec1c3f/crates/cortex_provider_comfyui/src/lib.rs
13. Blender modifiers and Geometry Nodes: https://docs.blender.org/manual/en/4.5/modeling/modifiers/introduction.html ; https://docs.blender.org/manual/en/4.5/modeling/modifiers/generate/geometry_nodes.html
14. Khronos glTF: https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html
15. Windows mouse capture: https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setcapture

Outstanding blockers before a real update: exact current *local* ForgePY/Forge/Cortex/ForgeGUI source manifests and versions, real GUI Host API/ABI choice, AI 3D provider capabilities and license, project local dirty files + last Full Gate fingerprint, and Studio native MSVC regression evidence. No donor code should be copied before these are available.

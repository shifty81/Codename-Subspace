# Null Harbor — cross-project structural audit and foundation program R3

Status: **verified source findings + limited code stabilization**. This is NOT a completed engine-wide refactor or completed rebrand. The patch's C++ changes address immediate widget visibility/render-state risks. The project remains named Codename Subspace until the identity migration is certified.

Evidence: GitHub `shifty81/Codename-Subspace` at `233f5104f93dfd9b484123e533b458d2cfdabccb`; September 17 broader source ZIP for inventory; user's post-R2 `StudioApplication.cpp` and post-shader-R1 payload. Local Windows checkout, actual GPU output, later uncommitted changes and full PCC gate are NOT visible to this package. Stop patch intake on preimage disagreement. The September 17 inventory cannot certify all September 20 paths.

## 1. Why widgets regress even when a quality gate says green

- `StudioAxisGizmo::Build` previously skipped camera-aligned X/Y/Z axes if the projected line was shorter than 1.8 pixels. Entire gizmos can vanish in otherwise valid camera orientations. R3 provides deterministic screen-space fallback directions without changing physical transform-axis identities. This is a usability workaround, not proof of pixel-perfect manipulation under every camera.
- The early gizmo eligibility return applied a Sockets inspector-tab exclusion to *Model* as well as Assembly. R3 restricts that exclusion to Assembly. The Model workspace must not inherit a stale Assembly-only inspector state.
- `StudioApplication::VisibleGizmoSnapshot` invalidates a handle if any of six shaft sample points intersect a floating panel, even if the tip remains visible; this can legitimately hide the whole widget when it lies beneath a panel. Input and draw should eventually use the same immutable clipping/occlusion geometry and expose a reason when a handle is unavailable.
- The renderer binds GLSL programs; the Studio overlay uses fixed-function immediate geometry and `glPushAttrib` but did not explicitly unbind a current program. R3 adds a guarded scoped GL program suspension around the Studio overlay, restores the previous program, and leaves the renderer's existing material implementation intact. This is a hardening fix; a live GPU comparison is still required to prove the reported regression's cause.
- Existing checks focus on `StudioGizmoMath` and static source tokens; they do not prove visibility, mouse pick, actual object movement, save/reopen or graphics-state isolation on a real WGL context. Add offscreen GL checks and Windows recorded-frame/input fixtures before calling UI interaction certified.

## 2. Source inventory and responsibility conflicts

| Existing authority | Evidence / conflict | Correct contract and future work |
|---|---|---|
| `ShipyardBuilderSystem` | Large runtime model aggregates authoring, selection, docs, appearance, PCG, interior and UI; `ActivateInternal` branches by command. | Make `ShipyardDocument` authoritative and versioned; `ShipyardSession` owns transient selection and preview; command handler produces a document transaction and diagnostics. Keep builder as compatibility facade until migrated. |
| `ShipyardDocumentSystem`, `ShipyardStableIdSystem`, `ShipyardSelectionSystem`, `ShipyardCommandSystem`, `ShipyardHistorySystem` | Already named by `professional_shipyard_normalization_v1.json`, alongside an explicit TODO to migrate builder state and full-runtime-model undo. | Finish this migration, do not introduce another document/selection/undo implementation. Stable IDs must never equal vector indices in published references. |
| `EditorDockSystem`, `SubspaceDockSystem`, `EditorDccShellLayoutSystem`, `ShipyardPanelCompositorSystem`, `ShipyardDockPointerSystem` | Shell/layout, panel materialization, overlays and input each recompute some geometry. `BuildControls` drops controls outside short panels. | One per-frame immutable `EditorLayoutSnapshot` containing rect, clip, scroll offset, z, visibility, DPI, focus/input capture. Every subsystem consumes it. Dock model owns layout state; compositor only projects it. |
| `StudioApplication`, `NativeGameApplication`, `NativeBattlefieldRenderer` | Separate host apps call the same large renderer; standalone Studio does not supply `editorInteriorShell` while game editor does. Studio draws a separate gizmo after renderer. | Shared authoring service plus separate Studio/game adapters; draw and pick from same snapshot. Studio must publish the shared interior shell without creating a parallel generator. |
| `ShipInteriorLayoutSystem`, `ShipInteriorDerivedShellSystem`, `ShipInteriorStructureAuthoringSystem` | Derived shell exists, but first geometry implementation explicitly supports axis-aligned envelopes; independent authored structure is not a single persisted runtime scene yet. | Shared compiled interior scene with stable IDs and explicit `PreviewOnly` vs collision/nav/runtime-ready certification. Ghost hull is presentation policy, never paint alpha mutation. |
| `ShipBlueprintLibrarySystem` + `StudioDocumentStore` + model codec | `.subspace_ship` stores recipe/equipment/appearance/tags. `.subspace_studio` stores the model separately. A ship save is not a complete Studio-project save. | Versioned manifest references assembly, models, interior, fittings, materials, generation lineage by stable ID and hash; transactional save/load/recovery; migrate without destroying legacy files. |
| `GenerateVariant` + PCG studio | Approved or safe-draft candidate is directly assigned to `model_.recipe`. | Generate detached candidates; inspect/compare/discard has no document mutations. Accept is a single undoable change with seed/profile/catalog provenance. Separate damaged ship instance from blueprint design. |
| `SpaceMaterialSystem.cpp` + `NativeBattlefieldRenderer.cpp` | Inline GLSL 1.20 source, profile params, shader compilation, GL state, fixed-function draw, space environment and overlay coordination span large files. | Split shader source/library, program lifecycle, material binding, render-state guard, render passes and content import through existing renderer facade, not a competing graphics engine. |
| Internal project PCC / Forge-compatible patch contract | `project.control.json` identifies `codename-subspace` in multiple fields and remote URLs; patch receipts and `.subspace` state reference old lineage. | Alias-aware identity migration and validation first; user-facing Null Harbor name next; repository URL change only *after* verified migration and explicit user action. |

## 3. Proposed target package boundaries (migrate, don't clone)

- `core`: stable object IDs, deterministic serialization, result/errors, version migration, resource provenance.
- `simulation`: ships/fleets/stations/interiors, damage/engineering/economy, independent of Studio widgets and GL.
- `authoring`: document/session/selection/commands/history, property schemas, validation, catalog, PCG candidate service, runtime publication.
- `render`: render device/state scope, materials/shaders, geometry/shell, transparent passes, HUD overlay; Studio and game use same renderer with different views.
- `ui`: one dock+layout+focus+input+style/typography owner; pixel geometry and rendered/pick rectangles identical.
- `hosts`: `Null Harbor` game host, `Null Harbor Studio` host, internal PCC launcher; adapters only, no duplicate gameplay/authoring authorities.
- `compat`: legacy `subspace` namespace, schema IDs, file extensions, path aliases, build target names and patch lineage. Retire individually after tests prove migration.

These are conceptual module boundaries, **not directories to move in one unverified patch**. The existing project owns the implementations; extract interface seams and migrate dependents in small GREEN checkpoints.

## 4. Shader modernization after restoring the widget

Immediate: isolate GPU state for world, translucent hull, gizmo and UI; capture compile/link failure logs and fixed-function fallback; use explicit program/material states. Keep Shader R1 source until verified. Normalize `SpaceMaterialProfile` into one material-description/override model, including armor, glass, thrusters, paint, decals and station materials. Avoid per-pixel expensive procedural fBm on ordinary hulls; profile and avoid specular aliasing with correct normal/roughness frequency handling.

Next, optional quality tier: linear-light/sRGB color workflow, energy-aware metallic/roughness response (glTF 2.0 as an interchange/reference, not automatic glTF conformance), normal/AO/metallic-roughness/emission textures, sun key+ambient fill, restrained screen-space tone mapping, emissive bloom as separable postprocess, profile-exact shield water ripples, camera-relative interior ghost alpha with depth sorting and selected-object outlines. Preserve existing paint zones/semantic materials. Defer shadows/Forward+/GPU instancing/HDR rewrite until baseline captures and timing budgets are available. No claim of PBR, HDR or bloom implementation in R3.

Reference: https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html . Current planned renderer contract: `content/architecture/renderer_modernization_contract_v1.json` (as of September 17 snapshot).

## 5. GUI consolidation: the professional workflow

Default permanent workspaces: Assembly, Model, Interior, Systems, Appearance, Test. Put PCG candidate browsing in a clear workflow entry accessible from Assembly and Advanced tools, without a second permanent toolbar. Keep viewport dominant; dock Outliner/Inspector/Assets/Activity/Validation as panels, with floating/popover controls. Implement panel minimum sizes, scrollbars, keyboard navigation, tooltips, command search, collapsed categories, proper disabled reasons, undo transaction status, save indicators and error telemetry.

Critical deletion policy: keep compatibility adapters until new layout+input+draw parity tests pass; then remove old parallel positioning. Do not merely hide controls when they don't fit. No placeholder control may advertise functionality without a real command and persisted result.

## 6. Current rebrand reality and migration plan

The September 17 source contains the contradictory active document `docs/NULLHARBOR_INTEGRATION_AUTHORITY.md` describing Codename Subspace as the current name, plus a project manifest with two `codename-subspace` IDs, old GitHub URLs, root `SubspaceTools.*` launchers and `.subspace` state. There are hundreds of textual/name identifiers. Do NOT global search-and-replace: this would break PCC patch matching, CMake target references, serialized saves and paths, and lose provenance. R3 includes a read-only identity scanner to build the exact current-checkout inventory.

Identity migration sequence:

1. Decide canonical display name **Null Harbor**, target new app labels **Null Harbor** and **Null Harbor Studio**, prospective repo slug `Null-Harbor`. Preserve `Codename Subspace` as historical lineage in migration docs; name no parallel game.
2. Audit all active name/logo/title/installer/service/launcher references. Change *display name* and source documentation with compatibility aliases first. Preserve existing save formats and content IDs until converters + round-trip fixtures work.
3. Change PCC project identity with dual-ID intake acceptance for approved old receipts and new IDs. Do not simply switch manifest id without patch-provider migration. Preserve `.subspace` path as a legacy state alias during staged migration.
4. Add compatibility tests opening prior `.subspace_ship`, `.subspace_studio`, patch receipt, world/save and content registries; test new writes and recovery. No destructive rewrite or rename-in-place of user documents.
5. Rename binary/window/product titles and packaging, with old CLI/build-target wrappers temporarily supported. Update README, credits, reference docs and active authority records; historical audit logs remain immutable, marked superseded.
6. Run clean Windows CMake configure, Full Gate, real Studio widget/GUI/interior/generation fixtures, game load/save and PCC patch roundtrip, generated installer/run smoke, link/remote scan; only then mark `GITHUB_RENAME_READY`.
7. User renames GitHub repository. Afterwards update `origin` URL, manifests, badges, workflow/action references, documentation, mirrors and external links; audit again. GitHub usually redirects old repo URLs but workflow actions hosted at renamed repo do not redirect. Do not rename GitHub remotely during this patch.

## 7. Verified and pending acceptance matrix

| Gate | Evidence required | Status at R3 packaging |
|---|---|---|
| Exact source | SHA-256 checks for R2 Studio, GitHub gizmo, shader R1 | Patch preconditions included; actual local PCC still pending |
| Camera-aligned gizmo | Pure C++ test of fallback directions | Compiled/executed locally; Windows viewport unverified |
| Shader/overlay state | Source guard and Win32 GPU test of program save/unbind/restore | Source fix supplied; Windows GPU test pending |
| GUI | 1120x740 to 2560x1440, 100–200% DPI, floating overlays, tooltips, mouse capture | Not implemented/certified in R3 |
| Studio document | Complete ship+model+interior+fitting save/reopen/recovery | Not implemented/certified in R3 |
| Runtime parity | Published same ship renders/works in game | Not implemented/certified in R3 |
| Shader visual improvement | shader compile log, before/after fixture, GPU frame timing | R1 source preserved; comparison pending |
| Rebrand | scanner reports no active unreviewed brand references and all migration fixtures pass | Not ready; scanner supplied |
| Full quality gate | Windows PCC certificate and hands-on test | Not run here |

## 8. Handoff to subsequent implementation

R3 contains two precise widget fixes and GPU program-state isolation, not a pretend whole-editor rewrite. Once R3 runs on the user's exact checkout, the next **implementation** pass should normalize UI layout/input snapshots and build the standalone Studio interior-shell bridge, with actual post-patch source and diagnostic receipt. Then migrate documents, candidate generation, in-game publishing, shader material pipeline and staged Null Harbor identity. Preserve the internal PCC and Forge patch contract at every checkpoint. Retain previous green source and recovery files.

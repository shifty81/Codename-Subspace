# PASS1508R4 — visual geometry root cause and Shipyard workflow audit

**Audit basis:** exact PASS1508R3 patch payload over the verified PASS1508R2 stage derived from the 2026-09-16 source rollup, with committed Git parent `a3221000ba127bc1969aa8f1b220ea419dd1d7c1`. No claim of auditing later, unpublished Windows edits. No PCC/patch-engine/asset modifications.

## Why PASS1508R3 did not finish the visual repair

There were **two independent layout paths**:

1. `ShipyardBuilderSystem::Layout(model,w,h)` drove panel bounds, controls, and the fixed rail through `SubspaceDockSystem::Materialize`. PASS1508R3 corrected *this* tree.
2. `DrawStandaloneShipyardBackdrop` still invoked `ShipyardBuilderSystem::Layout(w,h)` with **no model**, which reads `EditorDccShellLayoutSystem::Compute`. That legacy shell ended the rail at `shelfY`, started Assets at `x=0`, and sized the canvas/grid using Assets' Y. The renderer's floor, background, and fixed panel geometry could therefore disagree with the live dock UI, especially after Assets floated.

Additionally, the Assets header was drawn **before** the floating-panel backing was repainted to erase the ship's screen-space frame. Floating Assets could lose its header to that repaint.

**R4 correction:** use live-model viewport bounds in the backdrop; make model-less fallback rail/shelf layout consistent with separated columns; paint the Assets header after floating-panel backing. Add an isolated native DCC geometry regression and a bounded backdrop static gate. No separate dock store. Do not mark visual acceptance until the user's Windows runtime inspection.

## Source/GUI gap audit (observations, not completion claims)

| Area | Evidence found | Remaining work and acceptance requirement |
|---|---|---|
| Background vs dock authority | NativeBattlefieldRenderer backdrop was model-less while overlay used model-aware layout. | **R4 addresses**; move Assets without changing the floor-grid, rail or viewport geometry. |
| Dock model | `SubspaceDockSystem` provides tab state, float, move, resize and serialized text; Shipyard pointer invokes these. | Verify drag preview, targeted split/tabs, min sizes, focus/z-order, outside-window capture, and persistence across actual restarts. Tests of dock structures do **not** prove rendered mouse UX. |
| Empty bottom slot | The `content` splitter reserves a bottom branch after Assets is floated; R3 intentionally retained it for drops. | Reclaim empty area or draw an explicit compact drop strip, using **the same bounds** for `Materialize`, hit testing, and perspective camera; don't secretly change split ratios in rendering only. |
| Viewport camera and picking | `SetupPerspectiveProjection` and `StrategicViewProjection` use full window width/height, while the live 3D View is a subrect; module picking also takes full window dimensions. | Bind projection, scissor and picking to the same actual viewport and test selected-module center/pick rays, resizing, floating and maximize. Do not change projection alone. |
| Main menus | The 3D UI draws literal `File Edit View Help` text. No native menu creation found in scanned engine source. | Implement actual menu models, clickable bounds, keyboard navigation and actions or label decorative text honestly. |
| Command palette | F3 toggles and paints a list of command descriptions. Source search found no typed filter/selection dispatch for that display. | Search input, result focus, execute or disabled reason, Escape/Enter and reliable keyboard ownership. |
| Inspector/help abstractions | `ShipyardWorkspaceSystem::BuildInspector/BuildHelpRegistry` are called by tests; no runtime invocation found in scanned engine source. Live inspector uses separate hand-positioned command controls and handwritten help text. | Wire schema/properties/help into the actual renderer/editor state; one property authority, real editing commit, invalid/dirty feedback, tooltip boundaries. |
| Dockable tool catalog | Default workspace registers Modeling, Interior Program, Systems, Materials, Kitbash Intake and other leaves; the scanned renderer/visible cutover contains no corresponding panel-content adapter for most of those IDs. | A registered empty dock node is not a functional editor. Integrate one substantive panel at a time with selection, commands, undo and persistence. |
| Ship construction | `ShipyardBuilderSystem` has real staging, socket/transform commit, blueprint, model primitive, paint and interior commands. | Demonstrate an entire manual asset->snap->commit->validate->save->reload->test workflow with real content, not only `Activate` unit tests. |
| Modeling/UV | Modeling primitives and modifier commands exist; prior Foundry status explicitly labels production Boolean execution and automatic UV backend pending. | No production Boolean/UV completion claim until backend, geometry integrity and visual/persistence tests pass. |
| Materials and kitbash | Appearance data and governed intake systems exist in source; source rollups may omit hydrated vendor assets. | Test textures under actual hydrated assets, provenance/license and paint-zone consistency; source-only cert is not asset-quality cert. |
| Status and gates | Green Full Gate runs build/CTest/static stages; previous green passed with visible broken floating GUI. | Add actual interaction capture/screenshots and viewport region probes to gate; require human visual inspection until automated rendering is credible. |

## Next repair order after R4 is visually accepted

1. **Viewport authority:** unify 3D view rect, projection, selection/picking and grid for all dock states, with tests.
2. **Dock UX:** compact empty-zone reclaim, actual tab/split preview, cursor affordances, safe save/restore and correct Windows capture.
3. **Global chrome:** live File/Edit/View/Help menu models; command palette input and dispatch; focus/shortcut conflicts.
4. **Integrated authoring:** activate one dockable panel at a time using the existing command/document/undo/validation authorities (Assembly, Model, Interior, Systems, Paint, Test).
5. **Acceptance:** test source asset -> placement -> socket -> save/reload -> materials -> interior -> Test, with hydrated content and screenshots; report unsupported steps rather than mock UI.

## Windows visual checks (before commit)

1. Launch at maximized and a smaller supported window (minimum 1120x740).
2. Start with docked Assets; rail extends to the status bar, and Assets starts to the **right** of the rail.
3. Drag Assets to the center. The rail/background and grid boundaries stay stationary; the floating ASSETS header stays visible.
4. Move Assets above and below its initial Y, move Outliner and Properties, and redock all. No lines, text or pointer targets escape their panels.
5. Repeat with one or more right panels hidden; screen remains in one coherent coordinate space.

**Build status:** compile/CTest and source gate are not equivalent to Windows GUI acceptance. Project-owned PCC remains the full certification authority.

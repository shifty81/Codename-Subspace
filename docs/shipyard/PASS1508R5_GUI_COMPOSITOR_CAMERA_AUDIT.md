# PASS1508R5 — one Shipyard compositor, continuous inspection camera

**Input authority:** the user-supplied 2026-09-16 source rollup, with the direct PASS1507, PASS1508, PASS1508R1–R4 and R4R1 payloads layered in sequence. Published Git parent at the time of preparation: `a3221000ba127bc1969aa8f1b220ea419dd1d7c1`. No ForgePY migration, new GUI runtime, broad code replacement or change to PCC authority.

## Reproduced root causes

1. `DrawShipBuilderOverlay` drew floating-panel backings earlier in the function, painted controls from *all* panels afterwards, and only then painted some contextual content. Consequently, the last painted control or text could pass visually through an earlier floating panel. Its backing used a shared alpha/blended material rather than declaring an opaque foreground layer. Position fixes from R2–R4 could not resolve that ordering defect.
2. `BuildControls` and the pointer handler had an independent z-order/occlusion path. An empty floating-panel body must block clicks to the viewport and any docked button beneath it. A dragged existing float must rise to the front in the same array that controls rendering and hit-testing.
3. Standalone and in-game Shipyard fed `panX` and `-panY` into camera translation. Camera translation moves the visible subject in the opposite direction, so the scene moved *away* from MMB drag instead of following the cursor.
4. `StrategicViewProjection::Build` and `ConstructionEditorCameraSystem::FreeBasis` switched their reference world-up vector from Z to Y when the view direction approached vertical (`abs(forward.z) > .985`), roughly 80 degrees. That discrete reference-axis change rotates the projected screen frame abruptly; it looks like a snap to a top or bottom view. Orbit already limits pitch to ±89 degrees, so a continuous Z-up basis is non-degenerate over its usable range.

## Implemented source changes

- Introduce `ShipyardPanelCompositorSystem` as a *view over the existing* `SubspaceDockWorkspace`: dock panels first, then floating panels back-to-front. It does not store a competing workspace or copy selection state.
- Draw docked controls in their own scissor; only after all underlying content and viewport overlays, draw each floating panel as a complete opaque background/header/content unit. The floating Properties inspector uses the same contextual content function as docked Properties. Do not draw floating controls twice.
- Render/hit-test/pointer capture consult the same floating order. An empty floating body masks the viewport. Beginning a real drag raises the dragged floating panel via `SubspaceDockSystem::RaiseFloatingPanel`.
- Add one `ConstructionEditorCameraSystem::PanPixels` function for both entry points; it converts pixels at the current camera distance with the perspective projection's configured 45-degree FOV. The subject now follows pointer X/Y without a magic screen-scale coefficient; the camera eye and orbit target still translate together.
- Remove the discontinuous near-pole up-axis switch from renderer projection and construction-camera movement basis. Keep current turntable pitch clamp and independent Alt/free-flight mode.
- Add a standalone native CTest covering projected cursor-relative pan, continuity through both pole regions, a stable target/distance, floating front-order/hit priority, redock, and an independent source/connection gate. No renderer screenshot is claimed by these tests.

## Source-backed remaining workflow gaps — not completed by R5

| Area | Current source observation | Acceptance for the next tranche |
|---|---|---|
| Truly unrestricted 3D orbit | Orbit is Euler yaw/pitch with ±89-degree pitch clamp. The pole snap is addressed but it is **still a horizon-locked turntable**, not unrestricted trackball rotation. | Optional quaternion/trackball mode with continuous roll/right/up basis, no pole singularities, stable save/load and independent keyboard/free-flight profile; controls explicitly documented. |
| Projection and picking | `SetupPerspectiveProjection`, `StrategicViewProjection` and `PickShipyardModule` are passed the full client width/height, whereas live viewport is a sub-rectangle of the dock layout. | One viewport rect, aspect, GL viewport/scissor, UI screen-to-view translation, ray and gizmo hit test for every dock/float/maximize state. Avoid altering only projection or only picking. |
| Window composition | OpenGL drawing remains immediate-mode, including hand-built headers, controls and overlays. | Consolidate panel rect, clipping, z-order, mouse capture, focus, scroll and invalidation into a single editor-surface service, adopted by *all* Shipyard panels. R5 normalizes the floating layer but is not an entire GUI-framework rewrite. |
| Dock UX | Workspace model supports float/tab/redock/resize; implementation is leaf-drop targeting and custom title-hit logic. | Visible tab-strip/dock-target previews, edge split insertion, full keyboard and mouse capture, unclamped minimum sizes, panel focus and layout persistence proven across restarts. |
| Menus and command search | `DccToggleCommandPalette` toggles the display and renderer paints predefined rows; no typed-filter/selection execution found in inspected runtime. | Real editable search field with focus, live filtering, Enter/Escape dispatch and disabled explanations; live File/Edit/View/Help menus with command registry authority. |
| Inspector and help | `ShipyardWorkspaceSystem::BuildInspector/BuildHelpRegistry` exist but inspected production rendering uses separate manual controls/help strings. | Adopt schema-bound real properties, commit/undo/validation and a single help/tooltip registry. |
| Registered panels | Default workspace registers Modeling, Interior, Systems, Materials and Kitbash panels; not all have functional rendering adapters. | For each panel, visible content, selection coupling, real commands, undo, save/reload, validation and an interaction test, not only a registry entry. |
| Full gameplay workflow | Commands for staging, sockets, recipe, paint and interior exist. | Manual module browse -> drag -> socket snap -> commit -> save blueprint -> reload -> fitting/interior/paint -> playtest with a hydrated asset corpus and real screenshots. |
| Asset provenance and materials | Source-only rollups cannot prove textures and vendor content are hydrated. | Verify actual licensed kitbash packs and texture slots under source authority; do not certify absent content as PASS. |

## Reference conventions

- Blender's Navigation manual treats pan as translation of the viewport without camera orientation changes; its default MMB orbits and Shift+MMB pans. Subspace intentionally retains the user's desired RMB orbit/MMB pan keymap instead of silently copying Blender's keys.
- Blender's Navigation preferences distinguish horizon-locked **Turntable** from unrestricted **Trackball**, and describe orbit sensitivity and orbit-around-selection. Reaching true free rotation will require a separate trackball/quaternion implementation, not merely removing a pitch clamp. Relevant manuals: https://docs.blender.org/manual/en/5.0/editors/preferences/navigation.html and https://docs.blender.org/manual/en/5.0/editors/3dview/navigate/navigation.html .

## Acceptance before release

1. PCC full gate succeeds; its Windows result is authoritative. The linked headless tests alone do **not** establish Windows GREEN.
2. Float Assets and Properties with 50% overlap. Move each across the other; the dragged panel must become the topmost **opaque** panel, with its own text visible and lower text fully occluded. Clicking its empty body must not select modules underneath.
3. Float Outliner over Assets; repeat dragging and redocking. No orphaned status/viewport lines, stale hover cards or clipping outside the panel.
4. RMB orbit gradually through pitch 75–89 and -75–-89. There must be no sudden quarter-turn/pole snap. Current turntable will *stop* at the pitch limit; this is known, not true free-trackball completion.
5. MMB drag left/right/up/down. The visible subject should follow pointer motion in both standalone and in-game Shipyard; distance and orbit orientation must remain unchanged until orbit/zoom is requested.
6. After the above, test ship-part picking near all viewport edges with docked and floating panels. Any offset is a separate viewport-projection authority defect and must not be misreported as fixed.

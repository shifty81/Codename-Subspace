# Studio S15A — cumulative transform feedback and authoring prerequisites

Base: GitHub `main` commit `139279ca32b78cfc71f990823fc72042ff2c4883`.
Delivery: single root-drop `forge.patch.v1` ZIP, **cumulative S13 + S14 + S14R1 + S14R2 + S15A**, not a complete source rollup.

## Actually implemented this pass

- Visible viewport transform HUD in Build workspace, including Select mode, sourced **directly from the existing** selected `VisualModulePlacement` and canonical module catalog (no independent transform authority).
- `P`: assembly-local XYZ position, `R`: physical X pitch / Y roll / Z yaw in degrees, `S`: per-axis scale percentage; `D`: *nominal local W/L/H* dimensions derived from certified catalog half-extents, scale, recipe width/length multipliers (world unit = meter).
- `D` is intentionally unavailable if source dimensions are missing, zero, NaN/Infinity, or overflow. It does NOT claim mesh-exact rotated world dimensions or account for modifier/Boolean geometry. Source dimensions are local and approximate.
- Positive dimensions under mirror/negative scale. Values are display-only and cannot change authoritative coordinates.
- Larger shaft/handle hit targets improve discoverability without allowing the pivot itself to select an ambiguous axis.
- HUD is clipped to the actual viewport; suppressed for narrow viewports and when an existing floating panel covers sampled HUD area; does not claim a full dock/panel overlay rewrite.
- S13 guarded WM_CLOSE, S14 axis handles/angle arc, S14R1 Win32 `near` macro repair, S14R2 physical Y/Z rotation-field correction are all retained in one complete overlay.
- New CTest `SubspaceStudioTransformReadoutTests` checks position, pitch/roll/yaw order, percentages, source dimensions, missing/nonfinite data, negative scale, stored angles.

## Unimplemented — intentionally not represented as green

1. Full Windows Studio compile, actual OpenGL HUD pixel/render test, game smoke, and PCC Full Quality Gate must run on user's Windows machine.
2. The renderer still draws an oversized bounding-box selection; **not replaced** with exact mesh silhouette yet. Requires renderer-side depth/silhouette ID pass and matching pixel-picking certification.
3. RMB orbit/context-menu state machine; Studio still uses MMB orbit. Needs changes to shared Win32 NativeWindow with game-default behavior preserved.
4. Catalog free-placement, true 3D ray-plane/surface positioning, sockets-at-contact, mesh extraction, complete editable model/interior persistence, interior kit hydration, humanoid preview, generator UI and publication are outstanding.
5. This HUD does not edit numerical inputs yet; true editable Inspector follows shared transform transaction and document persistence.

## Dependency-based next steps

- S15B: single pointer owner for RMB orbit vs short RMB, GUI focus and world/surface ray placement; free cursor placement by default in Studio and optional deliberate magnet snapping.
- S15C: renderer-bound mesh silhouette selection (not a broad AABB); mouse-picking/selection-ID tests and inspector numeric editing.
- S16: versioned transactional authoring document that actually saves and reopens model recipes, interior drafts, sockets and asset links **before** creating more authoring data.
- S17 onward: topology-stable mesh selection/cut/extract, component/prefab publishing, selective generation, approved attachments, pressure hull, interior, runtime certification.

## PCC installation contract

- A clean main at `139279c` may approve **only this cumulative patch**. Do not also apply S13, S14, S14R1, S14R2.
- If any prior patches have already been applied to the working tree or HEAD advanced: DO NOT force, overlay manually, reset, or delete changes. Supply a current PCC status/debug bundle to rebase the exact overlay.
- No automatic Git commit/push. Run project-owned Full Gate and manually exercise X/Y/Z rotations, numeric readout, floating docks, Select mode, close cancellation, game smoke; commit only when GREEN.

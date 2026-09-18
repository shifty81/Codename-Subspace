# G4 camera and G5 seat foundation — implementation/acceptance handoff

Baseline: certified Git `5040815de49cd291b982b1b0a1410413631cf286`. This package intentionally does **not** overwrite the newer G3 app, renderer, native window, CMake, PCC or compositor work.

## Implemented in this package

- Studio `ConstructionEditorCameraSystem` gains orientation-preserving Frame Selection and editor-only front/right/top/back/left/bottom camera orientations. Top/bottom stay at ±89° in the existing perspective renderer to avoid a degenerate pole. Existing default orbit and gameplay camera behavior remain intact.
- A fail-closed `FleetCommandSeatSystem` validates the authoritative occupancy, permission, power, installed command equipment and link for a physical station, bridge or pilot seat. Session ownership is immutable until the owning actor exits; per-order checks revalidate live seat state and support comms revocation.
- `RuntimeControlContextSystem::BuildWithCommandSeat` exposes an opt-in, seat-authorized fleet view with flight, weapons and mouselook input disabled while fleet UI has input. Legacy `Build` remains unchanged for existing callers until the app transitions safely.
- Registered G4/G5 regression assertions are inserted into the existing camera/compositor and authoring/runtime test executables, so no stale G3 CMake file is overwritten.

## Not implemented / do not mark G4 or G5 fully complete

1. Numpad 1/3/7, 5 orthographic, 0 scene camera, Home/period framing, Shift+grave Fly/Walk, clickable axis gizmo and user-facing adjustable navigation settings still need NativeWindow/input routing and rendering/GUI ownership. The new camera methods are compiled and tested but **not yet bound to shortcuts**.
2. Full Save/Open/Save As, dirty structural/interior document round-trip, native OS file picker, true X-ray/boolean cutaways and GUI scroll/occlusion visual signoff remain open from G3.
3. `NativeGameApplication` still uses the transitional legacy control-context `Build()` and strategic flight toggle; it does **not** yet perform a real seat reservation, command order authorization or guaranteed return to the original avatar seat. Integrate `BuildWithCommandSeat` only after seat/world identity and order-bus ownership are connected.
4. Seamless planet streaming, ground collision, atmospheric landing/takeoff, persistent interiors and multiplayer seat arbitration are not delivered here.

## Required next integration pass

Replace ad-hoc strategic view toggles with a real physical seat interaction and server-authoritative session; bind the new seat-aware Build and check `CanCommand` at every fleet order dispatch, show comm-link failures and restore on-foot/cockpit view on exit. Separately finish editor shortcut mappings/projection/UI and round-trip document persistence. Run Windows PCC Full Gate, then verify with mouse/keyboard, floating panels, combat input suppression and save/reload. A Linux unit compile is not a Windows Full Gate.

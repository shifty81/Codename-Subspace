# NullHarbor R191 — Primary FPS control foundation

Date: 2026-10-02
Status: implemented source pass; Windows runtime acceptance required

This pass begins the implementation of the R190 control/GUI contract rather
than extending the R189 strategy-first bootstrap.

## Implemented

- authoritative `GameplayControlMode`: `OnFoot`, `Pilot`, `FleetCommand`;
- default runtime boot is `OnFoot`, physically inside the player ship;
- NativeWindow gameplay input profiles prevent WASD/Space/F/etc. from leaking
  across OnFoot/Pilot/Fleet contexts;
- Win32 raw relative mouse input (`WM_INPUT`) with captured/hidden cursor policy
  for OnFoot and Pilot;
- absolute visible cursor policy for Fleet Command;
- OnFoot mouse-look drives body/camera yaw and pitch;
- Alt diverts OnFoot mouse input to bounded independent head-look and smoothly
  recenters it after release;
- Shift sprint and Ctrl crouch now route through character semantic actions;
- Space is character jump intent in OnFoot rather than global `FirePrimary`;
- F is the physical helm interaction at the existing command-seat location;
- Pilot gets semantic W/S thrust, A/D lateral thrust, Space/Ctrl vertical
  thrust, Q/E roll, Shift boost, X brake, relative mouse steer, and Alt cockpit
  freelook;
- the renderer now receives a first-person cockpit pose while Pilot is active;
- Fleet Command preserves absolute-pointer camera operation and can only be
  entered from the physical command/helm interaction location in this pass;
- Fleet camera is updated once per frame (R189 double-tick removed);
- docking switches physical control authority cleanly instead of returning to
  detached Fleet Command.

## Deliberately not claimed complete

- physical jump/vault/mantle motion: the semantic Space action is now correct,
  but the current certified interior traversal system intentionally rejects Z
  locomotion and has no authored vault/mantle solver yet;
- Fleet LMB selection, drag-box selection, short-RMB contextual order, held-RMB
  radial command wheel, focus history, and cinematic director behavior;
- production OnFoot/Pilot/Fleet modular HUD rebuild from the R190 GUI reference;
- authored seat animation/sockets beyond the existing command-seat location;
- Windows visual/play acceptance for raw mouse capture.

These remain the next cumulative control/UI passes rather than being hidden by
fallback behavior.

## Validation performed

- GNU/CMake configuration succeeds;
- `subspace_engine`, native host sources, and `subspace_game` compile/link;
- expanded `subspace_r189_strategy_fps_cutover_tests` passes 15/15 assertions;
- Linux cannot execute the Win32 native-window backend, so Windows Full Gate and
  manual FPS mouse-look acceptance remain required.

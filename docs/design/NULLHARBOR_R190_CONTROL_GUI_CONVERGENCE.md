# NullHarbor R190 — Control + GUI convergence direction

Status: DESIGN CONTRACT / implementation target
Date: 2026-10-02

## Core runtime rule

The embodied player is the root of gameplay.

Primary gameplay control authorities:
1. `OnFoot`
2. `Pilot`
3. `FleetCommand`

Camera mode, HUD profile, cursor policy, overlays, and locomotion policy are
related to control authority but must not be collapsed into one enum.

Opening an overlay does not change gameplay authority.

## Boot/default gameplay

The game boots into `OnFoot` FPS at the player's physical location. For the
starter experience this is normally inside the current/starter ship, standing
near the command/helm area rather than already piloting.

Global boot into detached fleet strategy is retired as the target direction.

## OnFoot profile

- mouse: captured relative FPS look;
- body yaw follows normal mouse look;
- Alt: temporary independent head-look/free-look;
- W/A/S/D: character movement;
- Shift: sprint;
- Ctrl: crouch;
- Space tap: jump;
- Space held near valid traversal geometry: vault/mantle/climb intent;
- F: contextual world interaction;
- LMB/RMB: equipped item primary/secondary actions;
- Esc/UI overlays release pointer capture temporarily.

FPS movement/action input must not leak into ship flight.

## Pilot profile

Entered through a physical helm/pilot interaction.

- mouse: ship steer/aim by default;
- Alt + mouse: cockpit head-look without steering;
- W/S: forward/reverse thrust;
- A/D: lateral thrust;
- Space/Ctrl: vertical thrust;
- Q/E: roll;
- Shift: boost when supported;
- braking/dampening are ship-flight controls, not FPS controls;
- pilot camera and ship HUD are independent from FPS HUD.

Leaving the helm returns to `OnFoot` at the physical seat.

## FleetCommand profile

Entered through command-capable equipment/seat/console.

Fleet mode is mouse-first and is not another WASD flight mode.

Pointer:
- absolute visible cursor;
- LMB: select;
- Shift+LMB: additive/toggle selection;
- LMB drag: box selection;
- double-click: camera focus;
- wheel: zoom;
- MMB drag: orbit;
- short RMB: obvious contextual/default order;
- held RMB: contextual radial command wheel;
- WASD may optionally pan/offset the camera but is secondary.

Fleet control must never route movement keys into direct ship thrust.

## Fleet camera

The Fleet camera is subject-centric and cinematic-capable.

Selection, command target, and camera focus are separate states.

Supported target behaviors should converge toward:
- frame fleet;
- frame selection;
- follow ship/wing/fleet;
- engagement framing;
- previous focus;
- command presentation;
- cinematic presentation.

The camera follows the chosen subject while preserving user orbit/zoom where
possible. Director assistance may improve composition, but must yield to manual
input and never steal focus because something "interesting" happened.

Cinematic presentation is a camera treatment inside FleetCommand, not another
gameplay authority.

## Fleet orders

Ships execute durable directives; FleetCommand does not directly drive their
physics.

Fast order:
- short RMB issues the obvious contextual order.

Precise order:
- hold RMB to open a radial;
- radial target is locked at press/activation so moving toward a wedge cannot
  accidentally retarget;
- release over a wedge commits;
- release in dead-zone cancels.

Radial contents depend on:
`selection capabilities + target type + current context`.

Examples:
- empty space: Move / Hold / Formation / Patrol / Align;
- hostile: Attack / Orbit / Keep Range / Disable / Focus Fire;
- friendly: Follow / Escort / Form Up / Support;
- asteroid + miners: Mine / Orbit / Maintain Range / Guard;
- wreck + salvagers: Salvage / Tractor / Loot / Guard.

Immediate orders and persistent doctrine must remain separate concepts.

## HUD separation

Use a shared native GUI primitive/component library but separate HUD profiles.

### OnFoot HUD
Personal/contextual:
- interaction reticle/prompt;
- health/injury;
- suit/environment;
- equipped item;
- immediate objectives/warnings.

### Pilot HUD
Ship/flight:
- velocity/vector;
- thrust/dampening;
- shields/armor/hull;
- power/fuel;
- weapons/modules;
- target/range/closure;
- docking/navigation.

### Fleet HUD
Strategic/spatial:
- fleet/wing hierarchy;
- current selection;
- current directives/order queue;
- focus/target context;
- compact alerts;
- contextual radial;
- tactical overlays only when useful.

Character or cockpit telemetry must not remain permanently visible in Fleet mode.

## Modular GUI reference

Visual reference:
`reference/project/gui/nullharbor_modular_gui_reference_20261002.png`

Use it as a reference for modular native GUI elements:
- panel/card frames;
- buttons and state variants;
- tabs;
- toggles/checks;
- sliders;
- status/progress bars;
- alerts/notifications;
- fleet hierarchy rows;
- target/status cards;
- order queue chips/cards;
- radial commands;
- tooltips;
- compact camera controls;
- objective/status trackers.

Do not use the flattened image as a runtime asset.

## GUI component strategy

Build one reusable project-owned component vocabulary rather than mode-specific
one-off drawing.

Recommended primitive families:
- `NhPanel`
- `NhCard`
- `NhButton`
- `NhIconButton`
- `NhToggle`
- `NhSlider`
- `NhTabStrip`
- `NhStatusBar`
- `NhBadge`
- `NhAlert`
- `NhTooltip`
- `NhListRow`
- `NhTreeRow`
- `NhDataField`
- `NhProgress`
- `NhRadialMenu`
- `NhOrderChip`
- `NhTargetCard`
- `NhShipCard`

Each primitive should have standardized:
- normal / hover / pressed / focused / disabled states;
- spacing/padding;
- typography hierarchy;
- corner/border treatment;
- semantic colors;
- hit-testing;
- input ownership;
- scaling behavior.

OnFoot, Pilot, Fleet, menus, and Studio can compose from the same primitives
without sharing gameplay authority.

## Input-transition safety

A mode transition must:
1. stop/commit old-mode transient input;
2. clear one-frame edge events;
3. switch gameplay authority;
4. switch input profile;
5. switch cursor policy;
6. switch camera controller/profile;
7. switch HUD profile;
8. only then accept new-mode input.

This specifically prevents:
- Space jump becoming pilot thrust;
- FPS RMB becoming fleet order;
- radial release becoming weapon fire;
- Fleet WASD becoming ship thrust.

## Implementation sequencing

1. authoritative `GameplayControlMode`;
2. per-mode input profiles + mouse/cursor policy;
3. true relative mouse path for FPS/Pilot;
4. OnFoot boot + FPS camera/locomotion;
5. physical helm transition + Pilot profile;
6. physical command transition + Fleet profile;
7. Fleet focus/follow camera;
8. selection + short RMB command + held-RMB radial;
9. shared modular GUI primitives;
10. separate OnFoot/Pilot/Fleet HUD composition;
11. directive/autopilot/crew handoff integration;
12. runtime/manual acceptance and PCC gate.

This document is the target direction. Existing older camera/control paths are
not considered authoritative when they conflict with this contract.

# NullHarbor R192 — Starter Interior Playable Slice

R192 turns the R191 FPS control foundation into a meaningful first gameplay test.
The certified carved pressure shell remains collision/hull authority, but the
starter cabin is no longer presented as an empty box.

## Implemented

- Corrects horizontal FPS mouse-look direction: moving the physical mouse right
  turns the FPS camera/body right. Cockpit Alt freelook uses the same corrected
  yaw convention; direct Pilot steering keeps right-positive flight input.
- Adds a centralized `MouseLookProfileSystem` so FPS, cockpit freelook, and
  pilot steering do not independently reinvent mouse signs/sensitivity.
- Builds a deterministic `StarterInteriorScene` inside the largest certified
  walkable carved volume.
- Furnishes that cabin with physical, visible Helm, Fleet Command, Cargo,
  Engineering, and Airlock fixtures plus floor runner, overhead lighting, and a
  forward cockpit display bank.
- Furnishings participate in simple local collision instead of being completely
  ghost geometry.
- FPS spawns in the cabin facing the visible helm when the certified shell can
  occupy that spawn point.
- Center-look focus chooses nearby fixtures and publishes a contextual HUD
  prompt (`F  HELM / TAKE CONTROLS`, etc.).
- `F` on the Helm transfers to Pilot; leaving Pilot returns to the authored
  command-seat position.
- `F` on Fleet Command enters FleetCommand; `F` exits FleetCommand back to FPS.
- Cargo, Engineering, and Airlock interactions execute through the existing
  `InteriorInteractionSystem` and report visible status feedback.
- Global Tab no longer changes OnFoot/Fleet gameplay authority. Physical
  fixtures own those transitions.
- OnFoot now has its own low-clutter FPS HUD rather than inheriting the flight
  command rail and cockpit telemetry.

## Deliberately not claimed complete

- Starter fixtures are native project-owned procedural furnishing primitives,
  not final authored art meshes yet.
- Cargo/Engineering currently prove interaction dispatch/status, not their
  complete inventory/engineering application surfaces.
- Airlock interaction proves state and visual feedback; EVA traversal is later.
- Jump/vault semantic input exists from R191, but full vertical traversal and
  mantle/vault geometry are not certified in R192.
- FleetCommand still needs the planned mouse selection, short-RMB default order,
  held-RMB radial menu, and cinematic subject/follow director passes.

## Acceptance

1. Boot into OnFoot FPS inside the starter cabin.
2. Moving mouse right turns view right.
3. Visible helm is ahead; reticle focus shows `F ... TAKE CONTROLS`.
4. Press F at helm -> Pilot; F -> return to FPS at helm.
5. Walk to Fleet Command fixture; F -> FleetCommand; F -> FPS.
6. Cargo/Engineering/Airlock show contextual prompts and visible status after F.
7. Furnishings block basic walking instead of allowing direct walk-through.
8. OnFoot HUD remains separate from cockpit/fleet telemetry.

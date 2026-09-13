# Pass1023-1097 — Visual, Authoring, UI and Generator Parity

Baseline: certified GitHub `main` commit `9440a5254ee80ab901f5c2d67e6a5bc14ae8423a` (`QG-20260913-092557-full-54aaeb65`).

## Pass1023-1047 — visible fidelity, hard size/class authority, staged placement

- Full Gate verifies/materializes the governed Various Planets GLB before certification; configured surface/cloud files are checked individually.
- Missing authored ship base-color maps use deterministic semantic fallbacks instead of featureless gray.
- Existing `UniversalKitbashAuthority` XS→XL scaling becomes the module-size authority.
- `ShipClassRoleSystem` owns legal module tiers, physical hull-length envelopes and module-count limits.
- `ShipClassGenerationAuthoritySystem` measures real placed module bounds and scales generated ships into the selected class envelope.
- Palette drag release creates a staged cyan part rather than immediately attaching it.
- Staged parts support move/rotate/scale, snap-candidate cycling and explicit Confirm/Cancel.
- Broad hull surfaces expose denser editor snap fields.

## Pass1048-1070 — measured surfaces, character scale, semantic modeling, cohesive bake plan

- Generic snapping prefers measured planar contact surfaces with sufficient area/confidence over bounding-box guesses.
- Typed sockets, collision/clearance and bounded insertion depth govern final attachment.
- `1 world unit = 1 meter`; the canonical 1.80 m player envelope governs doors, corridors, decks, seats, consoles and interaction reach.
- Model/Add Shape objects can be assigned Generic, Seat, Table, Storage, Console, Door, Hatch, Airlock, Bed, Workbench or Fixture purpose from the visible Model workspace.
- Saved modular assemblies retain editable source and derive a cohesive exterior/interior bake plan that unions the silhouette, removes buried faces and preserves carved interior portals.
- The deterministic bake plan is implemented; arbitrary final triangle CSG/weld execution remains follow-up work and is not claimed complete here.

## Pass1071-1080 — FPS/remote-fleet authority and conformal shield performance

- Cockpit FPS is normal piloting authority.
- On-foot FPS is normal embodied interior authority.
- Tactical overhead is normalized as Remote Fleet Command.
- Shipyard is an authoring/dev camera over the same live runtime.
- Shield presentation targets one hull-conformal layer approximately 0.3048 m from the visible exterior.
- Buried module faces are filtered from the shield shell.
- Water-like calm motion and point-impact ripples are bounded by distance/count LOD so 100+ ship scenes can decimate or collapse to strategic-only representation.

## Pass1081-1089 — universal dark UI/docking

- `SubspaceUiTheme::Dark` is the project-wide token authority for runtime, Shipyard/dev, Remote Fleet Command, stations, dialogs and tooling.
- `SubspaceDockSystem` provides resizable splitters, tab stacks, floating/redockable panels and per-panel opacity.
- Opacity is clamped to a readable range; critical surfaces can forbid closing/floating/resizing/pointer pass-through.
- Scrollbars, inputs, popups, hover/focus and panel styling share the same tokens.
- Shipyard defaults toward Asset Browser + central 3D Viewport + Outliner/Properties with optional Generator/Systems and collapsible Validation/Console lanes.

## Pass1090-1097 — Blender/runtime generator parity

- `GeneratorParitySystem` registers Ship, Shuttle, Rover, Station, Interior, Prop, Character, Planet World and Solar System domains.
- Blender `SubspaceShipyard` consumes the same request contract instead of owning separate procedural math.
- Canonical transport is `subspace.generator-request.v1` carrying domain/profile/seed and supported role/class/size inputs.
- Parity does not fake missing runtime generators: incomplete domains remain incomplete in both clients until their shared runtime authority exists.

## Certification before Windows PCC

- Pass1023-1047 acceptance: **25/25 PASS**.
- Pass1048-1097 acceptance: **50/50 PASS**.
- Cumulative pass-scoped acceptance: **75/75 PASS**.
- Both source certification gates: **PASS**.
- Planet materializer and Blender addon Python compile: **PASS**.
- Modified authoring/modeling/module/UI/class/Shipyard core and native renderer compile under C++17 `-Wall -Wextra -Werror` in the portable validation slice.
- The archived Sep12 reconstruction lacks several files already present in pushed baseline `9440a52`; therefore the real Windows project-owned PCC Full Gate remains promotion authority for the whole repository.

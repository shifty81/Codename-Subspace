# Codename Subspace Status

## Current source line

**Pass1202R1 — PCC Commit/Push Idempotency Repair candidate** on top of certified GitHub commit `7e9fda2e19b436d7f9cde90175a1dcc69615fba5` (Pass1163-1202 Embodied Authoring Foundation).

## Promotion state

- Baseline Full Gate: **GREEN** (`QG-20260913-184114-full-7478ecb7`).
- Baseline certified source commit: **`c9754358185115a4fd929f3e79a4e8bae676b3b3`**.
- Pass1098-1122 established generator integrity: deterministic requests, true 3D bounds, fail-closed promotion and removal of giant whole-ship scale inflation.
- Pass1123-1162 converged deterministic safe-draft generation, View-space transforms, one authoring Undo/Redo history and native Forge/PCC project tools.
- Pass1163-1202 makes `Full3D` flight physically six-degree-of-freedom, establishes FPS embodiment/playtest camera authority, repairs Shipyard drag projection against the 6DOF construction camera, adds professional primary/developer workspace separation, and establishes aperture/hangar/cohesive-shield contracts.
- Pass1163-1202 was certified GREEN as `QG-20260913-201834-full-ce3e7748` and successfully reached GitHub as commit `7e9fda2e19b436d7f9cde90175a1dcc69615fba5` despite option 2 reporting a false FAIL afterward.
- Pass1202R1 repairs that option-2 post-commit lineage/reporting defect and adds explicit remote-head verification.
- Authoritative Windows project-owned PCC **Full Quality Gate is required after applying R1** before the repair itself may be committed.

## Highest-impact current truths

1. `GeneratorParityRequest` remains the canonical generator input contract. Generate does not mutate seed; Reroll alone changes it.
2. Generated ships may not satisfy class/XS-XL by extreme whole-ship scaling. Unsafe/collapsed candidates fail closed.
3. `PlayerControlSystem::Full3D` now applies local forward/right/up thrust and pitch/roll/yaw torque in real three-dimensional physics.
4. Cockpit flight, on-foot FPS and Remote Fleet Command are separate control perspectives over the same persistent simulation.
5. On-foot embodiment is player-scale, look-relative and traversal-bound aware rather than permanently top-down/prototype-room constrained.
6. Shipyard pointer projection follows the explicit construction camera focus plane instead of the old gameplay Z=0 plane.
7. Shipyard primary workspaces are Build / Interior / Systems / Appearance / Test; deeper Model/Character/PCG/World/Project/Authoring tools remain available as advanced workspaces.
8. Doors, windows, airlocks, docking collars, hangar doors and force fields are first-class aperture concepts. Hangars validate against physical vehicle envelopes.
9. Exterior modules describe shape/placement/interface opportunities; connected interior program + systems compile actual ship capability.
10. Final shield authority is one cohesive smoothed closed envelope with micro-detail suppression and fleet-scale LOD; the visible renderer still needs migration to that source.
11. The legacy ship showcase synthesizer and station stacking preview are still temporary candidate sources behind integrity gates.
12. Quaternius character/animation/interior packs remain governed external-source dependencies pending Vault intake and metric calibration.

## Next implementation lane

- wire native mouse-look and input contexts into cockpit/on-foot first-person cameras;
- connect Shipyard Play/Test controls to the visible runtime button/workspace and actual interior collision/nav products;
- add sub-object/region selection and use articulated regions to carve doors, ramps, hangar panels and moving fixtures from authored geometry;
- route cohesive exterior bake output into the shield renderer so the visible shield becomes one connected atmosphere-capable shell;
- calibrate the governed Quaternius player model and animation libraries against the 1 m world / 1.8 m player contract;
- ingest and classify Quaternius sci-fi interior vocabulary through Vault;
- replace the temporary ship/station candidate generators with capability/interior/topology-first synthesis;
- then migrate Remote Fleet Command from faux-3D strategic projection to true 3D 6DOF camera/order placement and hierarchical fleet/armada command.

See `docs/passes/PASS1163_1202_EMBODIED_AUTHORING_FOUNDATION.md` and `docs/passes/PASS1202R1_PCC_COMMIT_PUSH_IDEMPOTENCY.md`.

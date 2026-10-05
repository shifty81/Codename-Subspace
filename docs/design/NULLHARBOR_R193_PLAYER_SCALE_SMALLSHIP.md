# NullHarbor / Codename Subspace R193 — player scale + small-ship interior authority

R193 deliberately narrows development to the two failures blocking useful FPS testing: the player is still an invisible capsule and the starter interior is still effectively a generic cavity. The final exterior assembly remains editable source authority; interiors are derived products.

## Locked order

1. Repair Pilot 6DOF input/camera agreement: W/S nose forward/reverse, A/D lateral strafe, Q/E roll, Space/Ctrl local vertical thrust, mouse X yaw, mouse Y pitch, Alt+mouse head-look.
2. Hydrate a governed Quaternius Universal Base Characters Standard payload locally. Do not bundle third-party archives in the source patch.
3. Load one adult base as a real skinned player, measure its natural standing bounds in metres, and promote that measured height into `WorldScaleProfile`.
4. Retarget Universal Animation Library 2 locomotion/actions against the same humanoid rig. Locomotion remains gameplay-authoritative rather than root-motion authoritative.
5. Certify a one-room Cutter interior, then Shuttle, then single-deck Corvette. Multi-deck ships wait.
6. Replace the present axis-aligned envelope shell backend with a rotation-aware/arbitrary cohesive-mesh inner-shell boolean only after the small-ship acceptance ladder is stable.

## Player scale rule

`1 world unit == 1 metre` remains fixed. `1.80 m player` is no longer a hard target once a real character is certified. The player model is imported at its natural physical height (after source-unit conversion), and that measured height drives eye height, shoulder width, reach, doors, corridors, interior cells, deck clearances, seats and authoring guides. Existing content outside tolerance must be reviewed rather than silently rescaled around an invisible capsule.

## Interior generation rule

The editable exterior recipe is not destructively hollowed. The production chain is:

`assembly -> cohesive exterior -> derived interior cavity union -> inner pressure shell -> openings/portals -> traversal/collision -> authored furnishings`

The existing `ShipInteriorCarvingSystem` already excludes engines, weapons, wings and surface machinery, applies a human-scale pressure-hull inset, derives connected walkable cavities from the final assembly and validates graph cohesion. `ShipInteriorDerivedShellSystem` removes internal overlap faces, cuts door openings and shares those surfaces with renderer and traversal. Its first backend is intentionally axis-aligned and must continue to fail closed on unsupported rotation rather than fabricate collision.

## Small-ship certification ladder

- **Cutter:** one deck, 1-2 walkable cavities, physical cockpit required. This is the first live player test hull.
- **Shuttle:** one deck, 2-4 connected cavities, cockpit and physical airlock required.
- **Corvette:** one deck, 3-8 connected cavities, cockpit and physical airlock required. This proves a useful multi-room ship before stairs/elevators appear.

Every tier must clear the measured player height/width envelope, produce a ready derived shell, connect every walkable cavity back to command/root, and retain explicit excluded machinery volume.

## R193 code cutover

The included transactional migration repairs the current cdb0f0b cockpit path without overwriting unrelated local edits:

- transient mouse yaw/pitch bypass the held-key response filter that previously attenuated one-frame raw mouse deltas;
- cockpit and on-foot first-person poses inherit the ship's full pitch/roll/yaw transform;
- the authored interior inherits the same three-axis transform and renders in both walking and seated first-person modes;
- regression assertions cover level W having zero accidental vertical thrust, Q/E roll signs, Space local-up thrust, immediate mouse yaw/pitch torque, and nose-relative W after deliberate pitch.

This does not claim that the Quaternius skinned renderer is complete. R193 establishes the source/scale contract and small-ship acceptance boundary so the next runtime pass can implement the real mesh/skin/animation path against one stable authority.

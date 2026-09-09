# Subspace Sandbox Authority — 2026-08-31

## Top-level identity

Subspace is a **sandbox-first solo/PvE/co-op space game** with an optional reactive story spine. Its industrial systems are deep, but they are one career family among combat, exploration, salvage, trade, logistics, fleet command, station development and planetary development.

The intended high-level reference blend is:

- EVE Online: breadth of space careers, fitting, local economy, stations, fleets/corporations, exploration and strategic danger.
- Avorion: modular construction, procedural sandbox and owned fleets.
- C-Beams: direct Newtonian-ish handling, maneuvering thrusters, docking skill and tactical space action.
- Main Sequence: industrial expansion, modular vessels/stations and automation/logistics inspiration.
- Factorio/Dyson Sphere Program: production-network readability and large industrial optimization, primarily in Planetary Manufacturing rather than every ship interior.
- X4: hired captains, owned-fleet assignments and strategic automation.

No reference game's names, art, lore, UI assets or proprietary content are to be copied.

## Simulation/presentation split

Authoritative ship combat/flight remains a 2D X/Y simulation with Newtonian-ish inertia. Rendering may use 3D geometry, perspective, lighting, fog and astronomical depth. Camera yaw never changes the ship's physical forward vector.

## Spatial hierarchy

```text
Galaxy
  -> Solar system (astronomical authority)
     -> Orbital / belt / ring / salvage / station / deep-space region
        -> Streamed local cells
           -> Tactical simulation
```

Only the relevant local/regional content is fully materialized/rendered. Whole-system understanding belongs to the system map.

## Celestials

- Each normal solar system has a stellar lighting authority.
- Planets and stars retain massive physical scale.
- Nearby planets/rings may occupy most of the background.
- Ring systems are real resource macro-regions.
- Ships do not conventionally land on planets.

## Planetary development

```text
Discover
 -> Preliminary orbital scan
 -> Detailed/industrial survey
 -> Evaluate value/hazards
 -> Acquire permission/claim when required
 -> Deploy orbital tether seed
 -> Deliver construction material
 -> Complete space elevator
 -> Unlock Planetary Manufacturing
 -> Extract/process/manufacture
 -> Export through elevator/orbital terminal
```

The survey record must expose enough economic/environmental information for the player to decide whether the investment is worthwhile.

## Industry hierarchy

### Ships
Field independence: cargo/storage, slow furnace/refinery, compact GUI fabricator, repair, ammunition/basic parts, specialized mining/salvage equipment.

### Stations
Precision/heavy industry: modules, weapons, research, advanced processing, repair/refit, ship construction and logistics.

### Planets
Bulk strategic industry: extraction, mass processing, advanced materials/components, large logistics networks and orbital export.

**Rule:** ships consume the economy and can support themselves, but ordinary ships should not trivially replace the wider economy.

## Ship construction

A ship is a modular physical design. Installed modules generate its real bill of materials, mass, power load, heat, propulsion/fitting capacity and construction time. Hull/frame labels describe construction scale; role specialization emerges from installed systems. Hazard certification emerges from actual vessel/fleet protection.

Starter vessels must include command, propulsion, maneuvering, reactor, cargo/storage, furnace and field fabricator capability.

## Fitting/careers

Supported equipment/career families include kinetic/energy/missile combat, point defense, mining, salvage, tractor systems, drones, EWAR, fleet support, shields and sensors. Players should be able to buy rather than personally manufacture most goods, allowing industry to remain optional as a career.

## Stations

Freeform modular stations may be built wherever location/permission/environment rules permit: planetary/moon orbit, belts/rings, salvage sites, trade junctions, deep space and strategic frontier positions. A station's role emerges from its modules.

## Economy

Markets are local. NPC agents/corporations must ultimately mine, refine, manufacture, haul, consume and replace losses so prices and contracts have systemic causes. Pass219 establishes the aggregate NPC economic/LOD foundation; richer market coupling is still required.

## Exploration

Major obvious objects are discoverable through normal sensors; hidden signatures use probe/survey gameplay. Discoveries can escalate into deeper sites, temporary Subspace routes, story breadcrumbs or dangerous expedition regions. Information/bookmarks are persistent navigation assets.

## Fleet/corporation

The player begins as effectively a one-person corporation and can hire crew/captains, own multiple ships, construct stations/planetary infrastructure, and assign co-op permissions. Fleet composition should reward specialists: logistics, repair, EWAR, hazard support, mining/salvage, command and combat.

## PvE danger model

EVE-like geographic risk is adapted into NPC/environmental danger rather than involuntary PvP. Radiation, thermal, cryogenic, ion, corrosive, debris, gravitic and deep-space endurance requirements can demand specialized vessels or support ships.

Sector crises can be solved through multiple careers, not combat alone.

## Story

The story is optional/reactive and lives inside the sandbox. It may reveal mysteries, locations and world-state changes, but never locks basic sandbox professions behind mandatory missions. Completing story arcs does not end the save.

## Camera

- Ship flight: full 360-degree visual orbit and wide tactical zoom.
- Tactical/fleet: full orbit, wider strategic framing.
- Docked hangar/builders: full inspection orbit.
- Planetary Manufacturing: planet/region inspection camera.
- On-foot if used: 360 yaw but deliberately narrow zoom/pitch envelope.

## Non-negotiable physicality principle

Where practical, a visible object corresponds to a real system: thruster -> force/torque; cargo module -> storage; turret -> fitted weapon; elevator -> actual throughput; station module -> service; survey result -> generated planet attributes; industrial route -> actual economic throughput.

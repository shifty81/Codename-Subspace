# R6 Operational Supersession — Standalone PCC

**Effective 2026-09-12:** The design/architecture findings below remain cumulative. The R5 sections that designated ForgePY as the active Subspace patch/build front end are **operationally superseded** until Forge is fully integrated into Cortex and explicitly promoted. Current authority is the project-owned internal PCC documented in `docs/STANDALONE_PCC_AUTHORITY.md`.

The current operator flow is: root `.patch` -> internal PCC startup prompt -> transactional project-owned apply -> option 1 Full Quality Gate/GREEN -> manual gameplay testing -> option 2 guarded GREEN commit + GitHub push.

---

# Codename Subspace — Project Alignment Audit & Roadmap

**Audit baseline:** GitHub `main` at `1f714de6ea5c86ae45d38e01cd6167ca3dc6e5d3`  
**Audit date:** 2026-09-11  
**Purpose:** Stop breadth-first drift, establish one authoritative game/engine direction, accurately distinguish scaffolded systems from player-ready systems, and define the next development gates before further large feature passes.

---

## 1. Executive assessment

Codename Subspace is no longer an early prototype in the sense of “nothing exists.” It has a broad native C++ runtime, a serious ProjectOps/quality-gate spine, hundreds of gameplay/editor source files, a large Shipyard code surface, certified kitbash content, procedural ship/faction/class systems, economy/faction/fleet/home/interior/planet foundations, and a current certified run in which all 81 registered CTest targets passed.

The project’s primary problem is now **alignment and depth**, not lack of ideas or lack of classes.

The repository has accumulated several partially competing identities:

1. an older Avorion/voxel-inspired ship sandbox;
2. a C-Beams/EVE-like planar tactical ship game;
3. a persistent-home + roguelite-expedition industrial game;
4. a newer embodied No Man's Sky × EVE × Star-Citizen-like universe;
5. a large native editor/Shipyard/tooling project;
6. a growing strategic X4-like faction/economy/fleet simulation.

All of those contain useful ideas, but they cannot all remain equal architectural authorities.

### Recommended north star

> **Codename Subspace is an embodied persistent space sandbox, strategy and RPG in which the player can personally fly, walk, salvage, mine, build and explore while also growing into fleet, industry and corporate command.**
>
> Exploration and procedural discovery take cues from No Man’s Sky; physical ships, interiors, engineering, cargo and EVA take cues from Star Citizen; fitting, economy, corporations, security and territorial consequence take cues from EVE Online; X4 supplies the simulation glue for fleets, stations, production and a living economy; Starship EVO supplies high-value construction UX ideas; C-Beams supplies readable tactical ship handling; Factorio supplies the persistent industrial-home loop.
>
> These are reference pillars, **not feature checklists to copy**.

The game should be capable of transitioning naturally between **being the pilot** and **being the commander**, rather than choosing permanently between an FPS/vehicle sandbox and a strategic space simulation.

---

## 2. Evidence baseline: what is actually real today

The latest project evidence shows:

- Native C++ is the active runtime/build authority.
- The current quality gate has 81 registered CTest targets and the latest certified run passed 81/81.
- Runtime smoke launches `subspace_game.exe`, initializes native systems, loads the certified Shipyard content and exits cleanly.
- The project inventory reported 450 `.cpp` and 465 `.h` files under `engine/`.
- The certified Greyoxide Shipyard source contains 156 modules.
- The runtime generated 142 deterministic ship recipes in the inspected run.
- ProjectOps/supply-chain/source-continuity/root-audit/build/test/runtime-smoke infrastructure is materially stronger than most gameplay systems.
- Pass791–890 introduced useful architectural foundations around source authority, assemblies, interiors, PCG, render fidelity and completion truth.
- The Pass890/891 semantic ship-authority regression was repaired and the repaired lineage/class behavior is now committed; temporary recovery utilities were removed from `main`.

This means the project has a strong **development control plane**.

It does **not** mean the game is “81/81 complete.” Tests currently prove contracts and regressions; many gameplay systems remain maturity 1–3 on the scale defined below.

---

## 3. Completion truth: replace percentage completion

Do not use a single hand-maintained “78% complete” style number again. It gives false confidence because a class, header, source file and unit test can exist while the player cannot meaningfully experience the feature.

Use this maturity model everywhere:

| Level | Meaning | Minimum evidence |
|---|---|---|
| 0 | Absent | No authoritative implementation |
| 1 | Contract / Scaffold | Types/interfaces/data contract exist |
| 2 | Core Logic | Meaningful isolated behavior and tests exist |
| 3 | Runtime Integrated | Participates in current runtime and survives regression gates |
| 4 | Player-Usable | Complete end-to-end player workflow with UX/content/save integration |
| 5 | Production Ready | Polished, performant, resilient, balanced, documented and regression/performance certified |

A feature does not reach level 4 merely because:
- a system class exists;
- a unit test passes;
- it runs in a smoke test;
- a placeholder UI can trigger it;
- one generated demo asset shows it.

This maturity model should drive generated `STATUS.md`, the roadmap, ForgePY status, release gates and all future “continue N passes” work.

---

## 4. The critical contradictions that must be retired

### 4.1 2D X/Y flight vs embodied full-3D universe

`docs/SUBSPACE_SANDBOX_AUTHORITY_2026-08-31.md` represents an older authority where ship flight is fundamentally planar and conventional planetary landing is not part of the model.

That conflicts with:
- the newer NMS/EVE unified master direction;
- cockpit/direct flight;
- modular walkable interiors;
- EVA;
- docking/hangars;
- planetary/surface gameplay;
- carrier and landing-craft concepts;
- full spatial ship construction.

**Proposed lock:** full 3D hierarchical simulation is authoritative. A tactical plane can remain an EVE/C-Beams-like command, camera, autopilot or combat-assist mode.

This preserves tactical readability without making the universe mathematically 2D.

### 4.2 Roguelite expedition identity vs persistent sandbox identity

The persistent-home/expedition direction contains valuable loops:
- prepare at home;
- launch expedition;
- manage risk;
- return with spoils;
- automate home industry;
- unlock larger capabilities.

But making this the entire game identity unnecessarily constrains the NMS × Star Citizen × EVE sandbox.

**Proposed lock:** expeditions/extraction are a configurable campaign/progression loop. The persistent sandbox remains the project identity.

### 4.3 Voxel-first construction vs current modular/assembly direction

Older guides still describe voxel construction as if it were universally authoritative. The current Shipyard and recent architecture are much closer to modular authored assets plus assembly semantics.

**Proposed lock:** use a hybrid construction authority:
**Sparse Parametric Assembly + Authored Kitbash + Semantic Sockets + Hierarchical Subassemblies.**

Voxel/occupancy representations remain useful where they are technically ideal—terrain, proxy atmosphere/room occupancy, damage fields or specialized assets—but should not be the universal ship storage model.

---

## 5. Reference-game research: what to borrow and what not to borrow

### No Man's Sky — exploration and scalable procedural discovery

Useful lessons:
- procedural planetary/system variety;
- strong scan/discovery loop;
- base construction and persistent resource extraction;
- settlements as systemic content;
- game modes/rulesets instead of forcing one progression style;
- preservation/migration of existing player worlds as generation evolves.

Do **not** interpret this as a requirement to immediately build fully seamless planet-sized terrain across every planet. Subspace should first prove streamed high-fidelity surface zones connected to a planet-scale strategic/orbital representation.

Official references:
- https://www.nomanssky.com/worlds-part-ii-update/
- https://www.nomanssky.com/frontiers-update/
- https://www.nomanssky.com/beyond-update/

### Star Citizen — embodiment and physical ship systems

Useful lessons:
- cargo/items as world-interactable objects rather than menu-only numbers where gameplay benefits;
- component access, diagnostics, repair and replacement;
- resource networks and failure propagation;
- multicrew stations and MFD-style control;
- ship handling differentiated by mass, thrust and role;
- EVA/on-foot/ship/cargo continuity.

Do **not** copy its production scope. Subspace should prove these ideas in one Golden System first.

Official references:
- https://robertsspaceindustries.com/en/comm-link/transmission/20935-Engineering-Gameplay-Guide
- https://robertsspaceindustries.com/en/comm-link/engineering/14677-Design-Notes-Cargo-Interaction
- https://robertsspaceindustries.com/en/comm-link/engineering/12936-Engineering-Ship-Components-Systems

### EVE Online — fitting, economy, organizations and sovereignty

Useful lessons:
- clear fitting constraints and simulation/planning;
- module roles and meaningful tradeoffs;
- player/corporate economic consequence;
- security/risk geography;
- sovereignty infrastructure linked to resources and strategic upgrades;
- specialization rather than “one best ship.”

Subspace should combine EVE-like strategic clarity with physically represented ship spaces only where physicality creates gameplay.

Official references:
- https://support.eveonline.com/hc/en-us/articles/213287845-Fitting-Window
- https://support.eveonline.com/hc/en-us/articles/14339751569436-Sovereignty-Hub

### X4 — the missing simulation glue

X4 is arguably the most important reference for tying the other pillars together:
- fly ships directly or command fleets remotely;
- construct stations;
- factions expand/react;
- production and trade form a dynamic economy;
- ships/stations/factions exist in the same economic simulation.

Subspace needs this “middle layer” so exploration, combat, salvage and industry affect a living universe rather than isolated minigames.

Official reference:
- https://www.egosoft.com/games/x4/info_en.php

### Starship EVO — construction UX and scalable bricks

Current official/store material confirms especially relevant concepts:
- stretchable bricks;
- grid scales from 0.125m to 32m;
- paint and decals;
- player-built interiors;
- hinges/sliders/rotors/pistons;
- artificial gravity/life support/logic;
- survival resource progression.

A 2021 developer announcement also explicitly describes moving away from its original voxel storage toward rescalable brick construction to reduce memory and enable massive ships.

Useful construction lessons should be adapted, not copied.

References:
- https://store.steampowered.com/app/711980/Starship_EVO/
- Steam developer announcement commonly titled “Neovox Update”

### C-Beams / Avorion / Factorio

Use them as secondary design references:
- C-Beams: readable top-down/tactical combat, inertia, retro-burns, subsystem combat feel.
- Avorion: player-built ship language, fleet/industry sandbox, procedural modularity.
- Factorio: visible production graphs, logistics, throughput, automation and factory scaling.

They should not override the central embodied-space-sandbox identity.

---

## 6. Authoritative world architecture

### 6.1 Four simulation layers

Use explicit simulation layers instead of trying to run the whole galaxy at local fidelity:

#### Layer A — Galactic strategic
Tracks:
- factions;
- territory/security;
- macro economy;
- trade lanes;
- industrial capacity;
- strategic fleet state;
- wars/diplomacy;
- discoveries;
- system-level scarcity and demand.

Tick rate can be low. No detailed rigid-body simulation.

#### Layer B — System/orbital operational
Tracks:
- active fleets;
- ships/stations;
- orbital locations;
- encounters;
- local trade/mining;
- traffic;
- patrols;
- route interception;
- station services.

Still not every interior/component at full fidelity.

#### Layer C — Local high-fidelity bubble
Tracks:
- actual ship flight and collision;
- weapons/projectiles;
- detailed damage;
- docking;
- interiors;
- cargo;
- EVA;
- salvage;
- characters;
- local station spaces.

This is where rigid-body physics belongs.

#### Layer D — Streamed surface zones
Tracks:
- terrain;
- buildings;
- settlements;
- resources;
- factories;
- local vehicles;
- characters/fauna;
- combat;
- caves/POIs.

Planetary strategic state persists outside loaded zones.

### 6.2 State handoff is a first-class system

Every entity needs a canonical identity independent of representation.

Example:
`StrategicFleetRecord -> OperationalFleet -> LocalShips -> StrategicFleetRecord`

Likewise:
`PlanetPOIRecord -> LoadedSurfaceZone -> PlanetPOIRecord`

Do not make “unloading” mean “destroying the real state.”

---

## 7. Coordinate and physics architecture

### Proposed coordinate hierarchy

`Galaxy -> System -> Orbital/Planet Frame -> Local Zone -> Assembly -> Subassembly -> Interior/Character`

Global locations should use either:
- cell/sector integer coordinates + double local offsets; or
- another equivalent large-world-safe representation.

Rendering should be camera-relative.

Construction should use **quantized metric units**. Recommended prototype target: 1/32 m or 1/16 m. This does not mean storing empty voxel cells.

### Physics

A single physics world should never span a solar system.

Use local physics spaces/bubbles tied to the frame hierarchy.

Jolt Physics is a strong candidate because it is C++, multithread-oriented, has optional double-precision positions, moving-character support and permissive licensing. Its own architecture documentation explicitly warns that space simulations still face float broadphase/velocity/quaternion precision concerns, reinforcing the local-frame approach.

Candidate:
- https://github.com/jrouwe/JoltPhysics

Do not migrate physics merely because Jolt exists. Create an adapter and a certification fixture first.

---

## 8. Shipyard/construction: the architectural lock that solves multiple recurring failures

### 8.1 One universal BuildElement vocabulary

Proposed types:

- `ParametricPrimitive`
- `AuthoredModule`
- `FunctionalModule`
- `StructuralConnector`
- `SurfaceDetail`
- `Decal`
- `SubAssembly`
- `LogicComponent`

Common identity/data should include:

- `ElementId`
- `AssemblyId`
- `ParentFrameId`
- element type / asset or shape ID
- quantized local transform/dimensions
- material/livery/decal state
- structural role
- functional role
- mass/HP/armor
- typed attachment faces/sockets
- interior/airtight metadata where relevant
- faction/class/hull/role/manufacturer provenance

### 8.2 Parametric geometry is for structure, not everything

Good parametric elements:
- armor plate;
- hull plate;
- floor;
- wall;
- bulkhead;
- beam;
- truss;
- structural spine;
- corridor;
- ramp;
- simple window band;
- catwalk;
- cowling.

Keep authored discrete modules for:
- cockpit/bridge;
- engine/thruster;
- reactor;
- shield generator;
- turret/weapon;
- refinery;
- sensor;
- landing gear;
- hangar machinery;
- complex functional interior modules.

This solves the recurring “random modules in a line / floating engine / no correct connector exists” problem.

### 8.3 Structural connectors become generated elements

PCG or the player can request:
`Connect Face A -> Face B within clearance and style constraints`.

The connector generator produces faction-styled:
- armored spine;
- truss;
- ribbed bridge;
- curved shell;
- industrial frame;
- alien/rare-family connector.

The result is still a real BuildElement with structural semantics.

### 8.4 Local-face placement

Every hittable construction surface exposes:
`origin + tangent + bitangent + normal`.

Shipyard placement modes:
- Assembly Grid
- Face Grid
- Socket Grid
- Free Transform

This makes micro-detail, maneuvering thrusters, armor, windows, sensors and decals align properly to angled geometry.

### 8.5 Semantic attachment

Never infer connection solely from touching AABBs.

Attachment domains include:
- Structural
- Armor
- Interior
- Weapon
- Propulsion
- Docking
- Power
- Data
- Fuel/Fluid
- Cargo
- Utility

An engine may require structural + fuel + power/data + exhaust clearance. A turret may require structural + power/data + firing arc.

### 8.6 Editable vs compiled representations

Canonical:
`EditableAssembly`

Derived products:
- structural graph;
- render clusters;
- compound collision;
- nav/clearance graph;
- atmosphere occupancy/rooms;
- power/data/fluid graphs;
- mass/inertia;
- exterior hull envelope;
- inspection diagnostics.

Only dirty affected clusters/graphs rebuild after edits.

This is the architecture that should eventually fix Shipyard lag far more sustainably than micro-optimizing individual draw calls.

### 8.7 Undo/redo, networking and PCG should share commands

All mutations become serializable commands:
- Place
- Delete
- Move
- Rotate
- Resize
- Paint
- Decal
- Socket edit
- Link
- Mirror
- Create subassembly
- Detach

The same command vocabulary can power:
- Undo/Redo
- autosave deltas
- multiplayer replication
- AI/Cortex tooling
- procedural builders
- audit/replay

---

## 9. Ship compilation pipeline

A build should not simply be “a collection of meshes.”

Proposed compiler:

`EditableAssembly`
→ topology validation
→ semantic attachment validation
→ structural graph
→ functional networks
→ interior/portal/clearance graph
→ atmosphere rooms
→ mass/COM/inertia
→ thruster authority / handling
→ weapon arcs
→ collision compounds
→ render clusters
→ exterior envelope/shield shell
→ runtime ship package

Shipyard diagnostics can then report:

- Hull connected: PASS
- Power network: PASS
- Fuel path to engines: PASS
- Port maneuvering authority: WARN
- Turret 3 firing arc blocked: FAIL
- Engine exhaust intersects hull: FAIL
- Interior compartment B inaccessible: FAIL
- Life support cannot pressurize deck 2: WARN
- Center of mass offset: WARN

This should become the same validator used by player builds, PCG ships and NPC/manufacturer blueprints.

---

## 10. Physical ship systems

Combine EVE-like fitting clarity with Star-Citizen-like physical consequences.

### Networks

At minimum:
- power;
- data/control;
- fuel;
- coolant/heat;
- ammunition;
- cargo;
- atmosphere/life support.

These networks should be graph-based and able to fail partially.

### Fitting

Use class/hull/role constraints and clear slots/capability budgets where that improves usability.

Physical access does not mean every tiny abstract upgrade needs a 3D object.

A useful rule:
- if replacing/repairing/stealing the thing creates gameplay, physicalize it;
- if physicalization adds busywork but no decisions, keep it abstract.

### Mass and handling

Construction, cargo and damage should feed:
- total mass;
- center of mass;
- inertia tensor;
- thrust authority;
- braking;
- rotation;
- fuel usage.

This is a better long-term handling model than arbitrary class-only maneuverability constants, while class assists can still normalize player experience.

---

## 11. Damage, repair, salvage and shields

### Damage ladder

1. Cosmetic: scorch/decal/surface shader.
2. Armor/hull state: localized integrity loss.
3. Component damage: subsystem degradation/failure.
4. Network damage: power/data/coolant/fuel interruption.
5. Structural breach: graph connection loss.
6. Catastrophic split: disconnected assembly becomes independent debris/ship section.

Avoid uncontrolled primitive subdivision after every hit.

### Salvage

Intact components should retain:
- identity;
- manufacturer;
- model;
- condition;
- quality/roll;
- repair requirements;
- provenance/history;
- compatibility.

That supports the existing goal of recovering modules from derelicts rather than converting everything into anonymous scrap.

### Conformal shields

Compile an exterior hull envelope from the ship representation, offset approximately 0.3 m / 1 ft, and render the shield against that shell.

Impact data:
- position;
- normal;
- energy;
- timestamp;
- damage type.

The shader can create the desired still-water ripple/shimmer without simulating a huge spherical bubble.

---

## 12. Interiors, EVA and local gravity

The current interior/portal systems should become the data spine for actual player embodiment.

Required path:
- door/portal graph;
- walkable clearance;
- service access;
- navmesh/graph;
- room/pressure identity;
- local gravity frame;
- fire/hazard state;
- character interaction anchors;
- component access points.

A coarse hidden occupancy grid is appropriate for atmosphere sealing and flood-fill. It is a simulation proxy, not visible ship geometry.

Movement inside a moving ship should happen in the ship/local physics frame. EVA transitions to the surrounding local-space frame.

Recast/Detour is a reasonable future navigation candidate:
https://github.com/recastnavigation/recastnavigation

---

## 13. Planet and surface scope

Trying to immediately reproduce No Man's Sky's universal seamless planets would be a severe schedule and engine-risk multiplier.

Recommended progression:

### Stage 1
Planet is a system/orbital object with:
- orbital scan;
- climate/biome/resource metadata;
- settlements/POI records;
- strategic hex/territory data;
- landing zones.

### Stage 2
Landing transitions into a high-fidelity streamed local surface zone whose coordinates map back to the canonical planet.

### Stage 3
Zones can expand/neighbor-stream and roads/rivers/settlements become more continuous.

### Stage 4
Evaluate whether broader seamless spherical traversal is worth the engineering cost.

Nothing in stages 1–3 should make Stage 4 impossible.

---

## 14. Economy, factions and territorial simulation

The existing economy/faction systems are good **level-2 foundations**, but not yet a living EVE/X4-style universe.

Authoritative model should include:

### Economy
- sources/extraction;
- refining;
- recipes/manufacturing;
- transport;
- inventories;
- regional prices;
- demand/sinks;
- NPC/player buy/sell orders;
- scarcity;
- transport risk;
- infrastructure damage;
- market telemetry.

### Faction strategic model
- treasury/resources;
- industrial capacity;
- military strength;
- security posture;
- goals/doctrine;
- diplomacy;
- territory;
- logistics;
- patrols;
- wars;
- contracts;
- colonization/expansion.

### Security
Security rating should influence:
- patrol response;
- piracy;
- hostile/unknown faction spawns;
- salvage/relic rarity;
- insurance/risk;
- mission rewards;
- market transport costs;
- law/contraband.

Planetary hex claims can roll upward into system/sector/corporate sovereignty instead of being a disconnected minigame.

---

## 15. AI architecture

Avoid one monolithic AI.

Suggested hierarchy:

### Strategic AI
Chooses:
- economic goals;
- expansion;
- diplomacy;
- fleet allocation;
- infrastructure;
- war posture.

### Operational AI
Chooses:
- routes;
- missions;
- mining/trading;
- escort;
- interception;
- station defense.

### Tactical ship AI
Chooses:
- target;
- range;
- facing;
- weapon groups;
- repair/retreat;
- formation behavior.

### Character AI
Chooses:
- navigation;
- jobs;
- combat;
- repair;
- firefighting;
- cargo handling;
- interactions.

AI decisions should consume the same authority services and commands players/tools use wherever practical.

---

## 16. Networking scope

Do not make “EVE-like” mean “build single-shard MMO infrastructure now.”

That would consume the project.

Design these boundaries now:
- stable entity IDs;
- serializable gameplay/build commands;
- deterministic generation inputs;
- authoritative ownership;
- snapshot + delta persistence;
- interest management;
- simulation-layer handoff.

But first multiplayer delivery target should be:
**2–8 player co-op session**.

Suggested authority:
- authoritative host/server owns gameplay state;
- clients send intent/commands;
- server validates;
- clients receive state deltas/events;
- procedural/render meshes are generated locally from canonical data where deterministic;
- never network raw mesh buffers as the gameplay state.

Valve GameNetworkingSockets is a possible transport candidate to evaluate later:
https://github.com/ValveSoftware/GameNetworkingSockets

---

## 17. Renderer strategy

Do not rewrite the renderer merely because a newer backend exists.

First make the **data path** correct:

- compiled assembly render clusters;
- batching/instancing;
- material arrays;
- physical-scale projected materials where useful;
- GPU-friendly decal/livery layering;
- asynchronous asset upload;
- LOD/HLOD;
- occlusion/culling;
- render budgets;
- performance telemetry;
- camera-relative coordinates.

Then profile the Golden System.

If the current backend is the limiting factor, migrate through an abstraction rather than coupling gameplay to a renderer.

Diligent Engine remains a candidate for later evaluation:
https://github.com/DiligentGraphics/DiligentEngine

KTX2/Basis is a strong texture-delivery direction:
https://github.com/KhronosGroup/KTX-Software

---

## 18. Golden Vertical Slice — the gate before galaxy expansion

The single most important roadmap change is to stop proving breadth by adding classes.

Build **one excellent home solar system** that demonstrates the whole game.

A Golden System is not complete until the player can:

1. Load a persistent save.
2. Start in a docked/hangar or home context.
3. Walk/interact sufficiently to board a ship.
4. Enter cockpit or command view.
5. Undock.
6. Fly in full 3D.
7. Scan a destination.
8. Travel to an asteroid field/wreck/anomaly/contract.
9. Fight, mine or salvage.
10. Experience component/damage/resource consequences.
11. Recover physical cargo or an intact module.
12. Optionally EVA/board where the content requires it.
13. Return and dock.
14. Sell/refine/manufacture.
15. Fit/repair/refuel/rearm.
16. See market/faction/reputation/world state change.
17. Give at least one drone/wingman meaningful orders.
18. Save, quit and reload with the same authoritative state.
19. Use Shipyard to alter/build a valid ship and then fly that exact result.
20. Maintain acceptable frame-time, memory and load behavior.

Until this works, expanding the galaxy mostly multiplies unfinished content.

---

## 19. Proposed authoritative roadmap

### Phase 0 — Vision / authority / documentation lock

**Goal:** one project identity and one truth system.

Deliver:
- approve/revise ADR register;
- create `PROJECT_VISION.md`;
- create `ARCHITECTURE_AUTHORITY.md`;
- create one `ROADMAP.md`;
- create generated `STATUS.md`;
- install 0–5 maturity model;
- archive superseded direction/status docs;
- explicitly retire 2D-world authority;
- demote roguelite direction to a mode spec.

**Exit:** no two authoritative docs disagree about game identity, world dimensionality, planet scope, construction model or completion truth.

### Phase 1 — Spatial / persistence / simulation foundation

Deliver:
- authoritative frame hierarchy;
- large-world coordinate contract;
- simulation layers/LOD;
- local physics-world contract;
- stable entity identities;
- schema versioning/migrations;
- fix `HomeSystemSaveGame` temporary parser;
- finish FormationSystem placeholder/gaps;
- deterministic state handoff tests.

**Exit:** objects can move between strategic/system/local representations and survive save/reload without identity loss.

### Phase 2 — Shipyard / construction foundation

Deliver:
- BuildElement model;
- quantized metric construction coordinates;
- parametric structural primitives;
- semantic attachment faces/sockets;
- local-face placement;
- generated structural connectors;
- nested subassemblies;
- command-based undo/redo;
- editable -> compiled assembly pipeline;
- render/collision dirty regions;
- CAD-like mouse-first Shipyard UX.

**Exit:** hand-built and PCG test ships compile through the same validator and can be flown without floating/invalid modules.

### Phase 3 — Physical ship / embodiment foundation

Deliver:
- full-3D direct flight;
- mass/COM/inertia integration;
- power/data/fuel/coolant/ammo/cargo networks;
- fitting integration;
- physical damage/repair;
- conformal shields;
- real interior portal/clearance/nav;
- dock/hangar/on-foot state transition;
- early EVA;
- intact-module salvage.

**Exit:** one ship can be operated, damaged, repaired, boarded and salvaged end-to-end.

### Phase 4 — Golden Home System

Deliver:
- home station/industry;
- complete scan/travel loop;
- mine/salvage/combat contract;
- one strong station;
- one surface zone/landing loop;
- market/refining/manufacturing/research;
- NPC/faction presence;
- save/reload;
- player-facing UI/audio/visual baseline.

**Exit:** Golden Vertical Slice checklist passes.

### Phase 5 — Dynamic economy / factions / fleets

Deliver:
- regional markets;
- production chains;
- logistics;
- AI traders/miners/haulers;
- faction budgets/goals;
- territory/security;
- patrols/wars;
- fleet orders/formations;
- stations as economic/military infrastructure.

**Exit:** the home system changes meaningfully without direct player scripting and remains deterministic/persistent.

### Phase 6 — Procedural galaxy / exploration breadth

Deliver:
- galaxy/system topology;
- diverse system families;
- planets/moons/anomalies;
- authored+PCG POIs;
- station families;
- rare/unknown factions;
- wormhole/deep-space risk;
- deterministic discovery records;
- progression-aware procedural content.

**Exit:** multiple systems feel meaningfully different while respecting the same Golden System contracts.

### Phase 7 — Presentation / production scale

Deliver:
- optimized renderer path;
- materials/liveries/decals;
- lighting/space environment;
- animation;
- VFX;
- audio;
- UI normalization;
- large ship/station performance;
- content pipeline and certification;
- backend migration only if profiling justifies it.

**Exit:** representative capital/station scenes meet explicit frame/memory/load budgets.

### Phase 8 — Co-op

Deliver:
- authoritative server/session;
- replication;
- interest management;
- shared construction permissions;
- ship roles/multicrew;
- persistence conflict policy;
- reconnect/recovery;
- 2–8 player certification.

**Exit:** Golden System loop works in co-op without desync/corruption.

### Phase 9 — Expansion

Candidates:
- larger continuous surface coverage;
- deeper settlements/cities;
- advanced multicrew;
- carrier fleet life;
- corporate sovereignty;
- player-run markets/industry;
- broader procedural stories;
- larger persistent online models if justified.

---

## 20. Documentation authority after normalization

Recommended hierarchy:

1. `README.md` — brief entry point only.
2. `docs/PROJECT_VISION.md` — stable product north star.
3. `docs/ARCHITECTURE_AUTHORITY.md` — technical authority and ADR index.
4. `docs/ROADMAP.md` — only current roadmap.
5. `docs/STATUS.md` — generated from gate/maturity evidence.
6. `docs/FEATURE_MATRIX.md` or machine-readable equivalent — feature maturity/evidence.
7. `docs/decisions/ADR-xxxx-*.md` — architectural decisions.
8. `docs/systems/**` — subsystem specifications.
9. `docs/reference/**` — external-game research.
10. `docs/archive/**` — historical, explicitly non-authoritative.
11. `docs/passes/**` — development history only.

No pass document should be able to silently redefine the game.

---

## 21. Documentation audit findings

The documentation problem is structural:

- several overlapping top-level status files;
- historical pass-era completion claims;
- older managed/C# assumptions surviving after native normalization;
- multiple ship-generation guides representing different eras;
- voxel-first documents competing with the current assembly/kitbash direction;
- older planar-flight authority conflicting with embodied 3D goals;
- a later roguelite direction competing with the broader sandbox identity;
- many excellent subsystem documents that should survive, but under a clear authority hierarchy.

The accompanying `Document_Authority_Migration.csv` identifies the immediate KEEP / REWRITE / MERGE / ARCHIVE / DEMOTE actions.

A second normalization sweep should process **every historical guide line-by-line after the ADRs are approved**. Doing that before the architecture locks would rewrite the same documents twice.

---

## 22. Current project strengths

1. **Operational discipline is unusually strong.** Quality gates, source safety, ProjectOps, source authority and snapshotting are real assets.
2. **Native C++ migration is effectively settled.**
3. **The Shipyard has enough code to evolve rather than restart.**
4. **Certified kitbash content gives us a working reference corpus.**
5. **Faction/class/role lineage is now a useful PCG authority.**
6. **There are real foundations for nearly every intended gameplay domain.**
7. **The current codebase is broad enough to build a vertical slice without inventing every subsystem from zero.**

---

## 23. Current project weaknesses / highest risks

1. **Competing design authorities.**
2. **Breadth-first pass accumulation.**
3. **System existence mistaken for completion.**
4. **No single player-facing Golden System gate.**
5. **2D vs 3D conflict at the world-model level.**
6. **Persistence is not mature enough for the amount of future state planned.**
7. **Ship construction lacks one universal canonical representation.**
8. **Renderer/runtime geometry path is not yet designed for truly huge editable assemblies.**
9. **Physical ship systems are fragmented instead of one resource-network authority.**
10. **Embodiment/on-foot/EVA are directionally present but immature.**
11. **Dynamic economy/faction systems are foundations, not yet a living universe.**
12. **Networking could become a catastrophic scope sink if started as MMO infrastructure.**
13. **Seamless planetary scope could become another scope sink.**
14. **Material/art coherence remains behind systems breadth.**
15. **Documentation drift can cause future patches to resurrect retired assumptions.**

---

## 24. Stop / Start / Continue

### Stop
- adding 50–200 unrelated scaffold systems per pass;
- hand-maintained completion percentages;
- allowing old docs to remain architectural authority by accident;
- treating passing source gates as proof of player-ready features;
- making renderer/physics/network rewrites without a measured acceptance need;
- increasing galaxy breadth before the Golden System works.

### Start
- evidence-based maturity levels;
- ADRs;
- Golden System acceptance;
- BuildElement/assembly compiler normalization;
- full-3D hierarchical world frames;
- versioned persistence;
- player-facing integration tests;
- performance budgets;
- architecture linting against retired decisions.

### Continue
- quality-gate discipline;
- ForgePY compatibility;
- source provenance;
- deterministic PCG;
- faction/class/role lineage;
- certified content intake;
- incremental patches;
- snapshot/recovery;
- Shipyard tooling rather than abandoning it.

---

## 25. Immediate milestone sequence

### Milestone A — Alignment Lock
No gameplay breadth. Approve the decision register and rewrite authority docs.

### Milestone B — Truth Foundation
Fix persistence/parser and formation gaps; implement generated maturity/status truth.

### Milestone C — Spatial Foundation
Full-3D frame hierarchy, local physics contract, simulation LOD and state handoff.

### Milestone D — Construction Foundation
BuildElement + parametric structure + semantic sockets + command model + compiler.

### Milestone E — One Real Ship
Build/PCG one ship through the new pipeline; fly it; damage it; walk it; repair it; save/reload it.

### Milestone F — Golden System
Finish the entire loop before expanding galaxy breadth.

This ordering is intentionally narrower than previous 100-pass runs. It is designed to convert the existing breadth into a coherent game.

---

## 26. Decisions to discuss before code changes

The accompanying ADR register marks these as **PROPOSED**, not silently approved.

The most consequential decisions are:

1. Is full 3D now permanently authoritative, with tactical plane only a mode/assist?
2. Is the persistent sandbox the primary identity, with roguelite expeditions as a mode?
3. Do we lock hybrid sparse-parametric + authored-kitbash construction?
4. Do we adopt quantized 1/32m or 1/16m construction coordinates?
5. Do we explicitly defer globally seamless NMS-style planets in favor of streamed high-fidelity surface zones first?
6. Do we lock 2–8 player co-op as the first multiplayer target and reject MMO scope for now?
7. Is one Golden Home System the mandatory gate before more galaxy breadth?
8. Do we make physical ship resource networks (power/data/fuel/coolant/ammo/cargo/life support) a core pillar?
9. Do we keep current renderer/physics backends until profiling shows they block the vertical slice?
10. Do we accept the evidence-based 0–5 maturity model as the only completion truth?

Once these are locked, the documentation normalization and next implementation pass become straightforward.

---

## 27. Source/research index

### Codename Subspace repository
- https://github.com/shifty81/Codename-Subspace
- `README.md`
- `docs/CURRENT_STATUS.md`
- `docs/DEVELOPMENT_STATUS.md`
- `docs/ROADMAP_STATUS.md`
- `docs/NEXT_STEPS.md`
- `docs/WHATS_LEFT_TO_IMPLEMENT.md`
- `docs/FEATURES.md`
- `docs/SUBSPACE_SANDBOX_AUTHORITY_2026-08-31.md`
- `docs/EVE_PVE_COOP_FEATURE_MATRIX_2026-08-31.md`
- `docs/architecture/ARCHITECTURE.md`
- `docs/architecture/ARCHITECTURE_DIAGRAM.md`
- `docs/architecture/SUBSPACE_NMS_EVE_UNIFIED_FOUNDRY_RUNTIME_MASTER_SPEC.md`
- `docs/architecture/PLAYABLE_CLIENT_FRONTEND_PIPELINE.md`
- `docs/architecture/HOME_SURFACE_PRIMARY_VIEW_PIPELINE.md`
- `docs/design/SUBSPACE_ROGUELITE_INCREMENTAL_DIRECTION.md`
- `docs/design/HOME_SOLAR_SYSTEM_SPEC.md`
- `docs/design/SHIPYARD_INCREMENTAL_PROGRESSION_SPEC.md`
- `content/architecture/project_completion_truth_v1.json`
- `scripts/subspace_pass791_890_completion_truth_audit.py`

Representative source reviewed:
- `engine/src/world/SpatialFrameSystem.cpp`
- `engine/src/formation/FormationSystem.cpp`
- `engine/src/core/persistence/HomeSystemSaveGame.cpp`
- `engine/src/flight/ShipFlightControl.cpp`
- `engine/src/interior/ShipEmbodimentSystem.cpp`
- `engine/src/interior/PortalGraphSystem.cpp`
- `engine/src/networking/NetworkSystem.cpp`
- `engine/src/economy/EconomySimulationSystem.cpp`
- `engine/src/factions/DynamicFactionSystem.cpp`
- `engine/src/runtime/PersistentUniverseSystem.cpp`

### External references
- No Man's Sky: https://www.nomanssky.com/
- Star Citizen: https://robertsspaceindustries.com/
- EVE Online: https://www.eveonline.com/
- EVE support/fitting/sovereignty: https://support.eveonline.com/
- X4: https://www.egosoft.com/games/x4/info_en.php
- Starship EVO: https://store.steampowered.com/app/711980/Starship_EVO/
- Jolt Physics: https://github.com/jrouwe/JoltPhysics
- Recast/Detour: https://github.com/recastnavigation/recastnavigation
- Valve GameNetworkingSockets: https://github.com/ValveSoftware/GameNetworkingSockets
- Diligent Engine: https://github.com/DiligentGraphics/DiligentEngine
- Khronos KTX: https://github.com/KhronosGroup/KTX-Software

---

## 28. Bottom line

Subspace should **not restart**.

The codebase has too much useful native infrastructure and too many relevant systems to justify another technology reset.

What it needs is a controlled convergence:

**one vision → one world model → one assembly model → one persistence truth → one feature maturity model → one Golden System → then galaxy breadth.**

The project can still pursue the extremely ambitious “No Man's Sky + Star Citizen + EVE” goal, but the only credible path is to make those references occupy different architectural layers instead of attempting to reproduce all of them simultaneously at maximum fidelity.

The next pass should therefore be **alignment/documentation normalization and architectural contracts**, followed by the spatial/persistence and construction foundations—not another bulk feature expansion.


---

# R2 Addendum — Nullharbor Lineage + Ember Hosted-Project Authority

## 29. Nullharbor is an explicit source-design lineage, not a parallel game

The current master specification already states that Nullharbor's mature cockpit/interior, salvage, station and planetary-installation design work is folded into Codename Subspace's native C++ runtime rather than maintained as a parallel game.

This should be promoted into the project authority hierarchy:

**Codename Subspace** is the unified game/project identity.

**Nullharbor** is a retained source-design lineage whose approved systems are normalized into Subspace.

This avoids two failure modes:
1. accidentally dropping mature Nullharbor work during documentation cleanup; and
2. maintaining duplicate/competing implementations under two game identities.

Nullharbor-origin systems to preserve explicitly include:
- cockpit/direct-flight and embodied ship operation;
- traversable ship/freighter interiors;
- docked-station and hangar transitions;
- EVA/on-foot layers;
- FPS salvage and intact module recovery;
- carrier/hangar/fleet logistics;
- drone mining/salvage/repair/combat roles;
- R.I.G./powered-exosuit progression and tactical-command growth;
- scanning range bands and orbital planetary resolution;
- security ratings, pirates, hidden/unknown factions and deep-space/wormhole risk;
- corporation/fleet/territorial command;
- debt-contract starter scenario and learn-by-doing progression;
- planetary installations and hex-sector territorial gameplay;
- broad gameplay/simulation configuration.

Every retained Nullharbor feature should have:
- a current Subspace owning system/spec;
- a roadmap maturity entry;
- a migration disposition (KEEP / MERGE / REWRITE / DEFER);
- no active duplicate authority under a second game identity.

## 30. Ember hosted-project support becomes a hard requirement

Codename Subspace must be loadable by Ember as a first-class hosted game project while remaining fully independent from Ember.

### Authority boundary

ForgePY:
- universal project/build/control front end;
- invokes Subspace's project-owned CLI/PCC/gates;
- manages artifacts, patches, Git and certification.

Cortex:
- intelligence/project-understanding layer;
- may operate Subspace through approved project tools and Ember/Forge contracts.

Ember:
- universal authoring environment;
- loads Subspace's project descriptor;
- discovers supported workspaces/capabilities;
- edits canonical Subspace authoring data;
- launches validation and runtime workflows through Subspace-owned commands.

Codename Subspace:
- independent native C++ game repository/runtime;
- owns gameplay code, schemas, validators, save formats, content formats, tests, project CLI/PCC and runtime;
- remains buildable/runnable/testable/certifiable without Ember.

### Required project contract

Subspace should expose a versioned project descriptor containing at least:
- stable ProjectId and project version;
- runtime executable and launch profiles;
- project CLI/PCC authority;
- build/test/full-gate commands;
- content roots and asset roots;
- canonical world/scene/entity/assembly schemas;
- BuildElement schema versions;
- PCG authorities;
- validation commands;
- import/export operations;
- editor workspaces/extensions;
- play-from-here / debug-launch contracts;
- artifact/log/debug-bundle locations;
- capability flags and contract versions.

The final universal filename should follow the Ember/Forge ecosystem standard once that standard is locked. Do not hard-code a Subspace-only format if a shared project contract can serve every hosted game.

### Ember must consume canonical data, not shadow copies

Ember and the Subspace runtime must consume the same versioned schemas and canonical authoring records.

Ember may generate:
- thumbnails;
- previews;
- cached render meshes;
- derived collision;
- nav data;
- validation reports;
- temporary working state.

But Ember must not become the authoritative owner of an incompatible copy of:
- ships;
- stations;
- systems;
- planets;
- factions;
- economy definitions;
- quests;
- PCG rules;
- items/modules;
- materials/liveries;
- save/runtime schemas.

### Required Ember workspaces for the Subspace vertical slice

Initial required workspaces:
- Project / Golden System overview;
- Solar-System / celestial editor;
- Shipyard / assembly editor;
- Station assembly editor;
- Interior/portal/clearance editor;
- Planet/surface-zone editor;
- Faction/security editor;
- Economy/market/production editor;
- Encounter/mission editor;
- Item/module/fitting editor;
- Material/livery/decal editor;
- PCG grammar / generation-review editor;
- Validation / diagnostics / runtime launch panel.

These can be universal Ember workspaces backed by Subspace schemas, Subspace-specific extensions, or a combination. Ember core must not acquire hard-coded Subspace gameplay assumptions.

## 31. Golden System is also the Ember certification fixture

The Golden Home System becomes the first complete Subspace content set that Ember must be able to:
1. discover from the project descriptor;
2. open and inspect;
3. edit;
4. validate;
5. launch into the Subspace client;
6. jump directly to the edited context;
7. collect runtime/debug evidence;
8. return to Ember for continued authoring.

This makes Ember compatibility testable instead of aspirational.

A Golden System pass should eventually fail if:
- Ember cannot load required canonical data;
- the runtime interprets saved editor data differently;
- validation differs between editor and game;
- launch context cannot resolve;
- editor-generated content cannot survive save/reload/runtime compilation.

## 32. Construction/PCG authority shared with Ember

The hybrid construction decision becomes more valuable with Ember.

Ember should understand generic concepts:
- BuildElement;
- Assembly;
- SubAssembly;
- ParametricPrimitive;
- AuthoredModule;
- AttachmentFace;
- SemanticSocket;
- MaterialProfile;
- DecalLayer;
- BuildCommand;
- ValidationResult.

Subspace supplies project-specific semantics:
- ship classes and hull families;
- faction/manufacturer lineage;
- reactors;
- engines/thrusters;
- shields;
- weapons/hardpoints;
- fuel/coolant/ammo/data/power networks;
- hangars/docking;
- interior modules;
- life support;
- salvage condition/provenance;
- class/role construction rules.

PCG should be editable/reviewable through a deterministic pipeline:
Faction
→ Manufacturer
→ Class
→ Role
→ Hull Family
→ Functional Topology
→ Structural Skeleton
→ Authored Modules
→ Parametric Connectors
→ Exterior Shell
→ Surface Detail
→ Livery
→ Validation.

The editor should be able to generate batches, inspect failures, compare variants, approve exemplars and tune rules without creating a second runtime generator.

## 33. Revised immediate milestone order

### Phase 0A — Authority lock
- lock project identity;
- lock Nullharbor lineage integration;
- lock full-3D authority;
- lock construction representation;
- lock Golden System;
- lock Ember hosted-project requirement;
- lock evidence-based maturity model.

### Phase 0B — Documentation normalization
- create PROJECT_VISION;
- create ARCHITECTURE_AUTHORITY;
- create ROADMAP;
- create generated STATUS;
- create NULLHARBOR_INTEGRATION_AUTHORITY;
- create EMBER_PROJECT_CONTRACT;
- create ADR files;
- archive/retire conflicting authorities;
- migrate accepted content from overlapping ship/voxel/generation docs.

### Phase 0C — Project interoperability contract
- define universal hosted-project descriptor requirements with Ember/Forge;
- implement Subspace adapter/descriptor;
- expose project-owned build/test/validate/run/debug operations;
- add capability discovery;
- add contract-version checks;
- add smoke validation that Ember can open the project without owning the runtime.

### Phase 1 onward
Continue with the spatial/persistence foundation, construction foundation, physical ship/embodiment work and Golden System described in the main audit.

The important sequencing rule is:
**Ember compatibility is designed into the canonical schemas now, but Ember is not allowed to block Subspace from independently building or running.**


---

# R3 Addendum — World Continuity, Persistent Ships, Interior Cells and Travel Authority

## 34. Reference Solar System terminology

Retire the temporary phrase **Golden Home System**.

Use:

> **Reference Solar System** — the first complete production-quality solar system used to certify the full Subspace world, travel, ship, surface, interior, economy, faction, construction and persistence architecture before procedural galaxy expansion.

The Reference Solar System is not a small home map. It must contain genuinely large explorable planetary surfaces and the complete space-to-surface travel loop.

## 35. Player-visible continuity rule

The primary exploration loop is continuous:

`system space -> orbit -> atmosphere -> surface -> landing -> on-foot/vehicle exploration -> takeoff -> atmosphere -> orbit -> system space`

There is no player-visible loading scene during normal:
- planetary approach;
- atmospheric entry;
- atmospheric flight;
- landing;
- taking off;
- surface traversal;
- moving between on-foot, rover, hover bike, mech and landed ship;
- returning from a planet to space.

The engine may stream, rebase frames, change simulation LOD, swap terrain representations and prefetch content underneath the player. Those implementation details may not become arbitrary loading screens.

## 36. Diegetic cell-transition exceptions

Subspace intentionally supports loaded interior/underground cells when that gives better density, performance or authored control.

Approved transition types include:
- station hangars and drydocks;
- elevators descending into deep underground POIs;
- mine shafts;
- bunker lifts;
- sealed research facilities;
- building interiors;
- large authored dungeon/ruin interiors;
- pressure airlocks;
- transit doors or vestibules;
- other physically justified transition chambers.

The desired reference is the **Fallout-style exterior-world -> authored interior-cell** model, but the transition should be presented through an in-world action rather than an unexplained loading screen.

Examples:

`planet surface -> mine headframe -> elevator ride -> underground POI cell`

`city street -> building door/vestibule -> building interior cell`

`station exterior approach -> assigned hangar -> hangar/docked cell`

Each cell must retain a canonical relationship to its exterior entrance and parent world.

## 37. Interior-cell persistence

Loaded cells are not disposable minigame maps.

Each persistent or revisitable cell needs:
- stable CellId;
- parent world/body/station ID;
- entrance/exit anchors;
- persistent object IDs;
- loot/container state;
- door/security state;
- destruction/repair state where supported;
- NPC/quest state;
- faction/security ownership;
- discovered/explored state;
- optional respawn/reset policy;
- save schema version.

The cell can unload from memory without losing authoritative state.

## 38. Ship interiors are a special case

Ships are persistent moving assemblies.

A player's ship must not behave like a building cell that is recreated every time the player enters it.

Ship state follows the same ShipId across:
- landed state;
- atmospheric flight;
- orbital flight;
- system travel;
- jump-gate travel;
- docking;
- hangar servicing;
- save/reload.

The ship interior may be streamed or represented by a local physics/interior frame for performance, but entering the player's ship should normally be visually continuous and should not replace it with a different fake ship.

## 39. Persistent ship authority

A persistent ship carries at minimum:

### Identity and provenance
- ShipId;
- blueprint/hull lineage;
- faction/manufacturer lineage;
- construction history;
- ownership;
- registry/callsign where applicable.

### Physical assembly
- BuildElements/modules;
- transforms;
- structural graph;
- detached/missing elements;
- doors/ramps/landing gear;
- hangars/docked craft.

### Engineering state
- power;
- fuel;
- coolant/heat;
- data/control;
- ammunition;
- life support;
- shield state;
- reactor/generator state;
- battery/capacitor state.

### Damage and maintenance
- armor/hull health;
- component condition;
- leaks/breaches;
- disabled systems;
- repair state;
- cosmetic damage;
- paint/livery/decals.

### Logistics
- cargo;
- containers;
- salvage;
- drones;
- vehicles;
- carried ships;
- crew/passengers.

### Runtime motion state
- current frame;
- position/orientation;
- velocity/angular velocity where applicable;
- landed/docked/flight/travel state.

Travel between world layers changes the ship's frame and simulation representation, not its identity.

## 40. Planetary landing authority

Atmosphere-capable ships may enter atmosphere, fly around the open world and land where legal/physically possible.

Landing validation may consider:
- hull/ship class capability;
- atmospheric capability;
- gravity;
- terrain slope;
- footprint;
- landing gear/contact geometry;
- obstruction clearance;
- surface material;
- environmental hazard;
- local law/landing restrictions;
- ship mass;
- water/amphibious capability if supported.

Not every ship must be capable of planetary landing.

This creates meaningful roles for:
- shuttles;
- dropships;
- atmospheric fighters;
- exploration craft;
- rovers;
- hover bikes;
- mechs;
- carriers remaining in orbit.

## 41. Surface traversal authority

Large planetary worlds support:
- on-foot exploration;
- R.I.G./powered-suit traversal;
- hover bikes;
- rovers;
- utility/mining vehicles;
- mechs;
- atmospheric craft;
- atmosphere-capable spacecraft.

The surface is a continuous logical world divided internally into streamable regions/chunks.

Strategic planet hexes may govern:
- ownership;
- security;
- resources;
- infrastructure;
- faction influence;
- mission generation;
- taxation/rights;
- conflict.

The player should see continuous terrain, not visible Civ-style hex tiles unless a tactical overlay is enabled.

## 42. Three flight/travel regimes plus docking

### Regime A — Local flight

Used for:
- dogfighting;
- docking approaches;
- asteroid navigation;
- atmospheric flight;
- surface approach;
- orbital maneuvering;
- formation flying.

Characteristics:
- highest local physical fidelity;
- thruster/inertia authority;
- collision;
- weapons;
- environmental interaction.

### Regime B — In-System Travel Drive

Used to cross meaningful distances inside one solar system without menu teleportation.

Requirements:
- same persistent ship;
- no scene replacement;
- acceleration/spool and deceleration/exit phases;
- visible high-speed travel presentation;
- destination guidance;
- gravity-well / proximity restrictions;
- safe disengagement;
- interdiction/disruption hooks;
- route hazards/anomalies;
- possibility of emergency drop-out;
- fuel/energy/heat consequences according to final balance;
- background streaming of destination operational data.

The exact player-facing name should be decided later. Avoid coupling implementation to a temporary label such as “quantum,” “supercruise,” or “pulse.”

### Regime C — Interstellar Jump Gate

Normal interstellar travel occurs through a jump gate.

Sequence:
1. navigate to gate;
2. request/obtain access if required;
3. align/enter capture corridor;
4. gate validates destination and ship restrictions;
5. gate transition begins;
6. player enters a visible warp/jump tunnel;
7. destination system is streamed/prepared during transit;
8. persistent ShipId is reparented from source SystemFrame to destination SystemFrame;
9. ship exits through destination gate;
10. normal system flight resumes.

The warp tunnel is diegetic travel, not a conventional loading screen.

Travel duration can be used for:
- visual spectacle;
- destination streaming;
- network handoff;
- save/checkpoint;
- crew/ship status;
- limited UI interaction;
- lore/communications where appropriate.

### Regime D — Docking / Hangar Transition

Docking is the primary intentional world-loading transition.

The player physically approaches and enters:
- hangar;
- drydock;
- docking berth;
- carrier bay;
- service bay.

The transition allows the engine to change from exterior-space fidelity to detailed docked/hangar/interior presentation.

The hangar contains the player's actual persistent ship.

## 43. In-game Shipyard authority

The in-game Shipyard operates on the persistent ship currently being serviced.

Typical flow:

`dock -> hangar -> inspect ship -> enter Shipyard mode -> modify actual assembly -> validate -> commit -> exit Shipyard -> board same ship -> undock`

Shipyard functions may include:
- repair;
- module installation/removal;
- structural modification;
- parametric hull work;
- socket/hardpoint changes where permitted;
- paint/livery/decal work;
- cargo/hangar configuration;
- drone/vehicle management;
- power/resource-network routing;
- fitting;
- validation;
- manufacturing/refit requirements.

The developer Ember Shipyard and the in-game Shipyard consume the same canonical assembly schemas and validators. Ember can edit any project-authorized blueprint/content; the in-game Shipyard is constrained by ownership, facilities, skills, resources, legal restrictions and progression.

## 44. Travel streaming contract

Streaming is organized around predicted travel intent.

### Planet approach
- celestial representation;
- high-altitude terrain shells;
- regional terrain;
- local terrain;
- structures/vegetation;
- local entities/interiors.

### In-system travel
- maintain origin at reduced fidelity;
- prefetch destination operational bubble;
- progressively promote relevant destination entities;
- retain travel encounter hooks.

### Jump gate
- snapshot source state;
- prefetch destination system;
- prepare gate-exit bubble;
- reparent persistent ship;
- activate destination operational simulation;
- discard source high-fidelity data only after handoff is certified.

### Interior cell
- retain exterior parent in reduced form;
- load cell package;
- bind stable entrance;
- save cell deltas on transition/checkpoint;
- restore exact exterior anchor on exit.

## 45. No-fake-ship rule

The following is forbidden as the long-term architecture:

`planet ship actor -> unload -> create unrelated space ship actor`

or

`flying ship -> dock -> create decorative hangar copy`

Instead one persistent authority record owns every representation.

Different render/physics proxies may exist, but they reference the same ShipId and canonical assembly/state.

## 46. Reference Solar System certification additions

The Reference Solar System must eventually prove:

- persistent ship lands on a large planet;
- player exits and explores on foot;
- hover bike/rover/mech deployment works;
- player re-enters the same ship;
- ship takes off without a loading scene;
- ship reaches orbit;
- ship uses in-system travel drive to another major destination;
- player enters at least one building interior cell;
- player descends via elevator/shaft into at least one underground POI cell;
- cell state persists after leaving/re-entering;
- ship docks into a station hangar;
- same persistent ship is edited using in-game Shipyard;
- edited ship undocks and flies;
- ship enters an interstellar jump gate;
- visible warp tunnel masks/prepares the system transition;
- same ShipId exits in another solar system;
- save/reload preserves all of the above.

This is now part of the core vertical-slice acceptance criteria.


---

# R4 Addendum — Planetary Settlements, Exploration Economy and Subspace Identity

## 47. Identity rule

Codename Subspace may use Elite Dangerous, No Man's Sky, Star Citizen, X4, Avorion, Starship EVO, C-Beams and Factorio as reference pillars, but no external game's terminology, progression structure or vendor model becomes the Subspace identity.

The Subspace identity is:

> A persistent embodied space sandbox where information, geography, logistics, industry, territorial control, exploration and physical ships all belong to the same simulation.

Exploration is therefore not a side activity that ends when the player sells a scan at a generic kiosk. It creates persistent knowledge with economic, scientific, industrial, political and military value.

## 48. Planetary settlement authority

Settlements exist on large continuous planetary surfaces and range from tiny remote installations to major cities.

Suggested settlement scale bands:
- Field Site
- Camp
- Outpost
- Hamlet
- Settlement
- Town
- Industrial Complex
- City
- Major City / Regional Capital

These are simulation scales, not merely visual labels.

Every settlement has a persistent identity and can own:
- controlling faction;
- population;
- security;
- law state;
- economy/specialization;
- power;
- water/life support where relevant;
- storage;
- imports/exports;
- local production;
- landing infrastructure;
- roads;
- surface vehicles;
- defensive systems;
- communications;
- mission generation;
- services;
- nearby resource rights;
- territorial hex influence;
- relationships to surrounding settlements/stations.

## 49. Settlement specializations

Possible primary/secondary roles include:
- Agriculture
- Mining
- Refining
- Manufacturing
- Research
- Military
- Logistics
- Trade
- Tourism
- Colonial
- Salvage
- Ship Services
- Energy
- Terraforming / Environmental
- Archaeology
- Prison / Detention
- Smuggling / Pirate
- Religious / Cultural
- Corporate
- Government / Civic

A settlement can combine roles. Its physical layout, NPC jobs, traffic, markets, missions and defenses should derive from those roles.

## 50. Settlement state machine

Settlements can change state persistently:

- Prosperous
- Stable
- Growing
- Shortage
- Blockaded
- Unrest
- Strike
- Quarantine
- Damaged
- Power Failure
- Under Attack
- Occupied
- Abandoned
- Ruined
- Rebuilding
- Contested
- War Zone
- Evacuating
- Newly Founded

State influences:
- population;
- NPC presence;
- services;
- prices;
- security;
- mission types;
- available cargo;
- construction;
- defenses;
- faction control;
- traffic;
- local POIs.

Player/faction actions can move settlements between states.

## 51. Settlement encounter continuity

The same settlement can be experienced at multiple scales:

Strategic:
- population;
- owner;
- production;
- security;
- territory.

Regional:
- roads;
- trade routes;
- patrols;
- nearby mines/farms;
- supply traffic.

Local:
- streamed physical buildings;
- NPCs;
- vehicles;
- defenses;
- landing pads;
- external equipment.

Interior:
- selected buildings may load detailed interior cells through doors/vestibules/airlocks.

The settlement is still one persistent SettlementId across every representation.

## 52. Planetary exploration pillars

Planetary exploration should include:

### Geography
- continents;
- islands;
- mountain systems;
- valleys;
- canyons;
- deserts;
- forests;
- tundra;
- plains;
- wetlands;
- coastlines;
- oceans/lakes/rivers;
- volcanic/geothermal terrain;
- unusual planetary formations.

### Geology/resources
- ore deposits;
- rare minerals;
- gas vents;
- geothermal sites;
- crystals;
- salvage fields;
- subsurface deposits;
- cave systems.

### Biology/ecology where supported
- flora;
- fauna;
- microbial/biological signatures;
- ecosystem relationships;
- rare species;
- dangerous species;
- harvestable biological resources.

### Civilization
- settlements;
- ruins;
- crash sites;
- abandoned infrastructure;
- roads;
- pipelines;
- power grids;
- hidden bases;
- archaeological locations.

### Anomalies
- unusual energy;
- unknown structures;
- wormhole-related effects;
- alien/unknown faction activity;
- rare planetary phenomena.

## 53. Exploration information is a commodity

The canonical exploration result is a **Survey Record**, not merely a credit payout.

A Survey Record can contain:
- location;
- discovery timestamp;
- discoverer;
- confidence/quality;
- sensor package used;
- celestial classification;
- terrain maps;
- atmosphere;
- weather/climate;
- geology;
- resource estimates;
- biological findings;
- settlement/faction data;
- route hazards;
- anomaly signatures;
- ruins/artifacts;
- landing suitability;
- strategic observations;
- ownership/licensing state.

Survey Records can be incrementally improved.

Example quality ladder:
1. Detected
2. Identified
3. Scanned
4. Mapped
5. Surveyed
6. Ground Verified
7. Scientific/Industrial Grade

## 54. Exploration monetization

Players can earn from exploration through multiple channels rather than one universal vendor.

Potential buyers:
- scientific institutes;
- cartographic/survey organizations;
- mining corporations;
- industrial corporations;
- factions/governments;
- military intelligence;
- traders/logistics companies;
- colonization groups;
- archaeology organizations;
- private collectors;
- black markets;
- mission clients.

Survey value factors can include:
- first verified discovery;
- novelty;
- completeness;
- accuracy/confidence;
- distance from known space;
- danger;
- rarity;
- biological/geological importance;
- strategic importance;
- resource potential;
- settlement/colonization potential;
- route utility;
- buyer demand;
- freshness;
- exclusivity.

The player may choose to:
- sell broadly;
- sell exclusively;
- license data;
- keep it private;
- share with own corporation;
- give it to a faction;
- auction rare data;
- use it personally for mining/colonization/war.

This makes information strategically meaningful.

## 55. Discovery provenance

Persistent firsts may be recorded for:
- first detection;
- first detailed scan;
- first orbital map;
- first landing;
- first ground survey;
- first biological sample;
- first geological assay;
- first ruin/anomaly verification;
- first safe route;
- first settlement contact.

Not every first needs a permanent galaxy-wide nameplate. Provenance should be configurable and appropriate to the discovery.

## 56. Exploration profession loop

A full exploration career can be:

prepare ship/vehicle/suit
→ choose frontier/risk region
→ travel
→ system scan
→ identify candidate worlds/anomalies
→ orbital survey
→ enter atmosphere
→ land
→ deploy on foot/vehicle
→ sample/map/verify
→ investigate POIs
→ survive hazards
→ store Survey Records/samples/artifacts
→ return/transmit when possible
→ choose buyer/licensing strategy
→ earn credits/reputation/research/territorial advantage
→ upgrade exploration capability
→ push farther into frontier space.

Exploration therefore advances:
- money;
- reputation;
- science/research;
- faction knowledge;
- resource claims;
- colonization;
- strategic maps;
- manufacturing inputs;
- story/discovery.

## 57. Settlement + exploration coupling

Exploration can create future civilization.

A surveyed region may become:
discovered resource field
→ temporary expedition camp
→ mining outpost
→ road/power/logistics connection
→ settlement
→ town
→ industrial region.

Conversely:
war / depletion / disaster / faction collapse
may produce:
town
→ damaged settlement
→ abandoned site
→ ruin/salvage POI.

This gives the universe history.

## 58. Frontier economics

Distance matters.

Remote settlements should face:
- expensive imports;
- limited repair/refuel;
- shortages;
- stronger dependence on local extraction;
- fewer replacement parts;
- more valuable exploration/survey contracts;
- higher logistics rewards;
- greater piracy/security risk.

Well-connected core settlements have:
- better services;
- denser traffic;
- lower commodity variance;
- stronger security;
- more mature infrastructure.

The economy therefore reflects the world scale rather than ignoring travel time.

## 59. Exploration equipment progression

Equipment families may include:
- system scanner;
- orbital mapper;
- atmospheric sensor suite;
- geological scanner;
- biological scanner/sampler;
- anomaly detector;
- ground radar;
- deep-resource scanner;
- rover survey suite;
- drone survey package;
- deployable beacons;
- sample storage;
- hazardous-environment tools.

Higher capability should improve precision, depth, range and data quality rather than simply revealing every POI instantly.

## 60. Exploration risks

Exploration must retain uncertainty:
- storms;
- extreme heat/cold;
- radiation;
- pressure/atmospheric hazards;
- terrain;
- fuel;
- mechanical failure;
- dangerous fauna;
- hostile settlements;
- pirates;
- unknown factions;
- anomalies;
- communication blackouts;
- navigation error;
- remote recovery/rescue.

Risk contributes to data value and expedition planning.

## 61. Subspace's unique synthesis

The intended distinction is not:
“Elite Dangerous settlements plus No Man's Sky planets.”

It is:

> The planet, settlements, ships, markets, factions, survey data, roads, resources, territorial hexes and player discoveries all participate in one persistent simulation.

A mining discovery can change market behavior.
A road can make a settlement viable.
A settlement can become strategically important.
A faction can fight for it.
A player can discover a hidden route around it.
A corporation can buy that survey data.
A refinery can be built there.
A city may later grow around the same resource region.

That systemic continuity is a defining Codename Subspace identity.


---

# R5 Addendum — ForgePY Patch Workflow Authority

## 62. ForgePY is the normal patch intake/build-control front end

Codename Subspace remains independently buildable, testable, runnable and certifiable through its project-local CLI/PCC spine. The normal development workflow uses the standalone ForgePY application as the universal front end until Rust Forge is fully certified and explicitly approved for takeover. ForgePY does not become a Subspace runtime dependency and must not be vendored into the Subspace repository.

## 63. Normal Subspace development deliverable

The default development handoff is a small incremental overwrite-capable `.patch` package from the immediately previous accepted pass.

Each package must identify Codename Subspace unambiguously, identify its required baseline/lineage, include an explicit manifest and hashes, declare writes/replacements/removals, include timestamp and compatibility metadata, be safe for ForgePY intake, avoid manual extraction, and avoid repository-root clutter.

Cumulative patches and full source rollups are reserved for recovery, major milestone consolidation, baseline handoff, cross-chat/source continuity emergencies, or explicit request.

## 64. ForgePY intake flow

`receive .patch`
→ `ForgePY intake`
→ `identify target registered project`
→ `validate container/project/baseline/lineage/date`
→ `classify into project Vault patch intake`
→ `queue for project Full Gate`
→ `transactional backup/snapshot`
→ `apply writes/removals`
→ `verify hashes/results`
→ `run project-owned build/tests/validation`
→ `certify or rollback`
→ `archive applied/rejected patch with evidence`

A compatible Subspace patch should route to Subspace even if another project is currently selected in ForgePY, subject to user confirmation and ForgePY safety policy.

## 65. Ownership split

Subspace owns source changes, project-specific schemas, validators, migrations, tests, runtime smoke tests, acceptance criteria and project-local CLI/PCC commands.

ForgePY owns intake, routing, package integrity, lineage/history, transactional apply, rollback/recovery, archival, operation logs, Full Gate orchestration and universal Git/artifact workflow.

## 66. No ZIP fallback masquerading as patch workflow

A `.patch` package must be recognized and processed as a patch package. The normal Subspace workflow must not require renaming `.patch` to `.zip`, manual extraction, a separate ad hoc PowerShell installer, or hand-copying payload files.

## 67. Patch metadata

Recommended required metadata:
- package format version;
- ProjectId;
- PatchId;
- parent/required baseline;
- source Git commit when known;
- target branch when relevant;
- UTC timestamp;
- author/tool;
- pass or milestone identifier;
- incremental/cumulative classification;
- manifest hash;
- payload hashes;
- removals list;
- compatibility contract version;
- required project schema version;
- optional minimum ForgePY contract version;
- expected gate profile.

No patch may depend on a fixed local checkout path.

## 68. Patch result evidence

A successful ForgePY-applied Subspace patch should leave evidence linking PatchId, source/baseline SHA, resulting source SHA when committed, Full Gate ID, build/test result, runtime smoke result where required, applied manifest, rollback snapshot/recovery record, archived patch location and operation log/debug bundle.

This evidence becomes part of project status/maturity truth.

## 69. Immediate recommended patch sequence

### SUBSPACE-ALIGN-01
Project vision, architecture authority, roadmap, generated/evidence-backed status framework, ADR index and supersession markers for conflicting authority documents.

### SUBSPACE-ALIGN-02
Nullharbor integration authority, world continuity/travel authority, planetary settlement/exploration authority, Reference Solar System terminology and persistent ship/travel/docking locks.

### SUBSPACE-ALIGN-03
Ember hosted-project contract scaffold, capability/descriptor scaffold, schema/version reporting and project-owned validate/run/debug exposure.

### SUBSPACE-FOUNDATION-01
Spatial-frame/persistence groundwork, stable IDs, transition/handoff contracts, cell identity and persistent ship identity.

Each patch is incremental from the immediately previous accepted/green state.

## 70. Full Gate relationship

Applying a patch is not equivalent to accepting it. A Subspace patch is accepted only after the configured ForgePY Full Gate succeeds against the post-apply tree.

On failure, ForgePY must preserve evidence and a recovery path, and classify/archive the failed patch instead of silently treating it as complete.

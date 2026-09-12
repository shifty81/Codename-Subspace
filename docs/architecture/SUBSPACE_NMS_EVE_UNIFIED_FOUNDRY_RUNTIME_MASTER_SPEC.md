# Codename Subspace — Unified No Man's Sky × EVE Foundry / Runtime Authority

Status: architecture authority and implementation roadmap. This document does **not** claim that future-facing systems are already runtime-certified.

## Game identity

Codename Subspace combines No Man's Sky-like seamless embodied exploration, traversable ship/freighter interiors, vehicles, bases and construction with EVE-like ship classes, fitting, economy, corporations, industry, logistics and fleet command. Nullharbor's mature cockpit/interior, salvage, station and planetary-installation design work is folded into Subspace's native C++ runtime rather than maintained as a parallel game.

The same asset must survive every scale: strategic asset -> local exterior -> traversable interior -> player interaction.

## Generation authority

DOMAIN -> CLASS/ARCHETYPE -> ROLE/FUNCTION -> FACTION -> HULL/STYLE FAMILY -> GENERATION DOCTRINE -> FIT/FUNCTIONAL REQUIREMENTS -> COMPONENT SIZE PROFILE -> SEED -> FUNCTIONAL GRAPH -> SPATIAL SOLVER -> KITBASH ASSEMBLY -> INTERIOR/SYSTEMS -> VALIDATE -> EDIT -> PROMOTE AS EXEMPLAR.

Class is primary identity. XS/S/M/L/XL are component tiers only. Role changes topology, spatial requirements, module selection, fitting, interior functions, operational stations and visual language. Faction doctrine changes how a faction solves a role without changing the role itself.

## Universal Foundry

One Foundry Core serves Shipyard, Station Foundry, Vehicle Foundry, Planetary Installation Foundry, Interior Foundry, Weapon Foundry and Character/Armor Foundry. Shared authority covers canonical kitbash assets, source provenance/licenses, sockets/portals, size compatibility, materials, transform/symmetry, PCG, spatial certification, validation and exemplar lineage.

Only traversable assemblies opt into SpatialAssembly; weapons and ordinary armor do not carry room/pressure systems unnecessarily.

## Whole-assembly spatial authority

Final kitbash placement becomes one derived whole-assembly spatial field:
1. transform canonical module spatial proxies into assembly-local coordinates;
2. union solids/exterior fields;
3. seal only certified mating sockets;
4. cap intended portals during enclosed-volume classification;
5. outside flood fill;
6. derive exterior shell;
7. derive inner hull using armor/service thickness;
8. intersect hollowable/interior-capable regions;
9. subtract structural/machinery/propulsion/reactor/weapon/landing-gear keepouts;
10. produce SafeInteriorVolume;
11. derive human/armor/cargo/vehicle clearance fields;
12. solve rooms/corridors/decks;
13. dress interiors only after topology validates.

Outer hull, safe interior, collision, pressure boundaries, shield envelope and localized damage rebake should ultimately derive from this same assembly field.

## Seamless interiors and frames

Use hierarchical spatial frames: Universe/System -> Planet/Station/Ship -> Vehicle -> Character. Large roots use double precision; local gameplay uses stable local coordinates. Frame crossing preserves world position and inherited linear/angular velocity.

Doors, ramps, lifts and landing gear are normally kinematic children of the parent assembly instead of separate unstable physics subgrids.

## Interior graph

Compartments are nodes. Doors, airlocks, hangar doors and breaches are edges. The same graph feeds pressure/environment, navigation, rendering visibility, streaming, audio propagation, security/access and network interest. Door state changes graph connectivity; true structural damage triggers localized spatial rebake.

## Cockpits and operational cores

One Operational Core supports domain-specific presentation:
- Ship -> cockpit / bridge / CIC
- Vehicle -> cockpit / cabin / driver-gunner positions
- Station -> operations / traffic / security / industry
- Planetary installation -> control room / power / logistics / refining

Physical seats, consoles and control authority are generated blueprint members.

## Physical fitting

Logical fit maps to physical systems. Shield generators, reactors, mining equipment, cargo infrastructure and hangars require physical placements/volumes. Destroying the physical system degrades the logical capability.

## Vehicle and hangar access

Vehicle profiles include width, height, length, wheelbase, turning radius, ground clearance, maximum ramp angle and mass class. A bay is invalid unless the complete path passes from outside staging through aperture/ramp/lift to ingress, turning area and parking/tie-down.

## Kitbash source strategy

Priority governed sources include the existing certified Greyoxide corpus; Quaternius Modular Sci-Fi MegaKit, Sci-Fi Essentials, Universal Base Characters/UAL and Sci-Fi Modular Guns; Kenney Modular Space/Space Station/Factory/Building/Industrial kits; KayKit Space Base Bits; Molten Maps SciFi; Poly Haven and ambientCG materials.

Every acquisition records provider, pack, revision, acquisition date, URL, source hash, exact license evidence and derived lineage. Provider-wide license assumptions are forbidden; Quaternius is explicitly version/acquisition sensitive.

## Blender parity

Refactor SubspaceShipyard toward SubspaceFoundry. Blender is a client of the same native generation authority, never an independent authoritative PCG implementation. Blender parity includes semantic asset catalogs, sockets, portals, seals, cuttable/keepout volumes, clearances, seats/cameras/consoles, collision proxies, animation metadata, LOD/HLOD previews and exact blueprint V2 round-trip. GLB transports geometry/animation; Subspace blueprint JSON remains gameplay authority.

## Rendering modernization

Legacy WGL/OpenGL remains the compatibility lane until a replacement is certified. Target a Subspace-owned RenderDevice/RenderScene/RenderGraph boundary with indexed GPU meshes, shared resources, instancing, assembly HLOD, PBR/HDR, clustered Forward+, shadows, transparent VFX, portal/HZB culling, KTX2/Basis textures, meshoptimizer offline processing, projected decals, GPU thrusters and one whole-assembly shield envelope. Diligent + selected DiligentFX remains the leading bootstrap candidate behind Subspace-owned interfaces.

## Runtime scale and persistence

Simulation, rendering, residency and network interest are separate fidelity authorities. Distant ships remain aggregate strategic objects; nearby ships hydrate exteriors; boarded ships hydrate full interiors. Deterministic generated baseline + state deltas prevents thousands of unchanged procedural assemblies from bloating saves.

## Completion truth

Only CERTIFIED_RUNTIME means complete. APIs, source markers, compilation and isolated unit tests alone do not qualify.

Known major open items after the Pass791-890 foundation tranche remain: exact live ShipyardBuilder class/size bridge removal; whole-assembly SDF bake; full pressure/nav/audio/streaming integration; nested moving-frame embodiment certification; Home System persistence V2; runtime formation following; modern render backend; Blender native-generator parity; bulk kitbash intake; and final NMS×EVE Frigate vertical-slice certification.

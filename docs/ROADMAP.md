# Codename Subspace Roadmap

## Phase 0 — Authority and standalone PCC normalization

**State: active normalization.**

- one project vision/architecture/roadmap authority;
- explicitly supersede 2D/no-landing, rail-travel and mainline-roguelite authorities;
- internal PCC root `.patch` prompt, transactional apply, Full Gate on option 1, guarded GREEN commit/push on option 2;
- `GameData/` / `content/` authority split;
- evidence-backed maturity registry and build-graph inventory;
- Ember hosted-project contract aligned to Reference Solar System and internal PCC.

## Phase 1 — Runtime composition, spatial frames and persistence

Before broad world expansion:

- create one runtime/world-simulation composition authority instead of adding more independent state to `Engine`, `NativeRuntimeServices`, and `NativeGameApplication`;
- stable entity/ShipId/CellId/SettlementId identity;
- rotation-aware nested spatial-frame reparenting;
- local physics-space handoffs;
- save schema versions/migrations and complete round-trip persistence;
- simulation LOD representation handoffs;
- destination-prefetch and interior-cell persistence contracts.

## Phase 2 — Shipyard/construction foundation

- universal BuildElement/Assembly/SubAssembly model;
- lock quantized construction unit via ADR;
- parametric structural elements + authored kitbash modules;
- semantic sockets/attachment faces and generated structural connectors;
- local face grids, symmetry, nested subassemblies;
- serializable BuildCommand undo/redo/replication contract;
- editable canonical assembly -> validated/compiled render/collision/nav/atmosphere products;
- wire visible native editor/Shipyard tabs, splitters, drag/drop, floating/redocking and hit-testing to `EditorDockSystem`.

## Phase 3 — Physical ship and embodiment

- full 3D flight and local physics;
- mass/COM/inertia and propulsion authority;
- power/fuel/coolant/data/ammo/cargo/life-support networks;
- interiors/portals/clearance and atmosphere;
- docking/hangar/on-foot/EVA transitions;
- intact-module salvage/repair/refit;
- conformal one-foot shield shell and impact ripples;
- carrier/hangar/drones/vehicles.

## Phase 4 — Reference Solar System

The first complete end-to-end proving environment:

- spherical multi-resolution planet streaming;
- seamless orbit -> atmosphere -> surface -> takeoff;
- large open surface world with foot/hover-bike/rover/mech/atmospheric traversal;
- persistent settlements/cities, roads, resources, territory/security;
- persistent loaded building/underground cells;
- exploration Survey Records/information economy;
- system drive tuned to meaningful minute-scale travel;
- physical jump gate + destination-ready warp tunnel to a second system;
- hangar Shipyard operating on the same persistent ship;
- market/refining/manufacturing/repair/refuel loop;
- save/reload and performance-budget certification.

## Phase 5 — Dynamic economy, factions, fleets and territory

- NPC extraction/production/hauling/consumption/loss replacement;
- regional markets and logistics;
- dynamic faction goals/budgets/security/claims/conflict;
- settlements/stations as infrastructure;
- fleet orders/formations/carriers;
- exploration discoveries feeding claims, colonization and strategic knowledge.

## Phase 6 — Procedural galaxy breadth

- system/planet/moon/settlement/POI families;
- deterministic authored-quality PCG and generation receipts;
- security gradients, pirates, hidden/unknown factions, wormholes/deep space;
- discovery provenance and survey economy at galaxy scale.

## Phase 7 — Presentation/performance

- decompose/render modernize `NativeBattlefieldRenderer`;
- compiled assembly clusters, batching/instancing/HLOD/async uploads;
- normalized PBR/material/livery/decal pipeline;
- lighting/VFX/audio/UI polish;
- large ship/station/planet performance budgets;
- rendering-backend migration only when profiling justifies it.

## Phase 8 — 2–8 player co-op

- authoritative session/server;
- commands/snapshots/deltas and interest management;
- shared build permissions and multicrew;
- reconnect/persistence/conflict resolution;
- certification against Reference Solar System.

## Phase 9 — Expansion

- broader settlement/city diversity, deeper surface ecology, advanced carriers/corporations/sovereignty/player markets and larger online scale only after the core architecture is proven.

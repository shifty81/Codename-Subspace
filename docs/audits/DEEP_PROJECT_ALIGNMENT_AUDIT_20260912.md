# Codename Subspace Deep Project Alignment Audit

## Executive assessment

Codename Subspace should **not be restarted**. The current native C++ tree contains substantial working foundations, a strong project-control/gate spine, broad Shipyard/kitbash work, and enough simulation scaffolding to continue forward. The primary risk is not lack of code; it is **authority fragmentation**: old Avorion-like data, retired 2D/no-landing and rail/roguelite direction, multiple runtime state owners, detached historical source lanes, and many small “System” classes that can be mistaken for player-complete features.

The correct project identity is the current one: an embodied persistent space sandbox/strategy/RPG in which spaceflight, seamless planet traversal, settlements, exploration information, economy, factions, physical ships, interiors, vehicles, industry and fleet/corporate command participate in one simulation.^1 Full 3D hierarchical world/reference frames and continuous logical planets are authoritative.^2

### Audit verdict

**KEEP the repository and current native runtime. REWRITE authority boundaries. RETIRE competing direction. BUILD depth before breadth.**

The highest-value next implementation work is not another hundred disconnected gameplay classes. It is:

1. converge runtime ownership;
2. finish spatial/persistence identity and handoffs;
3. finish the canonical construction compiler/model;
4. make the visible editor consume the canonical dock model;
5. build the Reference Solar System end-to-end;
6. only then scale economy/factions/PCG/co-op breadth.

## Audited baseline and evidence

The audit baseline is the current standalone-PCC R6R2 source plus Pass892–901 dock normalization. The prior public GitHub baseline is `1f714de6ea5c86ae45d38e01cd6167ca3dc6e5d3`; newer local work is deliberately treated as **uncertified** until the Windows internal-PCC Full Gate passes.

Portable validation after this alignment pass:

- complete configured C++ build: **PASS**;
- registered CTest targets: **82/82 PASS** after resolving authority-gate drift;
- Windows/MSVC internal-PCC Full Gate: **still required for GREEN**.

This distinction is intentional: compilation and unit/source gates are evidence, but only the project Full Gate may promote the current Windows source line to GREEN.

## 1. Project identity is now coherent

The active vision already defines Subspace as an embodied persistent sandbox/strategy/RPG with systemic continuity.^1 The audit found that the strongest current documents agree on:

- full 3D hierarchical simulation;
- continuous planetary surfaces;
- seamless normal space -> atmosphere -> surface travel;
- persistent ships and stable IDs;
- diegetic loaded cells for dense interiors/underground spaces;
- hangar/docking transition authority;
- in-system travel measured in meaningful minutes;
- interstellar jump gates with a warp-tunnel loading mask;
- large planetary settlements and exploration economy;
- internal PCC current operations authority;
- Ember as a future external authoring host, not runtime dependency.

The main alignment defect was that older documents still competed with this authority. The August sandbox document explicitly declared 2D X/Y flight and no conventional planetary landing; the rail-travel document declared on-rails interstellar routes; the roguelite direction made a persistent “home system + expedition runs” structure the game identity. These files are now explicitly marked **SUPERSEDED**, **HISTORICAL**, or **OPTIONAL CAMPAIGN/REFERENCE** instead of being allowed to masquerade as current architecture.

## 2. Build-graph truth: not every source file is part of the game

The configured engine source graph contains 62 `engine/src` subsystem directories. Of 424 subsystem `.cpp` files, 382 are in the current compile graph and 42 are detached. The detached implementation lanes are concentrated rather than random:

| Detached lane | `.cpp` files | Audit disposition |
| --- | ---: | --- |
| `client/` | 4 | alternate historical client lane; do not displace `NativeGameApplication` accidentally |
| `home/` | 24 | old home/roguelite architecture; reference/optional-campaign concepts only |
| `expedition/` | 4 | old run-loop implementation; reference only unless deliberately ported |
| `roguelite/` | 3 | old mainline identity; optional scenario concepts only |
| `travel/` | 4 | old interstellar rail implementation; superseded |
| `migration/` | 3 | migration/reference implementation, not production runtime |

Four additional old test `.cpp` files are not registered, producing the previously observed 46 total uncompiled `.cpp` files when tests are included.

**Critical rule:** a future “CMake completeness” pass must not simply glob these directories back into the build. Each useful behavior needs a deliberate port into current authority.

The full per-subsystem matrix is in `BUILD_GRAPH_AUTHORITY_MATRIX_20260912_R1.csv`.

## 3. Runtime composition is the largest structural gap

`Engine::RegisterSystems()` registers the basic ECS lane (control, physics, combat, navigation, power, AI, mining, quest/tutorial, particles, achievements, UI).^3 Separately, `NativeRuntimeServices` owns economy, stations, factions, fleet missions, interiors, persistence, combat extensions, repair, EW, drones, factions, sector simulation, exploration, hazards, logistics, population, carriers, corporations and more.^4 `NativeGameApplication` then owns another large set of player-facing stateful systems: maps, system navigation, vector travel, surveys, industrialization, docking, embodiment, presentation, orbital dynamics, strategic flight, station ecology, fleet AI, Shipyard and editor camera.^5

That means there is no single obvious object that answers: **“who owns authoritative world state?”**

### Recommendation

Create a `SubspaceRuntimeComposition` / `WorldSimulationRuntime` authority in Phase 1. It should own canonical world-domain state and expose bounded adapters to:

- `Engine` lifecycle/ECS;
- native application/window/input/presentation shell;
- headless/server session;
- tests and launch-at-context;
- later Ember/Cortex tooling.

Do not add new independent stateful gameplay systems directly to `NativeGameApplication` while this convergence is pending.

## 4. Spatial frames are the right foundation but not finished

`SpatialFrameSystem` already uses double-precision position/quaternion data and transforms points/velocities through parent frames. This is the correct direction for galaxy/system/planet/ship/interior/local-physics hierarchy. However, `ReparentPreservingWorld` explicitly uses a translation-only inverse when reparenting into a non-world frame and states that rotation-aware inverse is still required before nested moving vehicles ship.^6

This blocks robust versions of exactly the features now central to Subspace:

- player walking inside a moving/rotating ship;
- carrier -> hangar -> carried craft frames;
- atmospheric ship -> landed surface frame;
- rover/mech deployment;
- jump-gate system-frame transfer;
- seamless representation/physics-LOD changes.

Finish this before large-world breadth.

## 5. Travel: keep VectorTravel, retire rail, build jump gates

The compiled `VectorTravelSystem` has `Idle -> Aligning -> Charging -> Cruise -> Decelerating -> Complete/Failed` state progression.^7 This is a strong foundation for the current **In-System Travel Drive**. It should be extended with destination prefetch, gravity/proximity restrictions, interdiction/disruption, emergency dropout, fuel/energy/heat consequences and the user-defined travel-time scale.

The old interstellar rail implementation is outside the build graph and its design document is now explicitly superseded. It may donate encounter-risk or fitting ideas, but it is not the transport model.

No compiled physical `JumpGate` transit authority was found. The real interstellar system therefore remains a missing runtime feature:

`approach gate -> validate destination/access -> capture/alignment -> enter tunnel -> load/prepare destination -> DestinationReady -> tunnel deceleration -> same ShipId exits destination gate`.

This should be built only after persistent identity/spatial handoffs are stable.

## 6. Planetary architecture: current code is foundation, not seamless planet runtime

The current architecture requires continuous logical planets and seamless ordinary space/atmosphere/surface traversal.^2 The playable shell now accurately reports that seamless surface landing is **current architecture but not yet runtime-wired** instead of claiming that no landing path should exist.^8

`PlanetWorldEngineSystem` contains world definition/terrain/chunk/strategic-hex foundations,^9 but its current implementation is a compact heightfield-style foundation rather than a production spherical multi-resolution world streamer. `RegionStreamingSystem` is useful deterministic streaming work, but it currently models 2D X/Y macro cells for asteroid/ring regions, not a seamless spherical planet.

### Required planet stack

1. celestial/orbital representation;
2. multi-resolution spherical terrain authority;
3. atmosphere/cloud/weather promotion;
4. streamed regional terrain/biome/geology/ecology;
5. local high-fidelity physics bubble;
6. persistent settlements/roads/infrastructure/traffic;
7. loaded building/underground cells linked by stable anchors;
8. strategic hex ownership underneath continuous visible terrain;
9. seamless takeoff back through the same hierarchy.

The user-defined scale targets—~10 minutes atmospheric pole-to-pole, rover days, walking weeks—must drive PCG spacing and streaming budgets, not be bolted on after content placement.

## 7. Settlements and exploration are design-authoritative but runtime-immature

The current project authority correctly makes planetary settlements persistent simulation nodes and exploration information a commodity. The source contains economy, faction, survey, resource and planetary-industry foundations, but no dedicated persistent `SettlementId` authority was found.

Build settlements as multi-resolution entities:

- strategic record: owner, population, security, production, storage, power/water/life support, imports/exports, influence;
- regional record: roads, mines, farms, traffic, patrols, supply links;
- local exterior: buildings, pads, defenses, NPCs, vehicles;
- selected interior cells: persistent loaded spaces behind doors/elevators/airlocks.

Likewise, exploration should converge on a versioned `SurveyRecord` schema with detection/mapping/ground-verification quality, provenance, samples, buyer/licensing policy, freshness and strategic value rather than a single scan payout.

## 8. Persistence is not ready for the world scale being designed

`HomeSystemSaveGame` is explicitly a simple line format with a “temporary compatible parser.”^10 It is also shaped around the detached old home/expedition model. It cannot be the long-term persistence authority for seamless worlds, persistent ships, settlement evolution, loaded cells, exploration records and multiplayer.

Phase 1 persistence must include:

- schema versions and migrations;
- stable IDs for ship/cell/settlement/world entities;
- unknown-field preservation where appropriate;
- canonical/delta state separation;
- representation-LOD independent identity;
- save/checkpoint semantics during gate transit/docking;
- deterministic PCG receipts plus mutable deltas;
- round-trip tests across version migrations.

## 9. Construction and Shipyard: broad investment, still missing one canonical compiler

The project has substantial Shipyard code, a certified Greyoxide corpus, taxonomy/classification, sockets, symmetry, authoring, thumbnails, paint/surface systems, PCG closure and validation. This is one of the strongest gameplay/tooling areas.

The architectural gap is consolidation. The target should be one canonical editable assembly:

`BuildElement / AuthoredModule -> Assembly/SubAssembly -> semantic attachments -> resource/interior graphs -> validation -> compiled mass/inertia/collision/render/nav/atmosphere/shield products`.

Large structural pieces should remain sparse parametric elements rather than exploding into tiny visible blocks. The exact construction quantum (earlier candidates 1/16 m or 1/32 m) should be locked by ADR before changing existing grid constants.

Legacy compiled module code still contains donor material and “Hyperdrive” identifiers. Those may have serialization/test compatibility consequences. This audit intentionally does **not** blind-rename those enums/IDs. Player-facing GameData has been normalized now; internal IDs need a dedicated versioned migration/alias pass.

## 10. Editor: canonical dock model exists; visible GUI still needs to use it

Pass892–901 establishes `EditorDockSystem` as a real tab/split/floating/persistence model. Its tests pass, and legacy layout projection remains compatible. The remaining problem is the one the user originally described in the editor family: visible native renderer/hit-testing still has specialized/hard-coded panel behavior.

Next editor work should make one dock tree authoritative for:

- tab bars and active tabs;
- close/reopen panel menu;
- draggable splitters;
- drag/drop docking targets;
- floating/redocking;
- scroll/resize behavior;
- selection/inspector/assets/console/history/PCG/runtime-diff panels;
- persisted layout presets.

No new workspace should create its own panel geometry authority.

## 11. Content/data authority was internally contradictory

README/project layout already treated `GameData/` as runtime-authored data, while several compiled normalization systems still classified it as legacy and planned a move to `content/data/`. This was a real source-level authority contradiction.

Pass902–911 normalizes the split:

- **`GameData/`** — canonical authored gameplay/runtime JSON and packaged runtime data;
- **`content/`** — schemas, source registries, provenance, governed metadata and governed derived/certification authority.

A future physical data-root migration is allowed only as an explicit versioned loader/schema migration—not an incidental cleanup operation.

Player-facing GameData has also been normalized away from the old donor material-tier ladder, instant sector hyperdrive, and voxel-block ship identity.

## 12. Donor identity still exists in compiled compatibility internals

Compiled `ShipModuleDefinition`, `ModularShipFactory` and `SalvageSystem` still contain Avorion-era material or hyperdrive identifiers. These are now classified as **migration debt**, not current design authority.

Safe replacement requires:

1. inventory serialized/save/content dependencies;
2. define Subspace-native resource/material/drive IDs;
3. introduce aliases/migration mapping;
4. migrate saves/blueprints/content;
5. update tests;
6. remove donor canonical IDs only after compatibility evidence passes.

This is intentionally different from merely replacing strings.

## 13. Fleet/corporation breadth exists; formation movement remains placeholder

Fleet, carrier, drone, captain, doctrine and corporation systems are present in the compiled tree, but `FormationSystem::Update()` explicitly says position updates are placeholder work.^11 This is a good example of why class counts cannot equal maturity.

Converge these systems after the runtime-composition work so strategic/LOD fleets and local physical fleets share identity/orders rather than being separate simulations.

## 14. Multiplayer is scaffold-level

`NetworkSystem` contains message serialization and in-memory server/client/sector abstractions, but it is not a production authoritative 2–8 player session stack.^12 The current decision to postpone player-facing co-op until after the Reference Solar System is correct.

However, Phase 1/2 data contracts should already favor deterministic commands, stable IDs, snapshots/deltas and interest scopes so multiplayer does not require a second data model later.

## 15. Renderer: optimize/decompose before backend replacement

The current native renderer builds and existing visual tests pass, but `NativeBattlefieldRenderer.cpp` is approximately 3,882 lines and mixes many presentation responsibilities. `ShipyardBuilderSystem.cpp` is also approximately 1,828 lines. These are maintainability and performance-risk concentration points.

Recommended progression:

1. split scene/domain passes without changing output;
2. make compiled assembly clusters/dirty regions explicit;
3. batch/instance/HLOD and async-upload hot paths;
4. normalize materials/livery/decal data;
5. establish frame-time/memory/draw-call budgets for large ships/stations/planet approach;
6. consider renderer-backend migration only when profiling demonstrates that the current abstraction/backend is the blocker.

## 16. Asset/source provenance needs to catch up to code references

The required kitbash manifest currently pins the Greyoxide Shipyard corpus with explicit source/license/revision information.^13 A broader foundry registry lists Quaternius/Kenney/KayKit/etc. as intake candidates, which is the correct status until exact acquisition/license/hash evidence is captured.

`ImportedPlanetVisualSystem` expects `content/derived/various_planets_v1`,^14 but the current required source registry does not govern that planet fixture. Before it becomes a required runtime dependency, record the exact user-supplied source, checksum, ownership/license evidence and derived lineage.

## 17. Standalone PCC stays current operational authority

The internal PCC remains the current operator path. Startup root `.patch` intake, option 1 Full Gate and option 2 guarded commit/push are the desired workflow. Forge/Cortex is deferred until explicitly certified for takeover. Ember is a future authoring host and should invoke project-owned commands rather than own build/patch/Git semantics.

The PowerShell 5.1 failures found during this chat are useful evidence: control tooling must continue to certify against the actual Windows host, not only modern PowerShell/.NET APIs.

## 18. Immediate implementation order

### Pass902–911 — this alignment normalization

- normalize docs/authority;
- align GameData/content root ownership;
- normalize player-facing donor vocabulary;
- add authority regression gate;
- publish build-graph and maturity truth;
- accurately label seamless landing/jump-gate/runtime-composition gaps.

### Next: Runtime Foundation tranche

- runtime composition root;
- stable IDs;
- spatial reparent/inverse transforms;
- save schema/migrations;
- simulation-LOD/representation handoffs;
- persistent ship/cell identity.

### Then: Construction + visible editor tranche

- BuildElement/Assembly compiler;
- quantization ADR;
- resource/interior networks;
- make visible native editor consume `EditorDockSystem`;
- persistent docked-ship Shipyard loop.

### Then: Reference Solar System tranche

- spherical planet streaming;
- atmosphere/landing/takeoff;
- surface vehicles;
- settlements/cells;
- survey economy;
- minute-scale system drive;
- jump gate to second system;
- end-to-end persistence/performance.

Only after that should broad procedural-galaxy, faction/economy scale and multiplayer become the dominant workstreams.

## 19. What this audit intentionally does not do

This pass does **not** pretend to implement seamless planets, jump gates, co-op, a modern renderer, full atmosphere simulation, settlements, or the canonical BuildElement compiler merely by documenting them. It creates accurate authority and guards against regression while preserving the compile-clean runtime.

That distinction is mandatory for Subspace: **architecture truth first, implementation evidence second, certification last.**

## Sources

1. `docs/PROJECT_VISION.md`, line 3 and subsequent identity/world sections — current Subspace identity, continuity and scale authority.
2. `docs/ARCHITECTURE_AUTHORITY.md`, lines 3–14 — full 3D hierarchy, continuous planets, stable IDs, construction, exploration, PCC/Ember and maturity rules.
3. `engine/src/core/Engine.cpp`, around lines 203–231 — current ECS system registration/composition.
4. `engine/include/runtime/NativeRuntimeServices.h`, around lines 42–90 — second stateful runtime service container and owned systems.
5. `engine/include/application/NativeGameApplication.h`, around lines 53–190 — player-facing application-owned systems/state.
6. `engine/src/world/SpatialFrameSystem.cpp`, around lines 41–50 — current reparent implementation and rotation-aware follow-up note.
7. `engine/include/navigation/VectorTravelSystem.h`, lines 9–31; `engine/src/navigation/VectorTravelSystem.cpp` — compiled in-system travel state machine.
8. `engine/src/application/NativeGameApplication.cpp`, line 1451 after Pass902–911 normalization — honest seamless-landing implementation-gap message.
9. `engine/include/world/PlanetWorldEngineSystem.h`, around lines 43–157; `engine/src/world/PlanetWorldEngineSystem.cpp` — planet/chunk/hex foundations.
10. `engine/src/core/persistence/HomeSystemSaveGame.cpp`, around lines 18–55 — deliberately simple save format and temporary parser.
11. `engine/src/formation/FormationSystem.cpp`, around lines 327–330 — explicit placeholder runtime movement.
12. `engine/src/networking/NetworkSystem.cpp`, especially `NetworkMessage`, `ClientConnection`, `SectorServer`, `GameServer` — current networking scaffold.
13. `content/kitbash/sources.subspace_sources.json`, source `greyoxide_shipyard_v07` — required governed kitbash source/provenance.
14. `engine/src/rendering/ImportedPlanetVisualSystem.cpp`, line 16 — expected `content/derived/various_planets_v1` path.
15. `docs/audits/BUILD_GRAPH_AUTHORITY_MATRIX_20260912_R1.csv` — compile-graph inventory generated from CMake `compile_commands.json`.
16. `docs/audits/DEEP_FEATURE_MATURITY_MATRIX_20260912_R1.csv` — evidence-based 0–5 feature maturity assessment.
17. `content/architecture/project_completion_truth_v1.json` — current completion-state vocabulary and known findings.

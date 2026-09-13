# Codename Subspace — Current Architecture Locks

- **ADR-023:** Ember hosted-project contract; Subspace stays independently operable.
- **ADR-024:** Ember/runtime share canonical data authority.
- **ADR-025:** Nullharbor is integrated lineage, not a parallel game.
- **ADR-027:** Selected interiors/underground POIs use diegetic persistent cell transitions.
- **ADR-028:** Persistent ship identity survives all travel/docking/save transitions.
- **ADR-029:** In-game Shipyard edits the actual docked persistent ship.
- **ADR-030:** In-system travel drive is continuous ship travel, not menu teleportation.
- **ADR-031:** Interstellar travel uses jump gates and destination-loading warp tunnels.
- **ADR-032:** Reference Solar System replaces “Golden Home System.”
- **ADR-033:** Exploration creates persistent economically/strategically useful survey information.
- **ADR-034:** Planetary settlements are persistent simulation entities.
- **ADR-035:** Settlements can grow, decline, be damaged, occupied, abandoned and rebuilt.
- **ADR-036:** Exploration and settlement development share persistent history.
- **ADR-037:** PCG is travel-time-aware.
- **ADR-038:** Reference games are non-authoritative inspirations.
- **ADR-039R:** Until Forge/Cortex integration is explicitly promoted, project-owned PCC patch/build/gate/publish operations are authoritative.

- **ADR-040:** One runtime/world-simulation composition authority must converge state currently split across Engine ECS, NativeRuntimeServices, and NativeGameApplication.
- **ADR-041:** `GameData/` is canonical authored runtime/game data; `content/` is governed metadata/schema/provenance/derived authority. Physical relocation requires an explicit versioned migration.
- **ADR-042:** Detached `home/`, `expedition/`, `roguelite/`, `travel/`, migration, and alternate-client implementation lanes are non-authoritative unless selected behavior is deliberately ported into current architecture.
- **ADR-043:** Compiled donor material/drive identifiers are migration compatibility debt; replacements require versioned save/content aliases rather than blind renaming.

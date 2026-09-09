# Carbon / Open-Source EVE Technology Intake Matrix — 2026-08-31

## Policy

Carbon is treated as an engineering source/reference, **not** as a new engine migration target. Subspace has a certified native C++ runtime; any Carbon code must enter behind an adapter, have a measurable benefit, preserve Subspace authority, and carry complete license/provenance records. EVE game art, lore, trademarks, UI assets and game content are not reusable merely because Carbon components are open source.

No Carbon source code is imported by Pass220. The runtime matrix is implemented in `runtime/CarbonIntakeSystem` so future spikes have an explicit disposition.

## Verified public components

| Component | Current disposition | Why | Source |
|---|---|---|---|
| Trinity | REFERENCE | Rendering/material/resource/large-scene architecture is highly relevant, but its documented public backends are DX11, DX12 and Metal; replacing the current OpenGL authority now would be another migration. It also has a non-trivial Carbon/third-party dependency graph. | https://github.com/carbonengine/trinity |
| Destiny | REFERENCE | Highly relevant world-simulation/interest-management reference. Public README currently says some build dependencies still require Perforce, so it is unsuitable as a required dependency. | https://github.com/carbonengine/destiny |
| Mesh | IMPORT CANDIDATE | MIT C++ library for mesh/skeleton/animation serialization and basic runtime support; worth an isolated adapter spike for modular ships and future animation/LOD work. | https://github.com/carbonengine/mesh |
| Resources | IMPORT CANDIDATE | MIT resource-operations framework; worth measuring against Subspace's native resource authority. | https://github.com/carbonengine/resources |
| Pathfinder | ADAPT | MIT C++ route/flood-fill library built around EVE map data. Generic graph/routing concepts may help hazard/fuel/temporary-route navigation, but EVE data-specific code should not become Subspace's schema. | https://github.com/carbonengine/pathfinder |
| Localization | IMPORT CANDIDATE | Public Carbon localization framework; evaluate later behind a native localization adapter. | https://github.com/carbonengine/localization |
| Core/Math | REFERENCE | Useful low-level patterns, but Subspace already owns working native core/math authorities. | https://github.com/carbonengine/core |
| Scheduler | REJECT DIRECT USE | Carbon scheduling/Python-era assumptions should not displace Subspace's native runtime model. | Carbon organization repository |
| Blue / Python bridge | REJECT DIRECT USE | Do not reintroduce a Python/C++ gameplay runtime bridge after completing the C# -> native C++ retirement. | Carbon organization repository |
| Carbon Audio | DEFER | Large-battle audio ideas are useful, but middleware/dependency choices need a separate audio architecture decision. | Carbon organization repository |

## Legal/provenance gate

Before any IMPORT/ADAPT candidate enters production, record:

```text
upstream repository
exact commit/tag
license + NOTICE/third-party notices
files/functions adopted
local modifications
Subspace adapter boundary
security/dependency scan
build-size/runtime-performance measurements
removal/rollback plan
```

Trinity's repository explicitly notes its MIT license does not grant rights to CCP trademarks or game content and includes a NOTICE for third-party code. Apply that provenance discipline to every Carbon intake.

## Recommended spikes after Pass225

1. **Mesh spike:** import only into an isolated test target; compare load/serialization/runtime structures against current OBJ path.
2. **Pathfinder spike:** extract/imitate generic Dijkstra/flood-fill concepts against Subspace system graph with hazard/fuel constraints.
3. **Resources spike:** compare async/resource-lifetime concepts against native resource services without changing production ownership.
4. **Trinity architecture audit:** document renderer/material/LOD/large-space patterns. Do not replace OpenGL in the same pass.
5. **Destiny architecture audit:** focus on interest management, simulation partitioning and large-fleet entity processing rather than direct linkage.

## Pass220 decision

**REFERENCE/ADAPT before IMPORT. No engine rewrite.**

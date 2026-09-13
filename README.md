# Codename Subspace

**Codename Subspace** is a native C++ space sandbox, strategy game, and RPG built around one persistent physical simulation: the player can fly, walk, salvage, mine, build, explore, trade, operate vehicles, and grow into fleet, industry, settlement, and corporate command.

The authoritative runtime is **native C++ only**. Historical C#/Avorion-derived code and older design experiments are reference/migration evidence, never fallback runtime authority.

## Current identity

Subspace is not a clone or a stack of disconnected reference-game mechanics. Its defining rule is **systemic continuity**:

- one persistent ship identity survives landing, atmosphere, orbit, in-system travel, jump-gate transit, docking, refit, and save/reload;
- normal travel is seamless from system space through orbit and atmosphere to large continuous planetary surfaces;
- dense building interiors, underground shafts/mines/bunkers, and station hangars may use persistent loaded cells behind diegetic doors, elevators, airlocks, or docking transitions;
- planetary settlements, roads, resources, exploration data, factions, security, logistics, territory, markets, and industry participate in the same simulation;
- exploration creates persistent Survey Records whose information can have scientific, industrial, economic, political, and military value;
- ships and stations use authored kitbash plus parametric construction, semantic sockets, subassemblies, fitting, damage, engineering networks, and physical interiors;
- direct embodied play and higher-level fleet/corporate command are views of the same world, not separate games.

External games are reference sources only. Accepted ideas are translated into Subspace-native terminology, schemas, progression, and simulation rules.

## World and travel scale

Current design authority targets:

- ideal full-system in-system-drive crossing: roughly **2 real minutes minimum**;
- average major-planet pole-to-pole atmospheric flight: roughly **10 real minutes** under good conditions;
- rover-scale planetary traversal: days-scale;
- walking-scale planetary traversal: weeks-scale;
- interstellar travel: physical **jump gates** with a visible warp tunnel that persists until the destination is ready, then visibly decelerates before exit.

PCG must respect travel-time topology. Cities, outposts, mines, ruins, wilderness, roads, caves, resources, and other POIs are not uniformly sprinkled for convenience.

## Runtime authority

| Area | Authority |
| --- | --- |
| Game/runtime | `engine/` native C++ |
| Build/certification | CMake + project-owned Full Quality Gate |
| Current operations | internal standalone Project Control Center |
| Root launcher | `SubspaceTools.cmd` |
| Root command authority | `SubspaceTools.ps1` + `project.control.json` |
| Normal patch transport | root-drop `.patch`, applied transactionally by internal PCC |
| Runtime/game data | `GameData/` |
| Governed metadata/schemas/provenance | `content/` |
| Project tools | `tools/` and `scripts/` |
| Current design/architecture | `docs/PROJECT_VISION.md`, `docs/ARCHITECTURE_AUTHORITY.md`, `docs/ROADMAP.md`, current ADRs |
| Historical/reference design | explicitly superseded/reference documents under `docs/` |

Forge/Cortex integration is intentionally **deferred** until it is explicitly certified for takeover. Ember remains a future external authoring host and must never become a Subspace runtime dependency.

## Supported Windows workflow

Launch:

```text
SubspaceTools.cmd
```

Normal standalone workflow:

```text
Drop .patch in project root
  -> launch internal PCC
  -> approve/reject startup patch prompt
  -> 1. FULL QUALITY GATE / CERTIFY GREEN
  -> test/play the certified build
  -> 2. COMMIT + PUSH CURRENT GREEN
```

A patch being present or successfully copied is **not** acceptance. The Full Quality Gate is promotion authority. Option 2 is guarded by the certified GREEN source/Git fingerprint and must refuse publication when the working tree no longer matches the accepted gate.

Direct automation remains available, for example:

```powershell
.\SubspaceTools.ps1 -Action full-gate
.\SubspaceTools.ps1 -Action run-game
.\SubspaceTools.ps1 -Action run-shipyard
```

## Repository layout

```text
Codename-Subspace/
├─ engine/                 Native C++ runtime, renderer, editor models, tests
├─ GameData/               Canonical authored gameplay/runtime JSON and packaged runtime data
├─ content/                Schemas, source registries, provenance, governed metadata/derived authority
├─ scripts/                Project build/intake/audit scripts
├─ tools/                  PCC/ProjectOps, Blender/PCG, validators
├─ docs/                   Current authorities plus marked historical/reference material
├─ project.control.json    Machine-readable project/operations contract
├─ SubspaceTools.cmd       Standalone PCC root launcher
└─ SubspaceTools.ps1       Project-owned command/gate authority
```

`GameData/` is intentionally current. It is **not** pending an automatic move to `content/data/`. Any future physical data-layout migration requires its own schema/loader migration, compatibility plan, Full Gate, and explicit approval.

## Current Shipyard/editor direction

- the actual ship/station is the construction subject;
- in-game Shipyard edits the same persistent docked ship that later undocks and flies;
- semantic part role is primary; size is compatibility/scale metadata;
- socket placement and attachment semantics are authorable;
- symmetry preserves handedness and socket orientation;
- profile-following shields sit about one foot from the hull and read like still water/glass, with localized impact ripples;
- `EditorDockSystem` is the canonical panel-layout model; the remaining renderer/hit-testing work must make the visible native editor use this same dock tree rather than a second hard-coded layout;
- construction is moving toward sparse parametric elements + authored kitbash + semantic sockets + hierarchical subassemblies rather than an Avorion-style voxel-block identity.

## Current Reference Solar System

The former “Golden Home System” phrase is retired. The **Reference Solar System** is the first production-quality proving environment for:

- seamless space -> atmosphere -> surface -> takeoff;
- large planetary exploration by foot, hover bike, rover, mech, atmospheric craft, and capable ships;
- persistent settlements and exploration economy;
- loaded building/underground cells;
- persistent ships and hangar Shipyard;
- in-system travel drive;
- jump-gate transit to another system;
- economy, factions, logistics, persistence, and save/reload continuity.

Broad procedural-galaxy expansion follows only after this end-to-end slice is real.

## Evidence-based completion

Subspace uses evidence-backed maturity. A class, source file, test fixture, design document, or compile pass does not by itself mean a feature is player-complete. Only `CERTIFIED_RUNTIME` features may be described as done/complete.

See:

- `docs/PROJECT_VISION.md`
- `docs/ARCHITECTURE_AUTHORITY.md`
- `docs/ROADMAP.md`
- `docs/STATUS.md`
- `docs/audits/DEEP_PROJECT_ALIGNMENT_AUDIT_20260912.md`
- `content/architecture/project_completion_truth_v1.json`

## Historical repository-migration compatibility

Older Pass746 certification still recognizes the literal menu label **Prepare normalized GitHub authority**. That phrase is retained here only so historical source gates remain reproducible; it is not the normal current workflow. Normal publication uses internal PCC option 2 after an unchanged GREEN option-1 certification.

## License and third-party content

See `LICENSE`, `CREDITS.md`, `docs/licenses/`, and provenance/source registries. Third-party content must enter through governed intake with license, source, checksum, transformation, and runtime-dependency status recorded.

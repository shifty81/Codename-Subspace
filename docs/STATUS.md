# Codename Subspace Status

## Current source line

**Pass973-997 — Runtime Composition + Assembly Authority A candidate** on top of certified GitHub baseline `f75724b53d2993e03abc00dfaea92fc1dad880d5`.

## Promotion state

- Baseline Full Gate before this candidate: **GREEN** (`QG-20260913-012445-full-8e9d6ae4`).
- Baseline certified source commit: **`f75724b53d2993e03abc00dfaea92fc1dad880d5`**.
- Pass973-997 adds a dedicated native C++ CTest target.
- A new Windows standalone-PCC **Full Quality Gate is required** after applying this patch before promotion/commit.

## Highest-impact current truths

1. Current project identity remains the persistent embodied sandbox/strategy/RPG described in `PROJECT_VISION.md`.
2. Full 3D hierarchical simulation is authoritative; legacy 2D/no-landing and charged sector-jump authorities remain superseded.
3. `NativeRuntimeServices` now owns `WorldSimulationAuthority`, which centralizes stable persistent identities, parent relationships, spatial-frame bindings, residency/representation state, destination prefetch, dirty revisions, persistence checkpoints, schema migration planning, and unresolved-reference evidence.
4. Rotation/velocity-safe `SpatialFrameSystem` remains the coordinate handoff authority; Pass973-997 consumes it rather than creating another transform hierarchy.
5. `AssemblyDefinition` is now the canonical editable ship-assembly contract for the new lane: parametric structure, authored modules, generated connectors, hierarchical subassemblies, semantic sockets, structural attachment graph, quantized transforms, capability validation, undo/redo, and deterministic compile snapshots.
6. `ShipyardAssemblySession` binds editor transactions to the **same persistent Ship ID** owned by world simulation. Commit marks that persistent ship dirty for persistence/replication; preview/revert stay transactional.
7. Existing renderer/PCG recipe and visible Shipyard UI paths are not yet fully migrated to `AssemblyDefinition`; the next priority is adapter/integration work, not another parallel construction model.
8. Normal space -> atmosphere -> planetary-surface traversal remains design authority; spherical streaming, seamless landing/takeoff and destination-prefetch integration are still incomplete runtime work.
9. The project-owned standalone PCC remains current operational authority with Forge-compatible `forge.project.v1` / `forge.project-update-policy.v1` semantics.

## Next highest-impact lane

- wire visible native Shipyard interactions to `ShipyardAssemblySession` and canonical `AssemblyDefinition`;
- add recipe/kitbash -> canonical assembly import adapters without deleting certified legacy recipe data prematurely;
- compile canonical assemblies into renderer/collision/interior/nav products;
- connect world prefetch/residency authority to actual system/interior/planet streaming transitions;
- continue Reference Solar System vertical integration before broad procedural-galaxy expansion.

See `docs/ROADMAP.md`, `docs/PROJECT_VISION.md`, `docs/ARCHITECTURE_AUTHORITY.md`, and `docs/build/PASS973_997_RUNTIME_ASSEMBLY_AUTHORITY_20260913.md`.

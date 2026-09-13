# Codename Subspace Status

## Current source line

**Pass998-1022 — Shipyard Canonical + Streaming Integration candidate** on top of certified GitHub baseline `49ae7ce84b1bf76d3d168c76a22019a156978f2e`.

## Promotion state

- Baseline Full Gate: **GREEN** (`QG-20260913-082855-full-d4cd251b`).
- Baseline certified source commit: **`49ae7ce84b1bf76d3d168c76a22019a156978f2e`**.
- Pass973-997 runtime-composition/assembly authority is now the pushed baseline.
- Pass998-1022 adds a dedicated native C++ CTest target with 25 pass-scoped acceptance assertions.
- A new Windows standalone-PCC **Full Quality Gate is required** after applying this patch before promotion/commit.

## Highest-impact current truths

1. `WorldSimulationAuthority` remains the single representation-independent owner of persistent identity, containment, spatial-frame binding, residency, representation, prefetch, dirty revisions and persistence checkpoint evidence.
2. `AssemblyDefinition` remains the canonical editable ship construction source, with `ShipyardAssemblySession` binding edits to the same persistent Ship ID.
3. Existing certified `ShipyardModuleRecord` + `ProceduralShipVisualRecipe` content can now be imported into canonical assemblies through `ShipyardAssemblyBridgeSystem`; the legacy data is retained as migration/source evidence rather than becoming a parallel editing authority.
4. `ShipyardCanonicalIntegrationSystem` provides the live bridge expected by the visible Shipyard: begin from current certified recipe/catalog, then preview/edit/commit/revert through the canonical persistent-ship session.
5. `AssemblyRuntimeProductSystem` derives renderer instances, collision proxies, aggregate bounds, center of mass, interior candidates, portals, navigation anchors and propulsion presentation from the same canonical assembly.
6. `WorldStreamingIntegrationSystem` turns world prefetch into explicit streaming handoffs; Full representation is not published until residency actually completes.
7. Normal space -> atmosphere -> planetary-surface traversal remains design authority. The next work is to connect these product/handoff contracts to the actual visible renderer/hit-testing and Reference Solar System system/planet/interior streamers.
8. The project-owned standalone PCC remains current operational authority with Forge-compatible `forge.project.v1` / `forge.project-update-policy.v1` semantics.

## Next highest-impact lane

- consume `ShipyardCanonicalIntegrationSystem` in the visible native Shipyard input/selection/manipulation path;
- feed `AssemblyRuntimeProducts` directly into current renderer/interior/collision presentation caches;
- persist accepted canonical assemblies into ship save records and rebuild them on load;
- connect actual system/interior/planet loading jobs to `WorldStreamingIntegrationSystem` completion receipts;
- start Reference Solar System orbit/atmosphere/surface residency boundaries using the same persistent world identity.

See `docs/ROADMAP.md`, `docs/PROJECT_VISION.md`, `docs/ARCHITECTURE_AUTHORITY.md`, `docs/build/PASS973_997_RUNTIME_ASSEMBLY_AUTHORITY_20260913.md`, and `docs/build/PASS998_1022_SHIPYARD_STREAMING_INTEGRATION_20260913.md`.

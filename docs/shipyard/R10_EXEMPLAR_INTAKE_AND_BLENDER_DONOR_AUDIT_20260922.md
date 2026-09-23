# R10 — Authored Ship Exemplar Intake + Blender Donor Inventory

**Status:** independent source-only patch candidate; Windows PCC/Studio acceptance pending.
**Source authority:** Codename-Subspace Git `a08a5d09f3b01a4e6fa134c0015d35ab9bb20c17`. The live Blender extension at `tools/blender/SubspaceShipyard/__init__.py` has Git blob `a5e529e88c769a6425f5e86af24b4cdf2abe4b95`, exactly matching the archived source file audited. It exports `subspace.shipyard_design` version 1 with `modules[].instanceId`, `moduleId`, `moduleClass`, `semantic`, world transform, `connection` parent/socket names, and equipment slots. The extension's Blender module orientation correction is `(X,Z,Y)` into Subspace's +X-right/+Y-forward/+Z-up meter space.

## Scope delivered

All payload paths are **new**. This patch does **not overwrite** `SubspaceTools.ps1`, `engine/CMakeLists.txt`, native renderer, Studio, generator, PCC, or Blender extension; no runtime/BuildGraph behavior is changed. This avoids clashes with outstanding R7/R8/R9 and remains applicable with R9 pending, applied, or committed. Do not add multiple superseded viewport patches to intake simultaneously.

1. `tools/shipyard/exemplar_intake.py` — standard-library, read-only CLI that ingests the **actual Blender extension's** `.subspace_shipyard.json` v1 export, strictly verifies schema, coordinates, numeric transforms and identities, reads optional Grade-A certified CSV, extracts stable nodes and socket-name graph edges, preserves relative *world-space* deltas, records module roles and candidate symmetry, and writes a deterministic staging-only candidate and audit.
2. Rejects duplicate identities, orphaned socket pairs, missing parents, cycles, nonuniform structural scale, catalog class/semantic mismatches and non-Grade-A catalog items. The output is explicitly `DRAFT_UNCERTIFIED`, `generatorEligible: false`, and `promotionAllowed: false` **even when the structural checks pass**, because exported JSON has no certified mesh-geometry/socket-frame/collision/interior/functional evidence. A root count other than one is an error; cosmetic independent decorations need an explicit authored parent or future detached-decoration rule.
3. `tools/blender/kitbash_donor_inventory.py` — **never executes or extracts** SourceWork code. Reads selected Python donors from `SourceWork.zip`, inventories paths/byte counts/SHA-256 and tentative capability categories. Archives, character tools and unclassified sources are kept separate; no source is vendored into Subspace.
4. `docs/shipyard/BLENDER_DONOR_INVENTORY_20260922.json` — metadata-only evidence from the attached `SourceWork.zip`, SHA-256 `4115510fca8e1575b63b81d63d6cb09ed34b0e58d0e8ac3b1b6e5dd65fd2b3f1`. Contains 91 script entries; **not** 91 tested capabilities. No `.blend` files or third-party donor source in this patch.
5. Sixteen Python regression tests and a PowerShell 5.1-compatible **manual focused** test entry point. The scripts are not silently wired into the PCC Full Gate, and their success must not be misreported as full Windows/GUI certification.

## Run on Windows (from the project root)

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\scripts\subspace_exemplar_intake_gate.ps1

# First export a ship using Blender's existing SubspaceShipyard Design JSON action.
# Supply the actual Grade-A CSV from your locally hydrated corpus when available.
python .\tools\shipyard\exemplar_intake.py `
  --input 'C:\Path\to\frigate.subspace_shipyard.json' `
  --catalog '.\content\derived\greyoxide_shipyard_v07\certified\certified_module_catalog.csv' `
  --out-dir "$env:TEMP\SubspaceExemplars\FrigateDraft"

# Optional read-only inventory refresh from your separately stored donor archive.
python .\tools\blender\kitbash_donor_inventory.py `
  --archive 'C:\Path\to\SourceWork.zip' `
  --output "$env:TEMP\SubspaceExemplars\donors.json"
```

The exemplar command returns code 0 for structurally inspectable drafts without errors, 2 for malformed design/structural errors. It **always retains the physical-validation blocker** and does **not** promote anything to procedural generation. It refuses existing output files unless `--overwrite` is explicitly supplied. The source JSON is never rewritten.

## Critical truth boundaries

- This intake handles **Blender `subspace.shipyard_design` v1** only. The native Studio `.subspace_ship`/`.subspace_studio` codecs need separate source-exact read-only adapters; no schema guessing or lossy conversion. The SourceWork `.shipyard.plan.json` files use `nullharbor.shipyard_plan.v1` and are design/planning fixtures, **not** verified module instance graphs.
- CSV class/semantic equality is not geometry validation. OBJ bounds, explicit socket frames/normals, collision, pressure, clearance, engine direction, mesh/mat/UV fidelity, ship-class bounds, interiors, systems, and runtime propagation still require Subspace native checks and visual review.
- `worldPositionDeltaMeters` is **not** a parent-local or socket-relative transform; it is an accurately labeled world-coordinate difference. Do not feed it to a socket solver as a local basis without a proper transform.
- The existing `ShipyardDesignDnaSystem`, `FactionShipDesignSystem`, `FoundryGenerationAuthoritySystem`, `ShipPcgRuntimeClosureSystem` and `GeneratorParitySystem` remain authoritative; this CLI produces only evidence for their future exemplar promotion bridge.
- SubspaceShipyard remains the official Blender client. SourceWork Nullharbor generators are donors only after code, schema, license, Blender version, exact-unit/axis, and visual-parity evaluation. Do not vendor SourceWork or merge its runtime into Subspace.
- A clean R10 focused gate is not a GREEN PCC Full Gate; visual ship acceptance is not covered by unit tests.

## Next source integration tranche (R11)

1. Confirm R9 PCC receipt and the current checkout source fingerprint. Integrate live renderer, viewport picking, and Studio input; manually certify camera/selection at canvas edges.
2. Add a read-only adapter for native Studio ship documents and reuse one internal exemplar candidate representation for both native and Blender exports; no different generator in Blender.
3. Wire `PcgTeachFromAssembly` to submit the **current stable instance graph** to an approval queue. Run the native topology, semantic sockets, mesh-penetration, class envelope, interior, propulsion and saved-ship tests; only after human visual acceptance publish a versioned exemplar.
4. Extend the existing `ShipyardDesignDnaSystem`/`FactionShipDesignSystem` to learn **parent/child semantic-socket pairs, relative socket transforms, mirrored subassemblies, style family and connection exclusions**, not just aggregate role and symmetry counts.
5. Create an end-to-end production proof on one Frigate: manually place a fixed wing, command section and engine pod; approve exemplar; generate two seeded variations; save/reopen both, run gameplay and compare silhouettes. Add a regression case for the original vertical/misaligned-wing defect.
6. Promote Blender donor features one by one into a `Kitbash Foundry` worker contract. Keep Studio native for everyday modeling and Blender optional for mesh cuts/UV/boolean/animation baking. Record each imported function's source SHA and parity test.

**Acceptance condition:** an approved authoring graph reproduces correct stable geometry, sockets, visuals, interior and gameplay function in Studio, the game and Blender, under one generator request + version + seed. R10 is the first read-only intake and donor audit step, **not** that completed outcome.

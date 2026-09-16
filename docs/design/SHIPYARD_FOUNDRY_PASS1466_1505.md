# Shipyard Foundry — PASS1466–1505

This tranche turns the first-class DCC shell into a coherent ship-authoring workflow. It does not introduce a second editor or second kitbash pipeline: existing Shipyard document/history/socket/blueprint authorities, the universal kitbash intake path, and the reusable Subspace dock host remain canonical.

## Pass groups

### PASS1466–1475 — interaction + editor structure
- Correct middle-mouse truck/pedestal behavior by translating camera eye and orbit target together.
- Preserve orbit yaw/pitch/distance after pan so RMB orbit continues without snapping to stale cardinal views.
- Add X/Y/Z and plane transform constraints across assembled modules, staged parts and modeled primitives.
- Normalize fixed application chrome to `File / Edit / View / Help`.
- Extend the native dock host with float, dock, pin, collapse, auto-hide, hover-reveal, resize and serialized layout state.
- Add guided workflow authority so a new author has an explicit starting path.

### PASS1476–1485 — attachments + interiors
- Separate attachment socket from articulation pivot.
- Add per-module Fixed / Manual / Scan Sweep / Track Target articulation with runtime-rendered preview.
- Add detach / reattach and “use selected socket as pivot” workflows.
- Generate a class-scaled interior program from the exterior module graph.
- Add authored floors, walls, ceilings, doors, hatches, airlocks, windows, ramps, stairs, ladders, elevators, corridors, bulkheads and custom structural elements.
- Treat airlocks and other apertures as removable/modelable pressure-boundary content.

### PASS1486–1495 — modeling + appearance
- Expand native model primitives and transformed canonical geometry bake.
- Add modifier contracts for stretch/taper/bend/twist/bevel/inset/extrude/mirror/arrays and future Boolean union/subtract/intersect.
- Add non-destructive source-material suppression and semantic fallback material presentation.
- Add physically inspired metal/paint finishes including polished titanium, brushed steel, black chrome, pearlescent and iridescent surfaces.
- Expand decals with faction-aware crests/wing marks/identity marks, generated hull numbers and warning stencils.

### PASS1496–1505 — material health + kitbash governance + certification
- Add hydrated-corpus OBJ/MTL/texture/UV/normal material-health audit authority.
- Do not report a source-only checkout as a successful asset audit; hydrated content is required.
- Keep external kitbash/material libraries behind the existing governed intake/provenance/license path.
- Make Modeling, Attachments & Pivots, Materials & Paint, Material Health and Kitbash Intake first-class dockable editor surfaces.
- Preserve previous PASS1453/PASS1465 identities as cumulative historical-lineage tokens while presenting `SHIPYARD-FOUNDRY | PASS1505` as the visible build identity.

## Required workflow

`ASSEMBLY → MODEL → ATTACHMENTS → INTERIOR → SYSTEMS → PAINT → VALIDATE → TEST → SAVE BLUEPRINT`

The primary workspace strip is `ASSEMBLY / MODEL / INTERIOR / SYSTEMS / PAINT / TEST`; attachments are contextual because they are edited from both Assembly and Model rather than requiring an isolated application.

## Explicitly not claimed complete

- Boolean topology execution has a native contract but the production robust Boolean backend is still pending. Manifold remains a researched candidate rather than a silently vendored dependency.
- Generated mesh UV unwrap has a native need and xatlas is a researched candidate, but xatlas is not vendored in this tranche.
- A complete Greyoxide/other external module material audit requires the hydrated local asset corpus. The repository-only audit must skip instead of inventing results.
- External source libraries such as Quaternius or Poly Haven are not blindly copied into the repository; each intake must preserve provenance, source license and hashes.

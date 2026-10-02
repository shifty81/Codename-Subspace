# NullHarbor R179 — Planetary Command Convergence

R179 is a cumulative gameplay/normalization milestone layered on R178. It converts the previously pending R158-R177 Planetary Command migration from a brittle textual prebuild transform into current native behavior and a semantic compatibility verifier.

## Delivered

- stable `planetId:q:r` planetary-sector identity;
- surveyed / claimed / developed territory state;
- owner-bound contiguous territory expansion;
- governed construction authorization;
- six strategic overlays: resources, ownership, industry, logistics, power and hazard;
- globe and axial-sector projection modes;
- shared projection authority for rendering and hit testing;
- sector selection, pinned inspector and claim-frontier visibility;
- P / LMB / RMB / F5 / Enter / Shift+Enter live command workflow;
- player-facing Planetary Command naming while retaining internal compatibility enum IDs;
- append-only input extension after R178;
- compatibility bridge to existing planet-scale industrialization state;
- retirement of historical R158-R177 transform replay;
- semantic migration verification suitable for future source evolution.

## Intentional compatibility boundaries

The existing `PlanetaryIndustrySystem`, `PlanetaryIndustrializationSystem`, save-facing data and workspace enum are evolved, not replaced. R179 does not introduce a duplicate planetary economy. Legacy low-level placement remains available to historical tests/data; new player-facing placement is governed by ownership/development.

## Next convergence target

The next large milestone should connect Planetary Command to the broader NullHarbor strategy simulation: corporation ownership and permissions, persistent settlement/installation identities, fleet logistics and transport orders, planetary extraction feeding the canonical resource graph, surface/FPS traversal handoff, and multiplayer-authoritative claim/build permissions. That work should reuse the R178 Fleet Strategy and FPS/interior authorities rather than grow another command stack.

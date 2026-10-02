# NullHarbor R185 — Native Compatibility Closure

R184 finally retired the hidden R177 Planetary Command text transformer and allowed the authoritative Windows Full Gate to reach CMake/MSBuild/CTest. The native build succeeded, exposing four remaining test targets with three underlying compatibility regressions.

## Failures exposed by the real Windows gate

1. `SubspaceForwardTests` / Pass309: accelerated first-person locomotion moved less than the historical cockpit interaction radius during one short test step, so `TakeControls()` stayed available after the player had started walking away.
2. `SubspacePass513To522ProjectContinuationTests`: the same cockpit interaction regression.
3. `SubspacePlayerFacingTests` / Pass408: R179 changed the historical direct PI helper to governed placement, breaking the pre-R179 ad-hoc `PlanetData` test contract.
4. `SubspaceProjectOpsStaticCertification`: the historical R158-R177 gate still demanded `SelectPlanetCommandHex` instead of validating the R179 semantic Planetary Command authority.

## R185 normalization

### Cockpit interaction

`ShipEmbodimentSystem` now tracks explicit command-seat interaction eligibility independently of accelerated displacement distance. Leaving the cockpit starts with the seat interaction armed. Any non-zero movement intent disarms it. The player must actually depart the seat radius and later re-enter it before the interaction becomes armed again. Shell-certified foot positions participate in the same state update.

This preserves R178 acceleration/stamina/stance locomotion while restoring the physical interaction behavior certified by Pass309 and Pass518-519.

### Planetary Industry compatibility boundary

`PlanetIndustryRuntimeModel` records `legacyPlacementCompatibility` only when the input `PlanetData` lacks a canonical `planetId`. Historical ad-hoc PI callers may still survey/place directly through `PlaceIndustry`, preserving Pass408.

Generated runtime planets receive stable `planetId` values from `GalaxyGenerator`; they therefore continue through R179 `PlaceGoverned`, requiring developed territory owned by the issuing player/corporation. The live Planetary Command claim/develop loop is not weakened.

`SelectPlanetCommandHex` is restored as a compatibility alias over the R179 stable-sector selection model rather than reintroducing an older parallel implementation.

### Historical static gate

`pass1578_1597_planetary_command.cmake` is normalized into a supersession gate. It now verifies the current R179 claim states, overlays, globe/sector projection, governed placement, command hit testing, selection compatibility alias, live application routing and renderer presentation. It no longer certifies retired source-transform preimages.

## Portable certification

- Pass296-315 Forward Tests: 77/77 PASS
- Pass401-410 Player-Facing Integration: 43/43 PASS
- Pass513-522 Project Continuation: 25/25 PASS
- Pass361-400 Command Galaxy: 76/76 PASS
- R178 Stabilization: 95/95 PASS
- Master native suite: 4096/4096 PASS
- normalized R158-R177 historical Planetary gate: PASS
- R185 compatibility closure gate: PASS

Windows PCC Full Quality Gate remains the final authority after patch application.

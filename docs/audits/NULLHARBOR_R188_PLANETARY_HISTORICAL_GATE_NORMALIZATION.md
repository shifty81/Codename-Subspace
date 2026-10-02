# NullHarbor R188 — Historical Planetary Prebuild/Matcher Gate Normalization

The 2026-10-02 08:56 Full Gate reached CTest with 4,790/4,790 native assertions green and 155/156 CTest targets passing. R187's normalized R183 gate passed.

The only failure was `pass1598_planetary_command_prebuild_repair.cmake`, which still required the retired R177R1 `Invoke-PlanetaryCommandMaterializationIfRequired` function. Inspection showed the immediately following `pass1599_planetary_command_migration_matcher.cmake` also still required retired R177R2 matcher internals.

R188 normalizes both historical gates together:
- pass1598 now certifies R184's canonical verification hook and verification-only PowerShell entrypoint;
- pass1599 now certifies the R179 semantic source verifier plus R179/R184 authority;
- a new R188 guard and regression test prevent those obsolete dependencies from returning.

No C++, runtime, gameplay, Studio, renderer, PCC implementation, application, recovery behavior, or content files are changed.

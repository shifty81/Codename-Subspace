# NullHarbor R184 — PCC Planetary Command orchestration recovery

R184 removes the last hidden R177R1/R177R2 prebuild replay from the project-owned PCC.

The authoritative `SubspaceTools.ps1` is reconstructed from the certified September 17 Git blob `b3f93b47a1f730b827335bacf4a82d23afe13a00` plus GitHub's committed delta through `8484a081f8d60be980d3aac300ba2f1f9397cd75`. Before the R184 addition, that reconstructed file was verified to equal Git blob `4b2b7bf8117c6fcdb9798624903485cc1e7097f1` exactly.

The locally installed R177R1/R177R2 orchestration was never part of the September 30 Git authority. It injected a pre-CMake exact/indentation-normalized source transformer which later attempted to replay historical R158-R177 text changes over R178/R179/R182 source. That is the source of the repeated `expected one certified preimage` failures.

R184 restores the certified PCC and adds one current behavior: `Invoke-CMakeBuild` calls `Invoke-PlanetaryCommandCanonicalVerification` before CMake. The verifier invokes `scripts/subspace_planetary_command_r158_r177_apply.ps1`, which since R179/R183 is verification-only. It proves the canonical Planetary Command semantic owners and may emit migration receipts, but never rewrites modern gameplay source.

The R184 static gate fails if the PCC contains the retired pending-materialization message, certified-preimage matcher text, or indentation-normalized replay language.

No C++ gameplay file is modified by R184.

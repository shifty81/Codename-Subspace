# R187 — R183 Static Gate Semantic Supersession

## Trigger

After R185 and R186, the Windows Full Quality Gate reached CTest with all native
assertion suites green (4,790 / 4,790). Only `SubspaceProjectOpsStaticCertification`
failed. The failing historical R183 gate still required the private Python token
`PLANETARY_AUTHORITIES` inside `studio_r180_overlay_repair.py`.

R186 intentionally refactored the one-time R182 recovery into a descendant-compatible
semantic verifier, so that private R183-era variable no longer exists. The Planetary
Command runtime and verification-only migration entrypoint remained healthy.

## Resolution

R187 changes no C++, gameplay, Studio, renderer, PCC, or recovery behavior. It updates
R183 certification to validate durable semantic owners instead:

- `scripts/subspace_planetary_command_r158_r177_apply.ps1` remains verification-only;
- `tools/control/tests/test_r158_r177_planetary_command_source.py` verifies current R179 semantics;
- `nullharbor_r179_planetary_command_convergence.cmake` remains the current source authority;
- `nullharbor_r184_pcc_planetary_orchestration.cmake` remains the PCC orchestration authority;
- R186 remains the governed-descendant authority after one-time R182 recovery.

The gate explicitly forbids dependencies on retired private names:
`PLANETARY_AUTHORITIES`, `PLANETARY_MARKERS`, `validate_planetary_authorities`, and
`write_planetary_materialization_markers`.

## Scope

Static-certification normalization only. No runtime source is modified.

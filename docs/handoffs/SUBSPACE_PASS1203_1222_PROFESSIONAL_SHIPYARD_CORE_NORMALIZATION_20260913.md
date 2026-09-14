# Pass1203-1222 Test Handoff

After patch intake, run the project-owned **FULL QUALITY GATE / CERTIFY GREEN**.

This tranche is intentionally architecture-first. The current Shipyard screen should remain operational while the new source files enter the normal CMake source glob.

Acceptance priorities:

1. Full build and historical CTest/static gates remain GREEN.
2. No existing Shipyard save, generator, placement, input or project-control behavior regresses.
3. New normalization sources compile as part of `subspace_engine`.
4. The new static gate confirms document/session separation, stable IDs, command/history authority, five primary workspaces and MountProfile root-surface policy.
5. Do not expect the visible top bar to change yet; that is the next tranche after this foundation is certified.

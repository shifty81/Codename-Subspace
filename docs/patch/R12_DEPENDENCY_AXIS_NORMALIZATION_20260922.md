# R12 — dependency restoration and generic axis compatibility (reconstructed package)

Baseline: `shifty81/Codename-Subspace` published `main` `351469aaf3954981c74990399f5a517798f3df9b`. Reconstructed from the original R10 payload and source bytes verified against the exact published Git blobs. The previously shared R12 file was absent in the handoff session, so this is a **newly created replacement artifact**, not a byte-for-byte copy of the lost ZIP. Do not infer previous package SHA equivalence.

## Changes

- Restores the seven exact R10 files missing from the published R11-only commit, including `tools/shipyard/exemplar_intake.py`; the R11 compiler imports it. The files are exact unchanged R10 bytes.
- Changes `ConstructionSymmetrySystem::AxisName()` editor-facing text to MIRROR X/Y/Z without changing the numeric legacy enum, reflection math, socket name mapping, assembly direction or source mesh coordinates. Adds `LegacyAxisName()` to preserve old human-readable terminology and `AxisIndex()` to test axis identity. These two overwritten source files were checked against Git blobs `2953dbb72d9cdf4657cebd96fe2a4eda893b7109` and `e5753315c0b9ea1c375db69f47ec9fe7bffbf797` at the pinned baseline.
- Adds a standalone symmetry/baseline shell geometry CTest and an explicit R10/R11 focused Python gate. No Blender or external engine dependency for game runtime.

## Apply

1. Check `git rev-parse HEAD` equals `351469aaf3954981c74990399f5a517798f3df9b`. If different, stop and recut. Check `git status --short`, especially `engine/include/editor/ConstructionSymmetrySystem.h` and `engine/src/editor/ConstructionSymmetrySystem.cpp`. If either is locally changed, stop rather than overwrite.
2. Place the `.patch` ZIP *unextracted* in the project root. Approve through the internal PCC. The ZIP includes no delete requests.
3. Run `scripts\subspace_studio_r12_gate.ps1`, then internal PCC **Full Quality Gate**, then inspect the editor manually. Do not commit/push on a failed gate.

## Limits

This does NOT fix rotated-part scaling, source geometry axes, overlap in the materialized live dock tree, or merge Model and Assembly. The isolated shell test proves only the legacy geometric baseline, not real GUI/window DPI, live OpenGL clipping or mouse interactions. Do not automatically promote exemplar drafts to executable PCG.

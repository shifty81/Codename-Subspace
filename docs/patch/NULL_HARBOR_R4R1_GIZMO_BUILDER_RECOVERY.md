# Null Harbor R4R1 — Restore the gizmo builder required by R4

## Trigger
The September 21 Windows Full Gate built the game and Studio but failed `SubspaceProjectOpsStaticCertification` on `engine/src/studio/StudioAxisGizmo.cpp :: StudioGizmoProjectionPolicy::ProbeUsable`. Test 130 (projection policy) and test 131 (R4 bridge) passed.

R4 upgraded `StudioGizmoProjectionPolicy.h` and its certification gate but **did not ship** `StudioAxisGizmo.cpp`. The R3 implementation can therefore remain in the checkout while the new policy and pure-policy tests pass. R3's builder discards off-screen probe endpoints, which is the original camera-angle visibility defect. Restoring the R3R2 builder addresses the actual missing integration; weakening the R4 gate would be incorrect.

## Scope
- Restore the exact R3R2 `engine/src/studio/StudioAxisGizmo.cpp` payload, which uses shared `PopulateHandles`, accepts projected probes with positive depth via `ProbeUsable`, builds distinct XYZ handles through `BuildHandles`, and preserves Model and Assembly transform mode behavior.
- Preserve all R4 StudioApplication, projection policy, dock reflow, interior preview bridge, shaders, CMake definitions, project IDs, and rebrand state.
- No gate edits or test removals. The matching R4 CMake gate and R4 Python checker remain authoritative.

## Prerequisites and safety
The R4 bulk patch must already be applied. The target `StudioAxisGizmo.cpp` must be the exact R3 source (SHA-256 verified by PCC). If a different revision exists, **do not force apply**: supply the local file or latest debug bundle for reconciliation. This patch is additive to R4 and does not reapply R3R2 or earlier handoffs.

## Verification
Isolated fixture: exact R4 gate fails on R3 builder at `ProbeUsable`, passes after the source restore; R4 Python checker passes with the restored builder and rejects the R3 regression. Full Windows build/CTest and visual Studio acceptance still require the user to run PCC and inspect gizmo from multiple angles. No claim of completed GUI normalization, ghost-hull quality, shader rewrite, or project rebrand.

## Apply
Drop this `.patch` unextracted into repo root; explicitly approve in PCC; run Full Quality Gate. Inspect the model/assembly gizmo across all camera directions, including panels covering the shaft and pivot. Commit/push only after green and hands-on acceptance.

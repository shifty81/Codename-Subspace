# Null Harbor R4R2 — Restore the missing all-angle projection regression test

## Root cause and evidence

The September 21 11:18 Full Gate compiled the game and Studio successfully, then passed 130/131 CTest tests. Its sole failing test was ProjectOpsStaticCertification: the R4 `studio_foundation_r3_authority.cmake` gate could not find `assert(samples==4940)` in `engine/tests/studio_gizmo_projection_policy_tests.cpp`.

The R4R1 patch restores `StudioAxisGizmo.cpp`, but does not deliver the expanded projection test. The R3R2 patch archive *does* contain that test. The published bd59a39 baseline and the R3 test contain only the earlier camera-aligned assertions, which explain why a successfully compiling test target is insufficient.

This repair restores the R3R2 test **without modifying the existing projection policy, gizmo builder, R4 interior bridge, UI, shaders, or static gate**. It checks exact SHA-256 preconditions on all four relevant files. It refuses to apply over an unexpected local test or differing R4/R4R1 implementation. It is a ZIP-format `.patch` root-drop for project PCC; do not extract it or reapply earlier patches.

## Test coverage restored

- Detect valid offscreen projection and reject behind-near-plane/nonfinite probes.
- Preserve physical axis identity and independent hit targets for end-on, parallel and near-parallel projections.
- Verify viewport-edge handling and deterministic 4,940-case camera/viewport sweep.
- Retain the unchanged static gate's exact regression-suite requirement.

## Verification boundary

In an isolated fixture constructed from the supplied R3, R3R2, R4 and R4R1 patch payloads, the R4 gate fails before repair for the same missing assertion, passes after restoring this test, and fails again when the earlier test is put back. The R4 Python guard passes, and the C++ projection test compiles and executes. This is not a Windows Full Gate, GPU viewport acceptance, or completed R5 GUI polish.

## Use

Drop the archive unextracted into the repository root, explicitly approve and apply through PCC, then run Option 1 Full Quality Gate. Do not commit/push until GREEN and actual gizmo visibility, mouse picking, interior display and GUI layout are checked. If source hash verification fails, stop and supply the latest debug bundle / the changed target source.

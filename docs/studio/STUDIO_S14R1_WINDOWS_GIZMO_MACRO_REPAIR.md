# Studio S14R1 - Windows gizmo compilation repair

## Evidence and applicability

The debug bundle `Subspace_DebugBundle_20260918-195330.zip` records the S13+S14 cumulative patch as applied at 19:53:28 and a subsequent Full Gate failure during the Studio MSVC build. Both `studio_main.cpp` and `StudioApplication.cpp` fail at `StudioGizmoMath.h(32,41)`, error C2059. The last certified GREEN belongs to S12; the reported 121 CTest passes are historical, not new S14 certification.

This incremental repair **requires S13+S14 already applied**. It targets the same currently uncommitted Git HEAD `139279ca32b78cfc71f990823fc72042ff2c4883` and changes just the affected gizmo header plus its existing focused test. Do not reapply the old S13+S14 package, reset local files, or commit the failed gate. It preserves S13 close protection, S14 gizmo/GUI changes, universal PCC, and game behavior.

## Cause and fix

The local variable `near` in `StudioGizmoMath::Hit` is incompatible with the Windows legacy `near` macro. A macro-emulated C++17 compile of the original reproduces a missing second argument at line 32. Rename the local variable to `nearestPoint` and use the new identifier in the distance check; no axis math, handling, or behavioral tuning changes.

The existing `studio_gizmo_math_tests.cpp` now defines a temporary `near` macro **before** including the gizmo header and undefines it afterward. Its existing axis-hit, drag, and angle assertions exercise the same header in a macro-polluted configuration. No CMake target additions or PCC contract rewrites.

## What to run on Windows

1. Place only this `.patch` unextracted at the repository root, then relaunch/approve it in the project-owned PCC or ForgePY native provider.
2. Run Full Quality Gate (option 1). Inspect the *new* run for green compiler, current tests, Studio smoke, and source fingerprint.
3. Open the Studio via project PCC / ForgePY; verify axis-hit, move/rotate/scale, tick/angle gauge, Esc, S13 close prompt, and the game launcher.
4. Commit/push only after the new gate and visual behavior pass. Upload a new debug bundle on a further failure.

## Limitations

No Windows compiler or interactive GUI is available in this environment. Focused C++ compiles and tests with macro injection do not certify full MSVC/link/Windows gate or the visual gizmo behavior. This patch addresses the exact first compiler blocker reported, not broader Studio usability requests.

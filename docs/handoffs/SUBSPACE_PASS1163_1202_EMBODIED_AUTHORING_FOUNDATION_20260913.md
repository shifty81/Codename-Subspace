# Pass1163-1202 Test Handoff

Baseline: certified GitHub `c9754358185115a4fd929f3e79a4e8bae676b3b3` (`QG-20260913-184114-full-7478ecb7`).

After root-drop intake, use the project-owned PCC and run **1 — FULL QUALITY GATE / CERTIFY GREEN**.

Manual acceptance priorities:

1. **Cockpit 6DOF:** normal W/S/A/D + Q/E remain functional. PageUp/PageDown must produce real local vertical thrust; arrow Up/Down must pitch; arrow Left/Right must roll. Pitch the ship and thrust forward: motion must follow the ship nose in Z rather than remaining on an XY plane.
2. **Shipyard safety:** open Shipyard and use PageUp/PageDown/arrows for editor work. The authored ship must not receive flight force/torque while build mode owns input.
3. **Editor placement:** orbit above, below and beside the ship, then drag a catalog part through the viewport. The staged ghost must remain visible/controllable instead of disappearing when the camera is no longer looking cleanly at global Z=0.
4. **Dock shell:** confirm the default professional layout is Tools + Asset Browser | large 3D Viewport | Outliner/Properties, with History/Validation/Console/Forge collapsed. Interior Program, Apertures & Hangars and Play/Test should exist as dockable panels.
5. **Interior/FPS authority:** use the current interior/on-foot lane and confirm look-relative movement is consistent. Generated traversal bounds should be able to replace the legacy demo room bounds without changing player scale.
6. **Aperture/hangar rules:** validate a standard player door and at least one rover bay. Reducing an opening below the certified envelope must fail rather than silently accepting an unusable hangar.
7. **Shield:** existing visible shield may still use the old renderer source in this pass. Do not judge final connectivity yet. The new contract requires the next renderer lane to produce one closed smoothed envelope from the cohesive exterior bake.

Portable validation performed before packaging: **42/42 acceptance checks PASS** and all modified/new C++ translation units used by the tranche compile under C++17 `-Wall -Wextra -Werror` against the source lineage. The Windows PCC Full Gate remains authoritative.

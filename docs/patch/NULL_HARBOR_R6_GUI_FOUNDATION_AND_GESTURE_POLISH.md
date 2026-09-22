# Null Harbor R6A — Studio GUI foundation and widget gesture polish

Patch ID: `null-harbor-r6a-gui-foundation-gesture-20260921`  
Depends on: **R5 Projection-Correct Drag & GUI Interaction** already applied.

## Implemented

- Preserves R5's perspective-calibrated model/assembly Move math and press-origin absolute offsets. No change to translation units or transform storage.
- Adds a 3-pixel activation threshold to both modeling and assembly gizmo transactions. Pointer jitter while clicking a handle must not move the object or create an undo entry.
- Uses one read-only GUI interaction policy for drag activation and full-rectangle HUD occlusion. Previously, nine point samples missed thin floating panels; the HUD then painted over them.
- Uses the existing ordered dock compositor as the source of floating-panel rectangles. A half-open intersection helper is reused by readout placement and tests.
- Normalizes the existing ForgeGUI style palette to dark navy / blue, improves muted-text contrast, and keeps compact rows and tool rail above minimum readable dimensions.
- Corrects the two unsaved-work dialog titles to Null Harbor Studio.
- Registers deterministic C++ regression tests with the existing CMake/CTest system.

## Intentionally NOT claimed or changed

- The battlefield renderer still has its own legacy gizmo visuals in addition to the Studio overlay; removing those safely requires editing the current `NativeBattlefieldRenderer.cpp` under a dedicated standalone/editor presentation flag. Do **not** hide them by mutating live `showGizmos` document state or copying the entire catalog per frame.
- This pass does not claim complete Inspector redesign, panel-layout persistence overhaul, true rotation ring/square scale widgets, native font replacement, or UI acceptance on Windows.
- This patch is only the first bounded implementation slice of the GUI overhaul, not a certification of the final interface.

## User acceptance after FULL QUALITY GATE

1. Confirm gizmo movement remains as responsive as R5 at multiple zooms, with snapping off then on.
2. Click an XYZ marker without dragging; no object's transform or undo state should change. Drag more than 3 pixels; the correct object/axis should move.
3. Float Inspector over upper-left 435x106 HUD rectangle; HUD disappears rather than overpainting Inspector; move Inspector aside and HUD returns.
4. Verify dark navy panels, blue selected controls, more readable Asset Browser/Inspector rows and action tool rail.
5. Confirm close dialogs say Null Harbor Studio and still protect unsaved work.

## Gate truth

Linux-focused standalone test compiles and runs. The partial September 17 source archive is missing newer Studio/renderer entrypoint files, so it cannot certify the complete current Windows build. Run project PCC Full Quality Gate and inspect in a real Studio window. Reject any preimage-hash mismatch rather than forcing an overwrite.

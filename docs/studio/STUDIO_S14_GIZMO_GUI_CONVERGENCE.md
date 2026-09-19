# Subspace Studio S13–S14 — cumulative close safety + axis manipulation / first GUI polish

## Source authority and safe intake

- Base: `shifty81/Codename-Subspace` Git `139279ca32b78cfc71f990823fc72042ff2c4883` (S12 certified).
- One **cumulative** `forge.patch.v1` root-drop package contains all nine replacement/addition files from S13, plus the S14 source and tests. It is not a complete source rollup.
- Apply **instead of S13** when S13 has not yet been installed. If S13 was already applied or committed, DO NOT force this old-baseline package through the PCC. Request a rebase from your current head/state.
- The standalone project's `SubspaceTools.cmd`, `forge.project.v1` contract, protected patch engine and Git gate remain authoritative. ForgePY stays external; no vendored Rust/egui build dependency.

## The actual confusion addressed

The previous `StudioApplication::HandleInput` selected a part on mouse press and immediately began an unconstrained move/rotate/scale transaction. It did not pick individual manipulators. Moving projected camera-right and camera-up was sensitive to camera orbit; rotation mapped the same 2-D drag to multiple Euler components. The small top-right XYZ widget was only a viewport orientation decoration, not a selectable part gizmo. The rail labeled real tools only Q/G/R/S.

### S14 implementation, scoped to native standalone Studio

- Added `StudioAxisGizmo` which uses the renderer's public authoritative `WorldToScreen` projection and *the same selected module position/scaling/root forward yaw as the existing ship part picker*. Three world-projected 68-pixel X/Y/Z handles are centered at the selected placed module. The fixed top-right orientation widget is not mistaken for the new manipulators.
- `StudioGizmoMath` owns pure selection, shaft/tip hit zones, projected single-axis pointer scalar, wrapping and canonical physical rotation mapping. Pivot clicks never ambiguously choose an axis. Gizmos are suppressed for Select, staged catalog drags, non-Assembly workspaces including unfinished Model primitives, and socket Inspector; they are not fake editable controls on those surfaces.
- Press an X/Y/Z handle to begin an existing `ShipyardBuilderSystem` transform. Drag MOVE along that exact **ship** axis, SCALE only that dimension, or ROTATE around that physical axis, using the already shared transaction and undo/history/validation. Free part-body dragging still works as a separate existing path. The new handle is not an independent physics transform or a second blueprint format.
- The renderer's module orientation order is `Rz(yaw) * Rx(pitch) * Ry(roll)`: **X→pitch, Y→roll, Z→yaw**. Stored translation axes remain X starboard, Y forward, Z dorsal. Module angles are relative to that ship frame, not arbitrary camera degrees; the recipe's `forwardVisualYawDegrees` only affects its on-screen direction. This is the derived authoring angle authority *for S14 gizmo edits*, not a claim that every modeling/simulation subsystem has been migrated to it.
- Drag targets are calculated from the transform's original state plus accumulated pointer pixels. Snap uses the transaction's existing translation, rotation and scale increments; this avoids losing many small mouse events against the 15-degree step. Shift gives precision and temporarily skips rotation snapping. The active rotation handle displays a colored arc, 15-degree ticks, sweep marker, and **numeric angle difference** derived from the actual applied transform.
- Clicking a handle without moving or without reaching a snap step cancels rather than adding a false undo entry. Esc cancels the entire transaction and preserves the previous constraint. Release commits exactly once and restores the prior global/local constraint. No gameplay host input mapping or source was touched.
- Existing ForgeGUI-inspired canvas/dock/control authority is retained. Standalone rails now use `SELECT`, `MOVE`, `ROTATE`, `SCALE`, `SNAP ON/OFF`, `FRAME`; the viewport toggle reads `AXES`, and narrow contextual Inspector tabs read `XYZ/ASM/SCK/DEF/MAT`. These are label improvements on **existing functional commands**, not decorative extra buttons or a separate GUI fork. Game-only labels remain unchanged.

## S13 carried forward unmodified

Native HWND WM_CLOSE veto; Save and Close / recovery / Cancel; second warning for data that the blueprint codec cannot serialize; failed or canceled save prevents close; Studio-only CMake wiring and S13 tests. No new assertion that recovery contains the editable model or interior draft.

## Remaining work / explicit limitations

- Not a complete ForgeGUI migration. Studio still uses the legacy shared immediate-mode native drawing/compositor, including the crowded advanced workspaces and missing **clickable File→Open / Save As** menu actions. Those two actions remain available only through Ctrl+O and Ctrl+Shift+S from S12.
- Axis gizmos are for **placed ship modules in Assembly**. Model primitive topology, socket-level gizmos, a genuine 3-D object/local rotate ring, multiple-selection pivots, precision numeric entry, accurate orthographic view selector, smart snap targets and model-recipe persistence remain for subsequent certified passes. Do not claim otherwise.
- Rotation uses existing Euler + snapped transaction, not a unified quaternion node/rigging transform; surface, socket and interior adapters still require a separate certification pass. Z direction can be nearly parallel to the camera at some angles: degenerate handle vectors fail closed rather than accepting an incorrect axis.
- The arc is a screen-space UI angle gauge, not a mesh or geometry modifier, and does not become gameplay geometry. Text is a small native 7-segment OpenGL angle label, no new Windows font dependencies.
- Windows OpenGL state/visibility, mouse capture, floating-panel occlusion, full PCC gate, exact pixel alignment and game smoke still require local validation. The older September 17 source snapshot is not the current GitHub tree; source-only C++ syntax and focused tests must not be called a complete Windows build.

## Local acceptance (in order)

1. Start from clean Git 139279c. Root-drop **only the cumulative S13+S14 .patch** unextracted; approve through PCC. Run Full Quality Gate, await new GREEN fingerprint.
2. Studio: place a hull, choose MOVE, click **its** red X, drag diagonally. Only X coordinate should change even after right-mouse camera orbit. Repeat green Y / blue Z, rotate and scale. Click handle without dragging → no undo; Esc during drag → reverts. Shift → precision; toggle SNAP → expected step behavior.
3. ROTATE: verify X pitch, Y roll, Z yaw with the arc numeric readout, both +/- movement and existing 15-degree snap. Switch camera view after releasing: committed physical angle must not change.
4. Dock/float Assets and Properties over the ship: panel hits must win; handles behind floating panels must not capture. Test narrow window and maximized viewport. Check status and Inspector labels remain legible. Ctrl+O and Ctrl+Shift+S still operate.
5. S13 close: unsaved ship + X / Alt+F4 Cancel keeps window; Save As cancel vetoes; recovery is verified before closing. Model/interior dirty warns before partial recovery. Launch game normally; its menus and visuals must be unaffected.
6. Commit/push only after the PCC GREEN, window visual acceptance and gameplay smoke. On failure package the new PCC debug bundle; never force a patch baseline mismatch or bypass the source-fingerprint gate.

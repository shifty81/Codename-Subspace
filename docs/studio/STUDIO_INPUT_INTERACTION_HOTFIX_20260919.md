# Codename Subspace — Studio Assembly input hotfix (after G02)

**Status:** ready for source-guarded local application; **not** Windows-tested, not Full Gate GREEN, not the G03 model document/rendering milestone.
**Verified published source:** `4f76f5124c2def175479f11e91422d1da3e015f0` / gate `QG-20260919-154436-full-1586cc95`. The local source may have advanced. Never force this onto a mismatch.

## Scope / actual source issues

- `NativeWindow` publishes DCC shortcuts for sidebar, toolbar, maximize, XYZ constraints, Delete and arrow/page nudges, but standalone `StudioApplication::HandleInput` only consumes a subset. Route the missing actions through the same authoritative `ShipyardBuilderCommand` used by existing UI.
- `WM_KILLFOCUS` clears `InputState` but leaves private `_controlDown` and `_shiftDown` flags set; if KEYUP occurs while another app is active, shortcuts can remain incorrectly modified. Clear modifiers and pending clicks upon focus loss.
- The existing `StudioToolInteractionPolicy.h` is present but not connected to the actual pointer path. Clicking an *unselected* ship under Move/Rotate/Scale previously retargets and begins a free transform in the same action. Allow selectable retargeting only with Select; allow free transform only when clicking the already-selected part; gizmo handles retain the existing Begin/Commit/Cancel transaction. This preserves explicit selection and prevents unexpected target jumps.
- A real drag can leave `suppressClick_` true although it has no click event. Clear the stale flag at the *next new press*, while preserving suppression of the release/click from the current gesture.
- Asset Search owns typed keys. Ctrl+N/O/S still use the existing separate document router; avoid binding Ctrl+S to Scale or Ctrl+N to sidebar.

## Installation / guard

1. Extract the outer delivery ZIP **outside** the repository. Put only its internal `.patch` in the Subspace repo root. Approve with project PCC; this stage installs the guarded script, launcher, tests and documentation only.
2. Run `tools\studio\ApplyInputHotfix.cmd --check`. This is read-only and checks the EXACT published Git blob of both affected C++ files and all code anchors before permitting any change.
3. **Only if PASS**, run `tools\studio\ApplyInputHotfix.cmd --apply`. The script backs up originals under `.subspace/recovery/studio-interaction-hotfix/<timestamp>`, atomically writes each file, hash-verifies and emits a receipt; errors trigger rollback. The script performs no Git reset, commit, push or quality gate.
4. Run the PCC Full Quality Gate and personally test in Studio. Do not commit/push on a failing gate or failed interaction test.

The existing PCC root patch engine overlays whole files; it cannot safely do source hunks on a locally changed file. The second explicit step is required rather than silently overwriting a local Studio or native-window implementation. A conflict means **STOP** and send the message/updated source—not a reason to run force commands.

## Windows interaction acceptance

Test with a **real assembled ship** in Assembly/Build (not an empty Model BOX): Q selects a known wing; W or G chooses Move; drag gizmo X/Y/Z one at a time; drag only the already-selected wing for free Move. E or R chooses Rotate, S Scale; verify accurate nonzero changes and that an unrelated module is not silently retargeted. X/Y/Z constrain the active transform, Delete removes selected part with Undo restoring it, arrow/Page Up/Down nudges, F frames selected, Home frames ship, T/N toggle rail/sidebar, Ctrl+Space maximize, F3 command palette, Shift+F asset filter, Ctrl+Z/Y Undo/Redo, Ctrl+S save and Ctrl+N new with normal unsaved guard. Type Q/W/E/S/X in focused Asset Search: no tool mutation. Switch to another Windows app while Ctrl/Shift is held, release outside Studio, return and check W/Q and Ctrl+S are not confused. After real drag, next unrelated UI click must register. Cancel an active gizmo with Escape and verify no mutation. Repeat close with Don't Save and verify G02 still returns process 0 after explicit consent.

**Excluded from this hotfix:** Model BOX visibility and transform; app-wide unified focus and modal keyboard controls; complete gizmo ray/plane maths, mouse capture loss, panel-occlusion certification, multi-DPI testing, interior/document persistence and complete GUI polish. These require subsequent G03/G04/T11–T20 implementations. Passing automated PCC gates alone is not interactive certification.

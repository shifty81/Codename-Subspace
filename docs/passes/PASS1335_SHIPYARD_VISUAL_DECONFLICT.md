# Pass1335 — Shipyard Visual Deconflict

The screenshot after Pass1334 exposed four concrete layout defects in the visible
Shipyard shell:

1. the DEV shelf occupied y=49..79 while the left/right panels began at y=58;
2. the selected-module summary is 54 px tall but the Outliner rows started only
   34 px below it;
3. advanced workspaces deliberately re-injected `LegacyBuildControls` into the
   visible right side, creating a second overlapping UI;
4. decorative Outliner/Properties controls duplicated renderer-owned panel chrome.

Pass1335 fixes those defects without changing the canonical ship model, generation,
placement, transform, persistence, or validation authorities.

## New layout rules

- dedicated 88 px top navigation band;
- DEV shelf and panel content never share vertical space;
- selected-module summary and hierarchy rows have explicit separation;
- renderer owns section chrome exactly once;
- visible controls are interaction rows only;
- legacy compatibility controls remain discoverable to historical tests but are
  permanently projected far off-screen;
- advanced workspaces no longer re-render the old right-side page over the new shell.

This is a stabilization pass before the standalone Blender-inspired editor shell.

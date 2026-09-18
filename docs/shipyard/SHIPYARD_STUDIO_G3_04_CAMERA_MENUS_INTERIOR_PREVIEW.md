# Subspace Studio G3: editor navigation + functional first menus + interior view preview

Prerequisite: G2 source and all prior Asset Visibility/G1/G2 changes. Input is the exact certified G2 commit `9c4689a7419e93223089c58275555baa327e1e8a`; reject conflicting local edits. No changes to PCC or shipping FPS/cockpit controls.

## Delivered
- NativeWindow explicitly switches to editor navigation only while a Shipyard workspace is active: MMB orbit, Shift+MMB pan, wheel dolly, RMB contextual click; gameplay retains RMB orbit/MMB pan. Empty/new Studio documents frame the origin in orbit mode, not FPS/free-fly.
- FILE/EDIT/VIEW/HELP are clickable menus with a **small subset of real commands**. File exposes New Empty, Save Blueprint, Generate; Edit Undo/Redo/Delete; View cycle view/grid/frame/reset; Help guidance/stats/recover assets. Not a complete desktop File menu or dialog framework. The existing top toolbar keeps ASSETS and RESET UI.
- Live viewport mode selector Exterior / Cutaway / Interior / X-Ray. Entering Interior workspace defaults to Interior-only. The existing structural program generation and floor/wall/door/airlock/hatch draft commands, element selection/removal and paint livery/primary/secondary/trim/finish/decal actions are now visible in the tool rail/properties. A shell is generated from the actual authored module recipe using the runtime ShipInteriorLayoutSystem::Plan and cached until the recipe structure changes. The preview draws source-derived surfaces with a visually omitted roof; the source collision and geometry remain unchanged. Cutaway/X-Ray exterior is a wire overlay; this is NOT exact mesh Boolean subtraction or material-transparent X-ray, and unsupported cavities display NOT READY, not fake geometry.

## Explicitly outstanding before calling milestones 01–04 complete
- Full File Open/Save As/import/export/recent & native OS picker, complete menu actions and popup occlusion/accessibility verification.
- Persistent per-project document authority and modeling/decals/doors/interior edit round-trip (current save blueprint cannot certify those). Structural generation now refuses to wipe dirty interior edits; no structural save bridge is claimed. A successful blueprint save clears assembly-only dirty state but intentionally retains the dirty guard if unsaved interior/model drafts exist; failed blueprint exports no longer produce a misleading success state.
- Full-width material paint workflow, true asset list/grid toggle+scrollbar & full docked panel-body scrolling.
- Blender keyboard numpad view shortcuts, view pie, optional fly/walk control binding and orthographic projection parity. No game FPS/cockpit changes in this patch.
- Exact imported-mesh boolean cutouts, rendered/selected room partitions, door leaf/hinge/animation, interior collision/nav/save validation, roof/individual deck toggles.
- Windows Full Quality Gate and hands-on visual acceptance remain required. Do not commit as Stage 01–04 COMPLETE solely from green static checks.

## Acceptance
1. Start standalone empty: orbit with MMB around origin, Shift+MMB pan, wheel zoom; RMB short click remains context. Enter gameplay and verify its input profile is unchanged.
2. Use FILE > Generate, VIEW > Cycle View. In Exterior see full hull; in Cutaway and Interior see room shell for supported modules. Unsupported shell must show NOT READY and never fake walkable interior.
3. Open FILE/EDIT/VIEW/HELP and exercise their enabled commands. Check previous ASSETS and RESET UI recovery buttons.
4. Run authoritative Windows PCC Full Quality Gate; manually inspect at 1280x768 and 1920x1080.

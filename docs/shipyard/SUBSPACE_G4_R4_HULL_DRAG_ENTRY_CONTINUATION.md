# G4 R4 — cumulative first-hull drag-entry visibility correction

**Delivery:** one PCC root-drop `.patch`, no PowerShell/helper ZIP/extraction. Exact published GitHub HEAD and prerequisite: `cf5107e4b36881ebfbcb46325a31558e84979597`. This R4 contains all five R3 payloads (including prior R2 corrections) plus this document, not an entire source rollup. Use **R4 instead of R2/R3** if none of those patches were applied. If a predecessor was applied but is uncommitted, let PCC validate the superseding overlay; if source/HEAD disagrees, stop rather than forcing it.

## Verified bug and change

R3 protected an already-active ghost and restored exterior on placement, but it did not cover **before** dragging if Build had already been manually set to Interior-only. Inspection of both `NativeGameApplication.cpp` input routes confirms that a catalog card invokes `Activate(SelectModule, value)` on primary press, followed by `BeginCatalogDrag` only after the distance threshold. R4 makes successful `SelectModule` call the shared `ForCatalogPress` policy. In non-Interior workspaces, Interior-only becomes Exterior **on catalog press**, before the ghost is created. The dedicated Interior workspace retains the intentional inspection view, and Cutaway and X-Ray are not overwritten. Invalid selection does not change the view. A new status line explains the automatic Exterior transition.

This patch changes only the existing G4 view-policy header, active Shipyard command router, existing focused C++ test, plus the R2/R3/R4 handoff documents. It does NOT overwrite the renderer, monolithic builder, native input loop, camera, asset imports, gameplay, fleet, CMake or PCC. No fake hull or mesh-loading bypass is added.

## Verification limits

The included test extends the previous 24 focused assertions with seven catalog-press policy assertions. The two production pointer routes were checked for SelectModule-before-BeginCatalogDrag ordering. This does not mean the OpenGL window or Windows PCC Full Quality Gate has run. A future direct `BeginCatalogDrag` call that bypasses SelectModule is not covered by this focused correction; current two production mouse callers both select first. If the Outliner receives the hull but EXTERIOR still renders blank, inspect `SHIPYARD_RUNTIME_NOT_READY`, `Could not load certified Shipyard module`, and visual/recipe asset readiness in the debug bundle rather than bypassing content gates.

## Visual acceptance after returning home

1. Open Studio blank; in ASSEMBLY manually cycle view to INTERIOR; press-and-drag the *first* hull card; the view should switch to EXTERIOR on press and the ghost should appear after the drag threshold, before release. Stage, PLACE, and select via Outliner: finished hull appears.
2. Switch INTERIOR workspace -> ASSEMBLY; INTERIOR -> TEST; keyboard workspace cycling; verify hull remains visible. Verify Cutaway/X-Ray are retained when deliberately selected.
3. Repeat catalog press and drop in docked/in-game Shipyard. Confirm cockpit, on-foot, controls and asset safety unchanged.
4. Run PCC Full Quality Gate, then hands-on viewport check. Commit/push only after both pass. This patch is source-verified only until then.

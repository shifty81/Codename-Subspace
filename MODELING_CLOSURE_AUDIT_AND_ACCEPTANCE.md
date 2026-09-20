# Codename Subspace — Studio modeling closure audit / S15D next-ten-pass cumulative handoff

Published source authority audited: `acabf9c5df9e1c382eb87bc1b7264354355828f9`, certified by `QG-20260919-213504-full-ae027bdc`.

The user had **not applied** the earlier `Subspace_S15D_Bulk_A_Audit_And_Repair_20260919.zip`. This handoff therefore supersedes it and includes its fixes cumulatively. Apply only this modeling-closure handoff.

## Source-proven problems closed by this tranche

- Delete is emitted by Win32 but standalone Studio routes it only in Build. Model and Interior already have real delete commands, so Delete is normalized to active document context.
- Model delete / duplicate / modifier changes were omitted from authoring history. They now participate in the existing undo authority.
- Win32 can lose focus/capture without LBUTTONUP. Studio now receives a one-shot capture-loss event and rolls back the active preview transaction instead of committing it later.
- Gizmo presentation and pointer-down used different snapshots. Both now consume one occlusion-filtered snapshot; the XYZ tip marker receives a larger hit target, nearly camera-aligned shafts are rejected, and edge clipping no longer removes a reachable gizmo just because the old fixed 78px margin was crossed.
- Model recipes already had real primitive geometry and transform methods, but no viewport bridge, direct primitive picker, model gizmo transaction, or Model Outliner authority. Those connections are added.
- Model Save was explicitly blocked and “any primitive exists” was treated as permanently unsaved. A versioned `.subspace_studio` model document codec now persists model recipes transactionally and tracks saved revision separately from geometry existence.
- Model-only recovery previously failed. It now writes typed model recovery instead of pretending `.subspace_ship` can preserve model data.
- Vertex / Edge / Face names existed without topology-element selection. This handoff deliberately stops presenting those names as functional. Object/primitive modeling is the implemented modeling contract in this tranche; mesh-element editing remains the advanced V26 topology milestone.

## Ten passes included

| Pass | Implemented scope | Acceptance |
|---|---|---|
| S15D-01 | Active-context Delete | Delete removes selected module / model primitive / interior element as appropriate. |
| S15D-02 | Model undo normalization | Delete, duplicate, modifiers and transforms create recoverable authoring history. |
| S15D-03 | Pointer capture rollback | Alt-tab/capture loss cancels active transform and never commits a stale drag. |
| S15D-04 | Gizmo marker/picking normalization | Presented XYZ marker is the marker that can be picked; tip easier to click; blocked handles not pickable. |
| S15D-05 | Model primitive selection | Viewport and Outliner select the same primitive index. |
| S15D-06 | Model transform transactions | Move / Rotate / Scale support preview, commit, cancel and one undo unit. |
| S15D-07 | Native model viewport scene | Model recipe bakes to existing CanonicalAsset geometry and renders in the Studio viewport with selected-object highlight. |
| S15D-08 | Model tool workflow | SELECT / MOVE / ROTATE / SCALE / ADD BOX / DEL rail plus selected/all framing. All primitive families remain reachable through Add Shape cycling. |
| S15D-09 | Model document persistence | `.subspace_studio` verified atomic save, Save As, open/reopen and revision dirty authority. |
| S15D-10 | Recovery / gates / truthfulness | Model-only recovery, codec CTest and source gate; fake topology selection removed from visible workflow. |

## Important architecture decisions

1. Existing `ShipyardModelingSystem` remains the non-destructive model recipe and canonical geometry authority; no second modeling database is introduced.
2. Existing `ShipyardBuilderSystem` remains selection/undo/command authority. Model transforms gain a parallel transaction state only because the existing ship placement transaction stores `VisualModulePlacement`, not model primitives.
3. Existing `NativeBattlefieldRenderer` remains viewport renderer. The new model bridge consumes `BakeCanonicalAsset`; it does not invent a second renderer.
4. `.subspace_ship` remains the gameplay ship blueprint. `.subspace_studio` is a typed Studio model-asset document because forcing model data into the ship blueprint was already known to lose it.
5. Unsupported Bevel/Boolean-style modeling modifiers remain honest validation failures; viewport preview does not disappear merely because one is stored. No unsupported modifier is silently claimed as evaluated.

## Build/test truth

This package has passed packaging/integrity checks, Python installer syntax, payload tests, and an independent C++17 syntax compile of the new model document codec against the available Subspace source headers. It has **not** been Windows-built or hands-on tested in the user's checkout. The project-owned PCC Full Quality Gate and interactive Studio test remain authoritative.

## Hands-on acceptance after apply

1. New empty Studio -> Model -> Add Box. Box must be visible immediately.
2. Click the Box in viewport and Outliner. Selection/highlight must agree.
3. Select Move and drag X, Y and Z tips. Marker must be easy to acquire and only that axis changes.
4. Rotate and Scale each axis. Escape cancels; mouse-up commits. Ctrl+Z restores the prior primitive state.
5. Delete removes the selected Box; Ctrl+Z restores it.
6. Add a second shape using the primitive selector / Create Shape and verify Outliner selection and independent transforms.
7. F frames selected model object; Home frames the model collection.
8. Ctrl+S / Save As to `.subspace_studio`, close, reopen, verify primitive types, dimensions, positions, rotations and supported modifiers survive.
9. Create an unsaved model-only edit and choose close-with-recovery; verify a unique `.subspace_studio` recovery is produced.
10. Retest Assembly gizmo tips, Delete, Undo and G02 Don't Save so model closure does not regress ship authoring or close behavior.

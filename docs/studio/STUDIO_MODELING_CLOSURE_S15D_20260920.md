# Subspace Studio — cumulative object-modeling closure

Baseline: `acabf9c5df9e1c382eb87bc1b7264354355828f9` / `QG-20260919-213504-full-ae027bdc`.

This is cumulative: it includes the previously un-applied S15D Bulk A interaction fixes. Do **not** apply Bulk A first.

## Ten-pass closure

1. Normalize Delete to the selected object in Build / Model / Interior.
2. Complete Model mutation undo history.
3. Cancel active pointer transactions on Win32 focus/capture loss.
4. Make rendered and picked gizmo snapshots identical; enlarge the XYZ tip hit target and reject degenerate shafts.
5. Add direct model-primitive selection and Model Outliner rows.
6. Add Model Move / Rotate / Scale transaction ownership with cancel/commit and one undo unit per gesture.
7. Render authoritative non-destructive Model recipe geometry in the native viewport, including selected-object highlight.
8. Make the Model tool rail expose Select / Move / Rotate / Scale / Add Box / Delete and add real selected/all framing.
9. Add typed `.subspace_studio` model documents with transactional verified save/reopen and revision-based dirty state.
10. Add model-only exit recovery, source/codec tests, and truthful topology-mode behavior.

## Deliberate boundary

Object/primitive modeling is the implemented contract in this closure. The existing enum names Vertex / Edge / Face did not have real topology selection or editable mesh-element authority. This pass stops presenting them as implemented. Advanced mesh topology editing remains the later V26 feature rather than being faked here.

## Apply

After PCC installs this payload:

```
tools\studio\ApplyModelingClosure.cmd --check
tools\studio\ApplyModelingClosure.cmd --apply
```

`--check` is read-only and verifies exact Git blobs for every edited pre-existing file. `--apply` creates a recovery receipt and atomically modifies source. It does not build, commit, push, stash, reset or force anything.

Then run the project-owned Full Quality Gate. Hands-on acceptance: new empty Studio -> Model -> Add Box -> visible box -> Select -> Move X/Y/Z -> Rotate -> Scale -> Delete -> Undo -> Save `.subspace_studio` -> close -> reopen -> geometry/transforms preserved. Also retest Assembly gizmo tips and G02 Don't Save.

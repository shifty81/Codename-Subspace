# Codename Subspace — one cumulative PCC intake (Hollow Hull + GUI access recovery)

Base: clean commit `02615ebf892fd95bd5d8d356ed7651d9800e20ed`, which includes overlay Slice 1A through R2. This package replaces the *multiple uncommitted* Hollow Hull Slice 1, Slice 2, R1 and initial GUI-access recovery into **one latest-file payload**. Do not also install those earlier Hollow Hull patches after this one. It includes the final contents of overwritten files, not nested .patch files or a second PCC.

## Implemented here

- Slice 1: derived interior shell surfaces from the existing carve plan, paired source/geometry regression.
- Slice 2: runtime shell renderer/traversal integration, passage threshold, collision and visual-only cutaway, runtime test/source guard.
- Floor-flush three-sided doorframe test correction (the last reported Windows Full Gate failure).
- Real pointer wheel navigation over the current Asset Browser overlay; same responsive visible card/page geometry shared by drawing, controls, and wheel; explicit next/previous actions and `N / M` visibility indicator.
- Click-to-focus ASCII catalog search + Backspace, Enter and Escape, explicit clear, filter matching actual runtime inventory; model workspace exposes existing shape commands and honest disabled Publish rather than pretending it works; contextual model properties; Outliner scroll and prev/next, first-run action guidance.
- Asset controls cannot receive clicks outside the clipped overlay; overlay test no longer dereferences pointers into destroyed temporary layout vectors.
- Adds `SubspaceShipyardGuiRecoveryTests`: first/last card reachable across width/density/resize, empty and single results, navigation bounds.

## Not completed — do not mark Shipyard GUI functionally certified

- Native File/Edit/View/Help menu commands and typed F3 command search remain unimplemented (F3 is labeled a noninteractive index).
- True scrollbar with draggable thumb, keyboard PageUp/PageDown/Home/End, paste/Unicode and complete source/favorites/compatibility filtering are not in this package.
- Model primitives are not rendered/picked or saved as derived game assets; mesh component separation, pivot editing, animation clips and operating doors are pending. Publish remains disabled.
- The full inspector still clips controls in undersized panels; panel body scrolling, hierarchical object selection, per-mode layout presets, full blueprint roundtrip and GUI screenshot automation are pending.
- No new Nullharbor/NovaForge gameplay systems are merged. No claim of actual Windows visual acceptance.

## Install / verify

1. Confirm active PCC `main` still at `02615eb` and no newer manual source edits were made to the affected files. If patches were already applied, PCC intake may overwrite those paths with their consolidated final revisions; use patch receipt/preflight review first. Do not force, reset, or auto-stash.
2. Drop this single `.patch` unextracted beside `SubspaceTools.cmd`, restart PCC and approve normal transactional intake. Do not extract fallback ZIP on top of the repo.
3. Run option 1 FULL QUALITY GATE; stop and send its debug bundle on failure. Then option 3 Run & Play, check all asset entries including the last by wheel and arrow, search/clear, Model tool visibility, Outliner scrolling, float/re-dock and stable full-size viewport.
4. Commit/push only after authoritative GREEN and *visible* workflow acceptance. When GREEN is achieved, future GUI patches must target the new committed HEAD, not stale `02615eb`.

This package is cumulative for the listed completed work, **not the entirety of the still-unimplemented GUI recovery roadmap**. Feature freeze remains in force until those UX acceptance items are implemented and tested.

# Subspace G4/G5 — R1 focused safety continuation

This is a **review/rebase source bundle**, not a `.patch` and NOT a project-root drop. The bundled builder produces a baseline-locked PCC `.patch` only after verifying the precise prior G4/G5 foundation file hashes and a clean working tree for every affected file. It resolves the local-commit/remote-commit mismatch without disabling PCC Git preconditions.

## Implemented changes in this delta

- Editor camera: retargeting an orbit pivot synchronizes yaw/pitch/distance to avoid a jump on the next orbit; framing from optional Fly navigation uses the actual look direction rather than the obsolete pivot vector; pole clamping keeps it finite; Top/Bottom axis orientation uses deterministic yaw.
- Existing tactical mode: conservatively disables flight, weapon, scanner, vector, docking and interior inputs in `RuntimeControlContextSystem::Build` while tactical UI owns the pointer. **This does not add physical seat authorization to the old tactical entry path.**
- Seat foundation: physical ship command consoles default to on-foot return; pilot seats require cockpit origin; a caller can explicitly capture its prior view. Re-entering an active session never rewrites origin; exiting can return the captured origin. Duplicate include removed.
- Tests: expands already-registered camera and authoring/runtime test executables; no older CMake or NativeGameApplication source is overwritten.

## Boundaries — NOT complete

- NativeWindow/NativeGameApplication are not changed: numpad shortcuts, orientation gizmo, real orthographic projection, Fly/Walk interaction and framing call sites are **not** wired by this delta. `FrameShipyardView` must switch from `Reset` to the orientation-preserving method as a separate correctly rebased change.
- New seat system is **not** linked to actual seat/world identity or order dispatch. The legacy tactical overview is preserved; terminal UI expansion stays deferred by user request.
- G3 outstanding: proper Save/Open/Save As, dirty model/interior/door/decals round-trip, collision parity, real cutaways, asset scrolling and full GUI acceptance.
- Seamless world streaming and planetary traversal remain outside this pass.

## Baseline / certification

- Prior foundation source: `Subspace_Studio_G4_G5_Camera_Seat_Foundation_5040815.patch`. The builder checks the prior foundation's exact 6 source hashes. If any file has diverged, **it refuses to generate a patch**. The active Git HEAD is captured at generation time into the `.patch` baseline.
- Local camera/compositor target: 61/61 PASS. Independent G5 safety smoke: 11/11 PASS. Authoring/runtime test file: C++ syntax PASS. Full engine build was attempted but did not complete before execution timeout. Windows PCC Full Gate and in-game visual acceptance not run here.
- Once builder succeeds, copy its generated `.patch` into the Subspace repository root unextracted, run PCC startup intake and Full Gate, then manually check camera and tactical-mode input. Do not commit without Windows green and hands-on acceptance.

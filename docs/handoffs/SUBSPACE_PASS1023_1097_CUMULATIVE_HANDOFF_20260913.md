# Codename Subspace Pass1023-1097 cumulative handoff

## Baseline

- GitHub `main`: `9440a5254ee80ab901f5c2d67e6a5bc14ae8423a`
- Certified gate: `QG-20260913-092557-full-54aaeb65`
- Patch policy: `forge.project-update-policy.v1`
- Intended intake: project root `.patch` -> restart PCC -> explicit **Y** -> option **1 Full Quality Gate**.

## What this cumulative patch is intended to change

1. Restore governed Various Planets surfaces/cloud layers and fail certification if the licensed source/runtime pack cannot be verified.
2. Replace advisory XS/S/M/L/XL behavior with actual physical module and ship-class sizing authority.
3. Replace palette-release-immediate-attach with a staged Blender-like move/rotate/scale/snap/confirm transaction.
4. Prefer measured flat contact surfaces and dense surface snap fields over bounding-box-only socket guesses.
5. Standardize interiors and semantic props against the canonical 1.80 m player-scale envelope.
6. Establish one cohesive exterior/interior bake plan while retaining editable module source.
7. Normalize cockpit FPS, on-foot FPS, Remote Fleet Command and Shipyard/dev camera roles.
8. Move shield presentation toward one hull-conformal water-like layer with bounded impact ripples and fleet-scale LOD.
9. Promote one minimal dark UI/docking authority across runtime/editor surfaces with resize, float/redock and panel opacity.
10. Make Blender generator tooling a parity client of the same generator request registry used in game.

## Manual acceptance after GREEN

Prioritize visual/runtime checks that unit tests cannot prove:

- Planets: imported surface detail and cloud layers visibly return; gas/ocean/rocky examples should no longer resemble plain fallback spheres.
- Ship class/size: changing class and legal XS-XL target produces clearly different real hull dimensions; larger classes must not normalize smaller.
- Palette workflow: drag a module, release, confirm it remains staged/unattached, move/rotate/scale it, cycle snap candidates, then explicitly attach or cancel.
- Surface snapping: test broad flat hull faces and verify several usable snap locations appear across the face rather than one guessed endpoint.
- Model workspace: Add Shape, resize it with mouse/tools, assign a semantic purpose (seat/console/storage/door/etc.), validate and publish/reject based on character fit.
- Interiors: verify doors/corridors/seats/consoles feel compatible with the player model and that exterior overlap does not imply whole hidden modules blocking the intended interior.
- Shield: inspect one ship closely for a single conformal shell appearance, then trigger a shield hit/debris impact and look for localized ripple behavior; also test a busy asteroid/ship scene for performance.
- UI: resize dock regions, float/redock a non-critical panel, adjust opacity, verify dark scrollbars/popups/inputs and ensure the central viewport remains usable.
- Controls: normal ship control should remain first-person/cockpit oriented; normal interior movement should remain on-foot FPS; tactical overhead should read as Remote Fleet Command, not the default embodied camera.
- Blender: confirm SubspaceShipyard exposes the same generator domains/seed/profile/class/size/role request surface and writes `subspace.generator-request.v1` rather than producing unrelated local generator logic.

If any of these fail visibly despite a GREEN gate, package a debug bundle plus screenshot/video evidence before promoting another feature tranche.

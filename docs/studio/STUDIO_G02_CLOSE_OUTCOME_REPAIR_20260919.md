# G02 — Studio intentional close outcome repair

**Status:** source change requires an explicitly invoked guarded second stage; G01 readiness audit and G02 have different responsibilities. This is an incremental development handoff, not the final single-stage cumulative root patch and not a claim of Windows GREEN.

## Source baseline

Published `shifty81/Codename-Subspace@14a5ca8b6039868b7968b3b00018307c6306e3ca`. G01 may already have added read-only tools. The guarded script requires the EXACT published blob SHA for each of the three C++ files. It checks all three before writing any one. Refuse local drift rather than overwriting prior user changes or previously applied alternate patches.

## Exact issue

S13's Studio-only WM_CLOSE interception verifies blueprint recovery before releasing the window. The later Run exit path examines still-dirty Model/Interior/override work and returns exit code 7 even when the user explicitly acknowledged partial recovery. PCC treats exit 7 as application failure, creating the reported misleading debug bundle. This is **not proof** that model/interior drafts have been saved: blueprint recovery is still partial.

## Repair

- Introduce `StudioExitOutcome` with explicit `Clean`, `Saved` and `UserConfirmedPartialRecovery` vs `Unconfirmed`.
- Only record a confirmed outcome after the close policy actually authorizes closing. `Cancel`, save failure and recovery failure never record user consent or close the window.
- Keep unmatched/unconfirmed exits with dirty unsupported drafts as exit 7, recovery errors as exit 6. Honor a confirmed partial recovery as normal process exit 0, leaving the existing data-loss warning visible.
- Emit one structured `STUDIO_EXIT_RESULT schema=subspace.studio-exit.v1` line in logs with outcome, recovery verification, unsupported-drafts presence, process code. This is a log event, NOT a durable comprehensive document-save receipt or a project GREEN gate.
- Preserve the S13 Studio-only Windows close guard and game window behavior. Do not change the model/interior serializer (G03/G04), fake a recovery file, or disable PCC failure handling.
- Extend the existing CTest `studio_close_policy_tests.cpp` with consent vs unconfirmed and precedence assertions.

## Apply — two explicit steps, no hidden mutation

1. Place the ZIP-container `.patch` alone at the Subspace root; approve/apply with the project's PCC. This stage only installs a new policy header, verified repair script, tests/doc and launcher. It does **not** alter the actual exit path yet.
2. From the same root run `tools\studio\ApplyG02.cmd --check` then `tools\studio\ApplyG02.cmd --apply`. No Full Gate or startup process invokes this script automatically. `--apply` verifies three current Git blob SHAs, every text anchor, backs up all three source files in `.subspace/recovery/studio-g02/`, atomically replaces each file, verifies final hashes and writes an explicit receipt. A detected write failure rolls back. Do not use `git reset` or force application on drift.
3. Run the existing Full Quality Gate after the actual source modification. Then manually test **clean**, **Yes save**, **No/acknowledged partial recovery**, **Cancel**, **Alt+F4**, first-save chooser cancellation and actual recovery failure. Send any new debug bundle on failure. Full gate alone does not certify these GUI interactions.

## Not fixed in G02

Model BOX visibility, full Studio document persistence, complete model/interior autosave, GUI shell, move gizmo and asset material resolution remain separate G03+ tasks. `No` in the current dialog means **close with blueprint recovery**, not a no-file discard. UI copy and full editor-native save/recovery are reserved for G04. No commit or push performed by this package.

## Cumulative roadmap

Preserve all earlier 190 planned passes, multi-domain generation (ship, station, vehicle, shuttle, interior, detailing, weapon, character, planetary facility/world, solar system), kitbash source census, Blender/Cortex/Forge integrations, and certified PCC authority. G02 neither renumbers nor completes those passes.

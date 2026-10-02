# NullHarbor R179 — Planetary Command preimage reconciliation audit

**Date:** 2026-10-01  
**Development identity:** Codename Subspace  
**Destination product identity:** NullHarbor  
**Authoritative repository baseline:** `shifty81/Codename-Subspace` `main` at `8484a081f8d60be980d3aac300ba2f1f9397cd75` / `QG-20260930-143924-full-53697b5b`  
**Prerequisite working milestone:** R178 FPS + Universal Interiors + Strategic Command Convergence

## Failure reproduced

The Windows PCC Full Quality Gate reached the pre-CMake R158-R177 Planetary Command materialization hook and failed with:

`overlay input action: expected one certified preimage in engine/include/input/InputState.h; exact=0 indentation-normalized=0`

This was not a C++ compile failure. R178 legitimately appended new semantic control actions to `InputState.h`, so the older R177R2 text transformer no longer found either of the two historical certified textual preimages. Replaying or broadening that transformer again would make every later input milestone another migration-preimage maintenance problem.

## Reconciliation decision

R179 retires the historical 17-transform text replay. The compatibility entrypoint `scripts/subspace_planetary_command_r158_r177_apply.ps1` now performs **canonical semantic verification** only. It verifies the current planetary command authority and writes a materialized migration marker; it never rewrites a drifted modern source file to resemble an older preimage.

The migration is therefore reconciled by promoting its intended behavior into current source, not by weakening fail-closed source matching.

## Canonical Planetary Command authority

R179 upgrades the existing planetary-industry hex foundation rather than adding a second planetary simulation.

- Stable sector identity is `planetId + axial(q,r)` through `PiSectorIdentity`.
- Sector progression is `Unsurveyed -> Surveyed -> Claimed -> Developed`.
- Claims are owner-bound and contiguous. After an owner establishes territory, a new claim must border an owned claimed/developed sector.
- Player-facing infrastructure placement uses `PlaceGoverned` and requires developed, owned territory.
- The historical low-level `Place` path remains for compatibility with old tests/data; it is not the new player-facing authorization path.
- Globe and sector projections share the same projection routine used by rendering and pointer hit testing.
- Six overlays are authoritative: Resources, Ownership, Industry, Logistics, Power, Hazard.
- The sector inspector exposes stable identity, state, resources, hazard, buildability, claim frontier and local installations.
- P opens the existing `PlanetaryManufacturing` workspace, whose player-facing title is now **Planetary Command**. The internal enum name remains compatibility-stable.
- LMB selects. RMB selects/pins a sector inspector; RMB on empty command space toggles Globe/Sector projection. F5 cycles overlays. Enter advances the selected sector. Shift+Enter deploys the recommended governed installation.

## R178 compatibility

R179 does not reorder historical or R178 `InputAction` values. `PlanetaryCommandCycleOverlay` is appended after the R178 fleet-command actions. R178 fleet strategy, FPS control, universal interior, donor-kit and workstation authorities remain intact.

## Certification

R179 must pass:

1. `tools/control/tests/test_r158_r177_planetary_command_source.py`.
2. `nullharbor_r179_planetary_command_convergence.cmake` static certification.
3. Pass361-400 command/galaxy tests including the R179 territory/projection cases.
4. Pass316-335 stabilization suite, preserving the R178 convergence checks.
5. Master native engine tests.
6. `subspace_game` native application link, proving the live application + renderer changes compile together.

The Windows PCC Full Quality Gate remains the final machine-authoritative certification after application.

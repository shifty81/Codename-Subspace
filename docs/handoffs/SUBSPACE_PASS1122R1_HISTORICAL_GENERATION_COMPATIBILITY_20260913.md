# Pass1122R1 — Historical Generation Compatibility Closure

## Why this repair exists
Pass1098-1122 intentionally changed generation semantics: `GENERATE` is deterministic, impossible class/topology requests fail closed, and only explicit `REROLL` mutates the seed. The Windows Full Gate proved the new Pass1098-1122 tests and source gate were green, but two historical Shipyard tests still asserted the retired pre-normalization behavior.

The same gate also exposed one migration issue in `ShipyardBuilderSystem::Validate()`: class/topology generation envelopes were being treated as hard errors for legacy/manual authored ships. That unintentionally disabled APPLY for otherwise structurally valid historical/manual designs.

## Repair
- Generated lineage (`CLASS_SIZE_ENVELOPE_V3` / `FACTION_CLASS_HULL_ROLE_V1`) keeps class/topology envelope failures as hard errors.
- Legacy/manual authored ships keep class-envelope mismatches as review warnings, preserving edit/refit compatibility while migration continues.
- Pass428 historical regression expectations now certify the current deterministic contract:
  - GENERATE does not change the seed.
  - generation either returns a certified class-correct result or fails closed without replacing the prior ship.
  - the current structurally valid ship remains eligible for APPLY.

## Validation
On the reconstructed current source line:
- `SubspaceShipyardRegressionTests`: 9/9 PASS.
- `SubspaceShipyardInteractionTests`: 9/9 PASS.
- Pass1098-1122 normalization tests: 25/25 PASS.
- Pass1098-1122 source gate: PASS.
- Modified builder and tests compile with `-Wall -Wextra -Werror`.

This repair does not weaken the new generator. It only separates **generated certification** from **manual/legacy design migration** and updates stale historical expectations.

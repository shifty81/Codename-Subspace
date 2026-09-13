# Codename Subspace — Pass1097R2 Historical Certification/UI Repair

Date: 2026-09-13
Baseline Git HEAD: `9440a5254ee80ab901f5c2d67e6a5bc14ae8423a`
Requires: cumulative Pass1023-1097 and Pass1097R1 already applied in the working tree.

## Why this repair exists

The Pass1097R1 Full Gate proved the planet source/materializer recovery path: 27 images were materialized from `various_planets.glb` and the planet texture/cloud fidelity gate passed. CMake configure/build also completed successfully. CTest then exposed nine historical regression expectations that had not been migrated to the new Pass1023-1097 authorities.

This repair updates the implementation where the regression was real and migrates historical gates only where the old token/label itself was obsolete.

## Runtime/UI repairs

- Restores the project-wide minimum clickable control height to 38 px through `SubspaceUiTheme`, so runtime/game UI inherits the readability contract instead of overriding it locally.
- Keeps the Blender-style Shipyard tool rail at a practical 52 px minimum click width and advertises `[Q]`, `[W]`, `[E]`, `[R]` shortcuts directly.
- Restores visibly scaled 4K Shipyard fallback panes while preserving the large central viewport.
- Replaces the stale player-facing `no landing path exists` message with the current truthful architecture status: seamless surface landing is the architecture but is not yet runtime-wired.

## Historical gate migrations

- Pass513/Pass746 shield gates now certify `ConformalShieldSurfaceSystem` policy values instead of requiring retired hard-coded renderer constants.
- Pass523 interior-context gate now certifies `InteriorOnFoot` + `ON FOOT / FIRST PERSON`, matching the FPS-first embodiment authority.
- Pass790 generation gate now certifies hard `ShipClassGenerationAuthoritySystem` class envelopes, class-compatible module filtering, requested-size clamping and physical measured-length stamping instead of obsolete marker strings.
- Pass595-614 regression expectations now align the Frigate/Battleship envelope preferred module tiers with the canonical component profiles while retaining the same hard 40-90 m and 450-750 m physical hull ranges.

## Validation performed before packaging

- Pass513-522 source gate: PASS.
- Pass523-532 source gate: PASS.
- Pass746 shield/performance source gate: PASS.
- Pass790 Shipyard authority source gate: PASS.
- Pass902 landing-claim migration slice: PASS.
- `ShipyardBuilderSystem.cpp`: strict C++17 compile with `-Wall -Wextra -Werror`: PASS.
- `SubspaceUiFramework.cpp`: strict C++17 compile with `-Wall -Wextra -Werror`: PASS.
- 4K layout acceptance slice: `uiScale=1.6`, `leftWidth=806.4`, `rightWidth=1017.6`: PASS.
- Game UI readability slice: `minimumContrast=0.72`, `clickableRowHeight=38`: PASS.
- Ship class preferred-size/physical-envelope slice: Frigate=S/40-90m, Battleship=L/450-750m: PASS.

The authoritative Windows PCC Full Gate remains the final certification because the portable archival checkout does not contain every post-rollup source file required to rebuild the entire 92-target current graph.

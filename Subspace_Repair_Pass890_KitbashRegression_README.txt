Codename Subspace - Pass890 Kitbash Regression Recovery

Purpose
-------
Surgically repair the Sep 11, 2026 regression affecting:
- Pass613 ship-class structural envelopes
- Pass638 faction/class/hull-family/role lineage

The script uses the existing certified GREEN source snapshot from the 17:54 gate.
It backs up the current implicated source files before changing anything.

Default run from the CodenameSubspaceNullharbor repository root:
  pwsh -NoProfile -ExecutionPolicy Bypass -File <path-to-script>

Or specify the repo explicitly:
  pwsh -NoProfile -ExecutionPolicy Bypass -File <path-to-script> -Root "C:\Users\Shifty\Desktop\CodenameSubspaceNullharbor"

Repair strategy
---------------
Stage A restores only:
- engine/src/ships/ShipClassRoleSystem.cpp
- engine/src/ships/FactionShipDesignSystem.cpp

It rebuilds and runs only the two failing CTest targets.

Only if those tests still fail, Stage B additionally restores:
- engine/src/ships/ShipPcgRuntimeClosureSystem.cpp

If targeted validation still fails, the script automatically restores the pre-repair
versions. If targeted validation passes, it runs the normal Subspace full quality gate.

No global rollback of Pass791-890 is performed.

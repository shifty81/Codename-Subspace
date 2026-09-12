Codename Subspace Pass891 Semantic Authority Repair
==================================================

The 2026-09-11 18:55 debug bundle still fails exactly three assertions:

  Pass613 - Frigate must normalize to XS / 40-90m
  Pass613 - Battleship must normalize to XL / 450-750m
  Pass638 - generated variants must preserve faction/class/hull-family/role lineage

79 of 81 CTest targets pass, and both associated source gates pass.

The earlier restore caused ShipClassRoleSystem.cpp, FactionShipDesignSystem.cpp,
and ShipPcgRuntimeClosureSystem.cpp to rebuild, but the same assertions remained
broken. That is consistent with the 17:54 incremental GREEN certifying stale
object code while its source snapshot already contained the semantic regression.

This repair performs a forward semantic repair. It does NOT restore an old snapshot.

It surgically replaces only:
  ShipClassRoleSystem::Envelope
  ShipPcgRuntimeClosureSystem::ApplyLineage
  FactionShipDesignSystem::VariantPreservesLineage

It backs up current source under:
  artifacts/recovery/pass891-semantic-authority-<timestamp>/pre-repair

Then it forces configure/build, runs the two failing CTest targets, rolls back
automatically if either still fails, and runs the normal Full Quality Gate if
targeted validation passes.

Keep this pack OUTSIDE the repository root to avoid root-audit clutter.

Run:
  pwsh -NoProfile -ExecutionPolicy Bypass -File .\Subspace_Pass891_SemanticAuthorityRepair.ps1 -Root "C:\Users\Shifty\Desktop\CodenameSubspaceNullharbor"

Targeted tests only:
  pwsh -NoProfile -ExecutionPolicy Bypass -File .\Subspace_Pass891_SemanticAuthorityRepair.ps1 -Root "C:\Users\Shifty\Desktop\CodenameSubspaceNullharbor" -NoFullGate

No tests are weakened or modified.

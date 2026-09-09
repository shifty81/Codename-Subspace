param()
$ErrorActionPreference = "Stop"
$root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$required = @(
  "engine/include/client/ClientAppScaffold.h",
  "engine/include/flight/ShipFlightControl.h",
  "engine/include/effects/ThrusterParticleSystem.h",
  "engine/include/ships/ShipPartGeneration.h",
  "engine/include/home/HomeShipBuilderBay.h",
  "engine/include/travel/RailRouteEncounterModel.h",
  "engine/include/roguelite/RogueliteLoopModel.h",
  "engine/include/migration/CppConversionBacklog.h",
  "engine/include/normalization/ProjectNormalizationPlan.h",
  "engine/include/content/ContentManifest.h"
)
$missing = @()
foreach ($rel in $required) {
  if (-not (Test-Path (Join-Path $root $rel))) { $missing += $rel }
}
Write-Host "SUBSPACE PASS90-99 STATUS"
Write-Host "Root: $root"
if ($missing.Count -eq 0) {
  Write-Host "RESULT: PASS - all Pass90-99 headers are present" -ForegroundColor Green
  exit 0
}
Write-Host "RESULT: FAIL - missing files" -ForegroundColor Red
$missing | ForEach-Object { Write-Host "  $_" -ForegroundColor Red }
exit 1

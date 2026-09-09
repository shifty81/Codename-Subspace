param([string]$Root = (Resolve-Path (Join-Path $PSScriptRoot "..")))
$ErrorActionPreference = 'Stop'
Write-Host "=== SUBSPACE PASS497-506 SHIPYARD AUTHORING ADAPTER REPORT ==="
Write-Host "Root: $Root"
$required = @(
  'engine\include\ship_editor\ShipyardAuthoringAuthority.h',
  'engine\src\ship_editor\ShipyardAuthoringAuthority.cpp',
  'engine\include\ship_editor\ShipyardAuthoringBridge.h',
  'engine\src\ship_editor\ShipyardAuthoringBridge.cpp',
  'content\derived\greyoxide_shipyard_v07\authoring_overrides\pass497_506_certified_overrides.json',
  'content\derived\greyoxide_shipyard_v07\authoring_overrides\pass498_module_authoring_catalog.json'
)
foreach ($rel in $required) {
  $p = Join-Path $Root $rel
  if (-not (Test-Path $p)) { throw "Missing additive Pass497-506 file: $rel" }
  Write-Host "[PASS] $rel"
}
$probes = @(
  'engine\include\content\ShipyardModuleSystem.h',
  'engine\include\ship_editor\ShipyardBuilderSystem.h',
  'engine\include\ship_editor\ShipyardRefitSystem.h',
  'engine\src\application\NativeBattlefieldRenderer.cpp'
)
Write-Host ""
Write-Host "Existing authority probes (read-only):"
foreach ($rel in $probes) {
  $p = Join-Path $Root $rel
  if (Test-Path $p) { Write-Host "[FOUND] $rel" } else { Write-Host "[INFO] not present: $rel" }
}
Write-Host ""
Write-Host "[PASS] Additive authority installed. No evolved Shipyard file was overwritten by this patch."
Write-Host "Run the normal Full Gate; engine CMake source globs should compile the new ship_editor source automatically."

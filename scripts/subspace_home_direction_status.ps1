param(
    [string]$Root = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
)

$ErrorActionPreference = "Stop"

Write-Host "========================================================================"
Write-Host " SUBSPACE HOME SYSTEM / ROGUELITE DIRECTION STATUS"
Write-Host "========================================================================"
Write-Host "Root: $Root"
Write-Host ""

$checks = @(
    @{ Name = "HomeSolarSystem model"; Path = "engine/include/home/HomeSolarSystem.h" },
    @{ Name = "Home factory network"; Path = "engine/include/home/HomeFactoryNetwork.h" },
    @{ Name = "Shipyard progression"; Path = "engine/include/home/HomeShipyardProgression.h" },
    @{ Name = "Expedition run model"; Path = "engine/include/expedition/ExpeditionRun.h" },
    @{ Name = "Roguelite director"; Path = "engine/include/roguelite/RogueliteDirector.h" },
    @{ Name = "Home save snapshot"; Path = "engine/include/core/persistence/HomeSystemSaveGame.h" },
    @{ Name = "Direction lock doc"; Path = "docs/design/SUBSPACE_ROGUELITE_INCREMENTAL_DIRECTION.md" },
    @{ Name = "Home system spec"; Path = "docs/design/HOME_SOLAR_SYSTEM_SPEC.md" },
    @{ Name = "Factory spec"; Path = "docs/design/SURFACE_FACTORY_AUTOMATION_SPEC.md" }
)

$missing = 0
foreach ($check in $checks) {
    $full = Join-Path $Root $check.Path
    if (Test-Path -LiteralPath $full) {
        Write-Host ("[PASS] {0,-28} {1}" -f $check.Name, $check.Path)
    }
    else {
        Write-Host ("[MISS] {0,-28} {1}" -f $check.Name, $check.Path)
        $missing++
    }
}

Write-Host ""
if ($missing -eq 0) {
    Write-Host "Home/roguelite foundation files are present."
    exit 0
}

Write-Host "$missing required files are missing. Apply Pass67-75 or regenerate the patch chain."
exit 1

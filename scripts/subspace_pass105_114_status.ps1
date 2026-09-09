param(
    [string]$Root = (Get-Location).Path
)

$ErrorActionPreference = 'Stop'
$required = @(
    'engine/include/home/HomeSurfaceWorldView.h',
    'engine/include/home/HomeSystemOverview.h',
    'engine/include/home/HomeOffworldOutpost.h',
    'engine/include/home/HomeImportExportLogistics.h',
    'engine/include/home/HomeSurfaceProductionGraph.h',
    'engine/include/home/HomeBuildableTerrain.h',
    'engine/include/home/HomeShipPrep.h',
    'engine/include/expedition/AdventureLaunchPlanner.h',
    'engine/include/client/VisualAcceptanceModel.h',
    'engine/include/migration/CppNormalizationRoadmap.h'
)

$missing = @()
foreach ($rel in $required) {
    if (-not (Test-Path (Join-Path $Root $rel))) { $missing += $rel }
}

Write-Host '========================================================================'
Write-Host ' SUBSPACE PASS105-114 HOME SURFACE REFOCUS STATUS'
Write-Host '========================================================================'
if ($missing.Count -eq 0) {
    Write-Host '[PASS] Pass105-114 files present.' -ForegroundColor Green
    exit 0
}
Write-Host '[FAIL] Missing Pass105-114 files:' -ForegroundColor Red
$missing | ForEach-Object { Write-Host "  - $_" }
exit 1

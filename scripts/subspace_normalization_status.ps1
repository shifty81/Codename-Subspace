param(
    [string]$Root = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
)

$ErrorActionPreference = "Stop"
$rootPath = (Resolve-Path $Root).Path
function Test-Rel([string]$Rel) { Test-Path -LiteralPath (Join-Path $rootPath $Rel) }
function Count-Files([string]$Rel, [string]$Filter = "*") {
    $path = Join-Path $rootPath $Rel
    if (-not (Test-Path -LiteralPath $path)) { return 0 }
    return @(Get-ChildItem -LiteralPath $path -Recurse -File -Filter $Filter -ErrorAction SilentlyContinue).Count
}

$items = @()
$items += [pscustomobject]@{ Area="Native runtime"; Status=$(if (Test-Rel "engine\src\application\NativeGameApplication.cpp") {"ACTIVE"} else {"MISSING"}); Next="Keep native C++ as sole runtime authority" }
$items += [pscustomobject]@{ Area="GameData"; Status=$(if (Test-Rel "GameData") {"ACTIVE-DATA"} else {"MISSING"}); Next="Retain as canonical authored gameplay/runtime data; migrate only through an explicit versioned loader/schema pass" }
$items += [pscustomobject]@{ Area="content"; Status=$(if (Test-Rel "content") {"ACTIVE-METADATA"} else {"MISSING"}); Next="Schemas, registries, provenance, governed metadata/derived authority" }
$items += [pscustomobject]@{ Area="Legacy C# donor"; Status=$(if (Test-Rel "AvorionLike") {"LOCAL-REFERENCE-PRESENT"} else {"NOT-IN-AUTHORITY"}); Next="Never runtime fallback; retain only for provenance/migration evidence if needed" }
$items += [pscustomobject]@{ Area="Legacy Assets roots"; Status=$(if ((Test-Rel "Assets") -or (Test-Rel "assets")) {"REVIEW-PRESENT"} else {"ABSENT"}); Next="If present, ingest with provenance into governed content; do not invent a move when absent" }
$items += [pscustomobject]@{ Area="Standalone PCC"; Status=$(if (Test-Rel "tools\control\StandaloneProjectControlCenter.ps1") {"ACTIVE"} else {"MISSING"}); Next="Internal PCC remains current operations authority until explicit Forge/Cortex takeover" }

Write-Host "========================================================================"
Write-Host " SUBSPACE NORMALIZATION STATUS"
Write-Host "========================================================================"
Write-Host "Root: $rootPath"
Write-Host ""
$items | Format-Table -AutoSize
Write-Host ""
Write-Host "Current normalization priorities:"
Write-Host "  1. Converge runtime state ownership/composition; do not reactivate detached historical lanes."
Write-Host "  2. Keep GameData/content split authoritative and normalize stale Avorion-era data vocabulary."
Write-Host "  3. Complete spatial/persistence foundations before broad seamless-planet implementation."
Write-Host "  4. Treat old rail/roguelite/home implementations as reference unless explicitly ported into current authority."

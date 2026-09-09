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
$items += [pscustomobject]@{ Area="Active C++ client"; Status=$(if (Test-Rel "engine\src\client\Win32PlayableClient.cpp") {"OK"} else {"MISSING"}); Next="Keep as visual acceptance harness" }
$items += [pscustomobject]@{ Area="C# source-to-port"; Status=$(if (Test-Rel "AvorionLike") {"ROOT-LEGACY-PRESENT"} elseif (Test-Rel "reference\csharp-to-cpp-source\AvorionLike") {"MIGRATION-AREA"} else {"MISSING"}); Next="Move only after conversion ledger/source-to-port intake is approved" }
$items += [pscustomobject]@{ Area="C# files remaining"; Status=(Count-Files "AvorionLike" "*.cs"); Next="Port or ledger each file before retiring" }
$items += [pscustomobject]@{ Area="C++ engine files"; Status=(Count-Files "engine" "*.cpp"); Next="Active implementation authority" }
$items += [pscustomobject]@{ Area="GameData root"; Status=$(if (Test-Rel "GameData") {"NEEDS-CONTENT-MIGRATION"} else {"OK"}); Next="Move to content/data after loader paths are audited" }
$items += [pscustomobject]@{ Area="Assets/assets roots"; Status=$(if ((Test-Rel "Assets") -or (Test-Rel "assets")) {"NEEDS-ASSET-MANIFEST"} else {"OK"}); Next="Normalize under content/assets with provenance manifest" }
$items += [pscustomobject]@{ Area="PixelPlanets reference"; Status=$(if (Test-Rel "reference\third_party\pixel_planets") {"OK"} else {"MISSING-IF-PASS52-NOT-APPLIED"}); Next="Reference-only C++ port source" }

Write-Host "========================================================================"
Write-Host " SUBSPACE NORMALIZATION STATUS"
Write-Host "========================================================================"
Write-Host "Root: $rootPath"
Write-Host ""
$items | Format-Table -AutoSize
Write-Host ""
Write-Host "Recommended next two normalization gates:"
Write-Host "  1. Finish C++ gameplay parity from AvorionLike systems still in root."
Write-Host "  2. Move AvorionLike into reference/csharp-to-cpp-source only after the migration ledger marks each subsystem ported/replaced/deferred."
Write-Host "  3. Normalize GameData/assets into content/data and content/assets once active C++ loader paths are known."

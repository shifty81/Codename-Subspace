param([string]$Root = (Get-Location).Path)
$ErrorActionPreference = "Stop"
$rootPath = (Resolve-Path -LiteralPath $Root).Path
$errors = New-Object System.Collections.Generic.List[string]

function Fail([string]$Message) { $errors.Add($Message) }

$cleanupScript = Join-Path $rootPath "scripts\subspace_legacy_artifact_cleanup.ps1"
if (Test-Path -LiteralPath $cleanupScript) {
    & $cleanupScript -Root $rootPath
    if ($LASTEXITCODE -ne 0) { Fail "Legacy generated-artifact cleanup failed." }
}
else {
    Fail "Missing baseline cleanup script: scripts/subspace_legacy_artifact_cleanup.ps1"
}

$engineCMake = Join-Path $rootPath "engine\CMakeLists.txt"
$engineMain = Join-Path $rootPath "engine\src\main.cpp"
if (-not (Test-Path -LiteralPath $engineCMake -PathType Leaf)) {
    Fail "Missing native build authority: engine/CMakeLists.txt"
}
if (-not (Test-Path -LiteralPath $engineMain -PathType Leaf)) {
    Fail "Missing native runtime entry point: engine/src/main.cpp"
}
if (Test-Path -LiteralPath $engineCMake -PathType Leaf) {
    $cmakeText = Get-Content -LiteralPath $engineCMake -Raw
    foreach ($needle in @('AvorionLike.sln','.csproj','dotnet build','dotnet run')) {
        if ($cmakeText -match [regex]::Escape($needle)) {
            Fail "Native CMake authority references retired managed build input '$needle'."
        }
    }
}

$operationalFiles = @(
    (Join-Path $rootPath "SubspaceTools.ps1"),
    (Join-Path $rootPath "Makefile"),
    (Join-Path $rootPath "scripts\setup.ps1"),
    (Join-Path $rootPath "scripts\setup.sh"),
    (Join-Path $rootPath "scripts\check-prerequisites.ps1"),
    (Join-Path $rootPath "scripts\check-prerequisites.sh"),
    (Join-Path $rootPath "scripts\test_ship_refinement.sh"),
    (Join-Path $rootPath "scripts\test_ship_connectivity.sh"),
    (Join-Path $rootPath "engine\CMakeLists.txt")
)
foreach ($file in $operationalFiles) {
    if (-not (Test-Path -LiteralPath $file)) { continue }
    $text = Get-Content -LiteralPath $file -Raw
    foreach ($needle in @('dotnet restore','dotnet build','dotnet run','build-csharp','run-csharp','subspace_client','AvorionLike.sln','AvorionLike.csproj')) {
        if ($text -match [regex]::Escape($needle)) {
            Fail "Operational build path still contains managed/legacy command '$needle' in $file"
        }
    }
}

$engineRoot = Join-Path $rootPath "engine"
if (Test-Path -LiteralPath $engineRoot) {
    $managed = @(Get-ChildItem -LiteralPath $engineRoot -Recurse -File -Include *.cs,*.csproj -ErrorAction SilentlyContinue)
    if ($managed.Count -gt 0) { Fail "Native engine tree contains managed source/project files." }
}

# PASS_CLEANCLONE_RETIREMENT_LEDGER_POLICY
# The CSV is a durable retirement/provenance ledger, not a promise that the
# retired C# archive must continue to exist forever. Every PHYSICAL legacy file
# must be classified exactly once with a terminal disposition; historical ledger
# rows may legitimately outlive the deleted physical archive.
$legacyRoot = Join-Path $rootPath "AvorionLike"
$manifestPath = Join-Path $rootPath "docs\legacy\CSHARP_RETIREMENT_MANIFEST.csv"
$legacyFiles = @()
if (Test-Path -LiteralPath $legacyRoot) {
    $legacyFiles = @(Get-ChildItem -LiteralPath $legacyRoot -Recurse -File -Filter *.cs -ErrorAction SilentlyContinue)
}

$manifestRowCount = 0
if (-not (Test-Path -LiteralPath $manifestPath)) {
    Fail "Missing C# retirement manifest: docs/legacy/CSHARP_RETIREMENT_MANIFEST.csv"
}
else {
    $rows = @(Import-Csv -LiteralPath $manifestPath)
    $manifestRowCount = $rows.Count
    $allowedDispositions = @('NATIVE_REPLACED','SUPERSEDED_CURRENT_DESIGN','REFERENCE_ONLY_ALGORITHM','REFERENCE_ONLY')
    $manifestPaths = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)

    foreach ($row in $rows) {
        $path = [string]$row.legacy_file
        $disposition = [string]$row.disposition
        if ([string]::IsNullOrWhiteSpace($path)) {
            Fail "Retirement manifest contains an empty legacy_file path."
            continue
        }

        $normalized = $path.Replace('\','/')
        if (-not $manifestPaths.Add($normalized)) {
            Fail "Retirement manifest contains duplicate path: $path"
        }
        if ($allowedDispositions -notcontains $disposition) {
            Fail "Retirement manifest contains non-terminal disposition '$disposition' for $path"
        }
    }

    foreach ($file in $legacyFiles) {
        $relative = $file.FullName.Substring($rootPath.Length + 1).Replace('\','/')
        if (-not $manifestPaths.Contains($relative)) {
            Fail "Legacy C# file is not classified in retirement manifest: $relative"
        }
    }
}

Write-Host "========================================================================"
Write-Host " SUBSPACE NATIVE RUNTIME GUARD"
Write-Host "========================================================================"
Write-Host "Root: $rootPath"
Write-Host ""
Write-Host "Legacy C# source may remain only under AvorionLike/ as inert reference material."
Write-Host "The physical archive may also be fully retired; the manifest remains provenance."
Write-Host "Shipping/build/runtime authority must remain entirely under native C++ targets."
Write-Host ""

if ($errors.Count -gt 0) {
    foreach ($e in $errors) { Write-Host "[FAIL] $e" -ForegroundColor Red }
    Write-Host "RESULT: FAIL ($($errors.Count) native-runtime regression(s))" -ForegroundColor Red
    exit 1
}

Write-Host "[PASS] Native build authority is engine/CMakeLists.txt with a native C++ entry point." -ForegroundColor Green
Write-Host "[PASS] Retired C# material is not part of the active build path." -ForegroundColor Green
Write-Host "[PASS] Operational build/setup path contains no dotnet/C# commands or legacy client fallback." -ForegroundColor Green
Write-Host "[PASS] engine/ contains no managed source or project files." -ForegroundColor Green
Write-Host ("[PASS] Retirement ledger has {0} terminal historical row(s) and classifies every one of {1} physical legacy C# source file(s)." -f $manifestRowCount,$legacyFiles.Count) -ForegroundColor Green
Write-Host "[PASS] Stale generated conversion-era client artifacts are absent." -ForegroundColor Green
Write-Host "RESULT: PASS - Pass190 production runtime authority is native C++." -ForegroundColor Green
exit 0

param([string]$Root = (Get-Location).Path)
$ErrorActionPreference = "Stop"
$failures = 0
$warnings = 0

function Write-Status {
    param([string]$Status, [string]$Name, [string]$Detail = "")
    $color = switch ($Status) { "PASS" { "Green" } "WARN" { "Yellow" } "FAIL" { "Red" } default { "Gray" } }
    Write-Host ("[{0}] {1}" -f $Status, $Name) -ForegroundColor $color
    if ($Detail) { Write-Host ("      {0}" -f $Detail) -ForegroundColor DarkGray }
    if ($Status -eq "FAIL") { $script:failures++ }
    if ($Status -eq "WARN") { $script:warnings++ }
}
function Count-Files {
    param([string]$Path,[string]$Filter="*")
    if (-not (Test-Path -LiteralPath $Path)) { return 0 }
    return @(Get-ChildItem -LiteralPath $Path -Recurse -File -Filter $Filter -ErrorAction SilentlyContinue).Count
}

$rootPath = (Resolve-Path -LiteralPath $Root).Path
$engineRoot = Join-Path $rootPath "engine"
$engineCmake = Join-Path $engineRoot "CMakeLists.txt"
$gameMain = Join-Path $engineRoot "src\main.cpp"
$nativeApp = Join-Path $engineRoot "src\application\NativeGameApplication.cpp"
$nativeRenderer = Join-Path $engineRoot "src\application\NativeBattlefieldRenderer.cpp"
$retirementManifest = Join-Path $rootPath "docs\legacy\CSHARP_RETIREMENT_MANIFEST.csv"

Write-Host "========================================================================"
Write-Host " CODENAME SUBSPACE - PROJECT SCOPE STATUS (PASS190)"
Write-Host "========================================================================"
Write-Host "Root: $rootPath"
Write-Host ""
Write-Host "Build/runtime model"
Write-Host "-------------------"
Write-Host "Production workspace           : engine/"
Write-Host "Production executable          : subspace_game.exe"
Write-Host "Runtime authority              : native C++ only"
Write-Host "Build authority                : engine/CMakeLists.txt + C++ Visual Studio solution"
Write-Host "Legacy AvorionLike/*.cs        : retired provenance only; physical archive is optional"
Write-Host ""

if (Test-Path -LiteralPath $engineRoot) { Write-Status PASS "engine/ exists" $engineRoot } else { Write-Status FAIL "engine/ missing" $engineRoot }
if (Test-Path -LiteralPath $engineCmake) { Write-Status PASS "engine/CMakeLists.txt exists" } else { Write-Status FAIL "engine/CMakeLists.txt missing" }
if (Test-Path -LiteralPath $gameMain) { Write-Status PASS "native game entry point exists" "engine/src/main.cpp" } else { Write-Status FAIL "native game entry point missing" }
if (Test-Path -LiteralPath $nativeApp) { Write-Status PASS "native application shell exists" "engine/src/application/NativeGameApplication.cpp" } else { Write-Status FAIL "native application shell missing" }
if (Test-Path -LiteralPath $nativeRenderer) { Write-Status PASS "native battlefield renderer exists" "engine/src/application/NativeBattlefieldRenderer.cpp" } else { Write-Status FAIL "native battlefield renderer missing" }

$cppSources = Count-Files -Path (Join-Path $engineRoot "src") -Filter "*.cpp"
$cppHeaders = Count-Files -Path (Join-Path $engineRoot "include") -Filter "*.h"
Write-Status PASS "Native C++ source inventory" ("{0} .cpp files, {1} .h files under engine/" -f $cppSources,$cppHeaders)

$expectedAreas = @('application','cargo','combat','core','debug_tools','economy','factions','fleet','input','interior','mining','procedural','rendering','runtime','salvage','ships','station','trading','ui')
foreach ($area in $expectedAreas) {
    $src = Join-Path $engineRoot ("src\" + $area)
    $inc = Join-Path $engineRoot ("include\" + $area)
    if ((Test-Path -LiteralPath $src) -and (Test-Path -LiteralPath $inc)) { Write-Status PASS "native area present: $area" }
    else { Write-Status FAIL "native area incomplete: $area" "Expected matching engine/src and engine/include areas." }
}

Write-Host ""
Write-Host "Retired managed reference archive"
Write-Host "--------------------------------"
$legacyRoot = Join-Path $rootPath "AvorionLike"
$legacyFiles = @()
if (Test-Path -LiteralPath $legacyRoot) {
    $legacyFiles = @(Get-ChildItem -LiteralPath $legacyRoot -Recurse -File -Filter *.cs -ErrorAction SilentlyContinue)
}
$legacyCount = $legacyFiles.Count

if ($legacyCount -gt 0) {
    Write-Status PASS "Legacy C# archive retained for provenance/reference" ("{0} .cs files; not compiled or shipped." -f $legacyCount)
} else {
    Write-Status PASS "Legacy C# archive physically retired" "No legacy .cs files remain in the checkout; the retirement manifest remains as provenance."
}

# PASS_CLEANCLONE_RETIREMENT_LEDGER_POLICY
# The retirement manifest is a durable historical/provenance ledger. Once the
# legacy archive is intentionally deleted, its historical rows must NOT be
# treated as missing-source failures. If any physical legacy file exists, it
# must still be represented exactly once with a terminal disposition.
if (Test-Path -LiteralPath $retirementManifest) {
    $rows = @(Import-Csv -LiteralPath $retirementManifest)
    $allowedDispositions = @('NATIVE_REPLACED','SUPERSEDED_CURRENT_DESIGN','REFERENCE_ONLY_ALGORITHM','REFERENCE_ONLY')
    $manifestPaths = [System.Collections.Generic.HashSet[string]]::new([System.StringComparer]::OrdinalIgnoreCase)
    $manifestInvalid = $false

    foreach ($row in $rows) {
        $path = [string]$row.legacy_file
        $disposition = [string]$row.disposition
        if ([string]::IsNullOrWhiteSpace($path)) {
            Write-Status FAIL "Retirement manifest contains empty legacy path"
            $manifestInvalid = $true
            continue
        }
        $normalized = $path.Replace('\','/')
        if (-not $manifestPaths.Add($normalized)) {
            Write-Status FAIL "Retirement manifest contains duplicate legacy path" $path
            $manifestInvalid = $true
        }
        if ($allowedDispositions -notcontains $disposition) {
            Write-Status FAIL "Retirement manifest contains non-terminal disposition" ("{0}: {1}" -f $path,$disposition)
            $manifestInvalid = $true
        }
    }

    foreach ($file in $legacyFiles) {
        $relative = $file.FullName.Substring($rootPath.Length + 1).Replace('\','/')
        if (-not $manifestPaths.Contains($relative)) {
            Write-Status FAIL "Physical legacy C# file is not classified in retirement manifest" $relative
            $manifestInvalid = $true
        }
    }

    if (-not $manifestInvalid) {
        if ($legacyCount -eq 0) {
            Write-Status PASS "Retirement provenance ledger retained after physical archive removal" ("{0} historical terminal classification row(s), 0 physical .cs file(s)." -f $rows.Count)
        } else {
            Write-Status PASS "Retirement manifest classifies every physical legacy source" ("{0} historical row(s); {1} physical .cs file(s)." -f $rows.Count,$legacyCount)
        }
    }
} else {
    Write-Status FAIL "Retirement manifest missing" "docs/legacy/CSHARP_RETIREMENT_MANIFEST.csv"
}

Write-Host ""
Write-Host "Project support/content scope"
Write-Host "-----------------------------"
foreach ($dir in @('Assets','GameData','docs','scripts','tools')) {
    $path = Join-Path $rootPath $dir
    if (Test-Path -LiteralPath $path) { Write-Status PASS "root area present: $dir/" }
    else { Write-Status WARN "root area missing: $dir/" }
}
if (Test-Path -LiteralPath (Join-Path $rootPath 'reference')) {
    Write-Status PASS "optional reference/ area present"
} else {
    Write-Status PASS "optional reference/ area not required" "Retired C# provenance is preserved by docs/legacy/CSHARP_RETIREMENT_MANIFEST.csv."
}

Write-Host ""
Write-Host "Summary"
Write-Host "-------"
if ($failures -eq 0) {
    Write-Host ("RESULT: PASS with {0} warning(s). Native production scope is normalized." -f $warnings) -ForegroundColor Green
    exit 0
}
Write-Host ("RESULT: FAIL with {0} failure(s), {1} warning(s)." -f $failures,$warnings) -ForegroundColor Red
exit 1

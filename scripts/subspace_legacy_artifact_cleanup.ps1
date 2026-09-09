param([string]$Root = (Get-Location).Path)
$ErrorActionPreference = "Stop"
$rootPath = (Resolve-Path -LiteralPath $Root).Path
$removed = New-Object System.Collections.Generic.List[string]
$failures = New-Object System.Collections.Generic.List[string]

function Remove-GeneratedPath([string]$Path) {
    if (-not (Test-Path -LiteralPath $Path)) { return }
    try {
        Remove-Item -LiteralPath $Path -Recurse -Force -ErrorAction Stop
        $removed.Add($Path)
    }
    catch {
        $failures.Add(("{0}: {1}" -f $Path, $_.Exception.Message))
    }
}

Write-Host "========================================================================"
Write-Host " SUBSPACE LEGACY GENERATED-ARTIFACT CLEANUP"
Write-Host "========================================================================"
Write-Host "Root: $rootPath"
Write-Host "Only generated conversion-era build artifacts are eligible for removal."
Write-Host "Legacy C# source/reference files are never deleted by this script."
Write-Host ""

# Old managed build output is generated and no longer part of the production path.
Remove-GeneratedPath (Join-Path $rootPath "AvorionLike\bin")
Remove-GeneratedPath (Join-Path $rootPath "AvorionLike\obj")

# Remove only the obsolete executable family from native build directories.
$engineRoot = Join-Path $rootPath "engine"
if (Test-Path -LiteralPath $engineRoot) {
    $buildDirs = @(Get-ChildItem -LiteralPath $engineRoot -Directory -ErrorAction SilentlyContinue | Where-Object { $_.Name -like "build*" })
    foreach ($buildDir in $buildDirs) {
        $legacyArtifacts = @(Get-ChildItem -LiteralPath $buildDir.FullName -Recurse -File -ErrorAction SilentlyContinue | Where-Object {
            $_.BaseName -eq "subspace_client" -or $_.Name -like "subspace_client.*"
        })
        foreach ($artifact in $legacyArtifacts) { Remove-GeneratedPath $artifact.FullName }
    }
}

if ($failures.Count -gt 0) {
    foreach ($failure in $failures) { Write-Host "[FAIL] $failure" -ForegroundColor Red }
    Write-Host "RESULT: FAIL - one or more stale generated artifacts could not be removed." -ForegroundColor Red
    exit 1
}

if ($removed.Count -eq 0) {
    Write-Host "[PASS] No stale conversion-era generated artifacts were present." -ForegroundColor Green
}
else {
    foreach ($path in $removed) { Write-Host "[CLEAN] $path" -ForegroundColor DarkGray }
    Write-Host ("[PASS] Removed {0} stale generated artifact path(s)." -f $removed.Count) -ForegroundColor Green
}
Write-Host "RESULT: PASS - generated runtime artifacts are native-baseline clean." -ForegroundColor Green
exit 0

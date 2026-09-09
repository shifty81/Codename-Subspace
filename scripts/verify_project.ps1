param(
    [string]$Root = (Resolve-Path (Join-Path $PSScriptRoot "..")),
    [switch]$Configure,
    [switch]$Build,
    [switch]$Test,
    [switch]$Headless
)

$ErrorActionPreference = "Stop"
$engine = Join-Path $Root "engine"
$build = Join-Path $Root "build\subspace-verify"

Write-Host "Subspace project verification"
Write-Host "Root: $Root"

$legacyMarkers = @("AvorionLike", "EVEInspired", "NovaForge", "X4")
foreach ($marker in $legacyMarkers) {
    $matches = Get-ChildItem -Path $Root -Recurse -File -ErrorAction SilentlyContinue |
        Where-Object { $_.FullName -notmatch "\\build\\|\\.git\\|\\bin\\|\\obj\\" } |
        Select-String -Pattern $marker -SimpleMatch -ErrorAction SilentlyContinue
    if ($matches) {
        Write-Warning "Legacy marker '$marker' still appears $($matches.Count) time(s)."
    }
}

$recursiveLink = Join-Path $engine "engine"
if (Test-Path $recursiveLink) {
    Write-Warning "Potential recursive engine/engine path exists. Remove it before broad scans or packaging."
}

if ($Configure) {
    New-Item -ItemType Directory -Force -Path $build | Out-Null
    $args = @("-S", $engine, "-B", $build, "-DSUBSPACE_BUILD_TESTS=ON")
    if ($Headless) { $args += "-DSUBSPACE_HEADLESS=ON" }
    cmake @args
}

if ($Build) {
    cmake --build $build --config RelWithDebInfo
}

if ($Test) {
    ctest --test-dir $build --output-on-failure -C RelWithDebInfo
}

Write-Host "Verification script completed."

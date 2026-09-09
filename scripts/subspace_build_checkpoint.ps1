param(
    [string]$Root = (Get-Location).Path,
    [ValidateSet("Debug", "Release", "RelWithDebInfo")]
    [string]$Configuration = "Debug",
    [switch]$Headless,
    [switch]$Clean
)

$ErrorActionPreference = "Stop"
$engine = Join-Path $Root "engine"
$build = Join-Path $engine "build"
if ($Headless) { $build = Join-Path $engine "build-headless" }

if ($Clean -and (Test-Path $build)) {
    Remove-Item -Recurse -Force $build
}
New-Item -ItemType Directory -Force -Path $build | Out-Null

$openGl = if ($Headless) { "OFF" } else { "ON" }
$headlessValue = if ($Headless) { "ON" } else { "OFF" }

Write-Host "Configuring Subspace engine"
cmake -S $engine -B $build -DSUBSPACE_HEADLESS=$headlessValue -DSUBSPACE_BUILD_OPENGL=$openGl -DSUBSPACE_BUILD_TESTS=ON

Write-Host "Building Subspace engine/tests"
cmake --build $build --config $Configuration

Write-Host "Running tests"
ctest --test-dir $build -C $Configuration --output-on-failure

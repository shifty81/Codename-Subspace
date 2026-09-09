#!/usr/bin/env pwsh
$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$Engine = Join-Path $Root "engine"

Write-Host "========================================" -ForegroundColor Cyan
Write-Host " Codename: Subspace Native C++ Setup" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

foreach ($tool in @("cmake")) {
    if (-not (Get-Command $tool -ErrorAction SilentlyContinue)) {
        throw "$tool is required. Install CMake and Visual Studio 2022 Desktop development with C++."
    }
}

$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (-not (Test-Path -LiteralPath $vswhere)) { throw "Visual Studio Installer/vswhere was not found." }
$vsPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vsPath) { throw "Visual Studio 2022 C++ toolchain was not found." }

Write-Host "[PASS] CMake and Visual Studio C++ toolchain found." -ForegroundColor Green
& (Join-Path $Root "scripts\subspace_native_runtime_guard.ps1") -Root $Root
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$build = Join-Path $Engine "build"
cmake -S $Engine -B $build -DSUBSPACE_BUILD_TESTS=ON
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
cmake --build $build --config Debug
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
ctest --test-dir $build -C Debug --output-on-failure
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host ""
Write-Host "Native Subspace setup/build/test complete." -ForegroundColor Green
Write-Host "Launch through SubspaceTools.ps1 -> Run native C++ game." -ForegroundColor Cyan

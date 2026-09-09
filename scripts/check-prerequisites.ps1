#!/usr/bin/env pwsh
$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$allOk = $true

Write-Host "========================================" -ForegroundColor Cyan
Write-Host " Codename: Subspace Native Prerequisites" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan

foreach ($tool in @("cmake", "git")) {
    if (Get-Command $tool -ErrorAction SilentlyContinue) { Write-Host "[PASS] $tool" -ForegroundColor Green }
    else { Write-Host "[WARN] $tool not found" -ForegroundColor Yellow; if ($tool -eq "cmake") { $allOk=$false } }
}

$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path -LiteralPath $vswhere) {
    $vsPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath 2>$null
    if ($vsPath) { Write-Host "[PASS] Visual Studio C++ toolchain" -ForegroundColor Green }
    else { Write-Host "[FAIL] Visual Studio Desktop development with C++ workload not found" -ForegroundColor Red; $allOk=$false }
} else { Write-Host "[FAIL] Visual Studio Installer/vswhere not found" -ForegroundColor Red; $allOk=$false }

$guard = Join-Path $Root "scripts\subspace_native_runtime_guard.ps1"
if (Test-Path -LiteralPath $guard) { & $guard -Root $Root; if ($LASTEXITCODE -ne 0) { $allOk=$false } }

if (-not $allOk) { exit 1 }
Write-Host "Native C++ prerequisites are ready." -ForegroundColor Green

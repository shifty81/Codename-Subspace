#!/usr/bin/env pwsh
# Codename: Subspace - Automated Setup Script for Windows/PowerShell
# This script checks for prerequisites and sets up both the C# prototype
# and the new C++ engine.

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Codename: Subspace Setup Script" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Function to check if a command exists
function Test-Command {
    param($Command)
    try {
        if (Get-Command $Command -ErrorAction Stop) {
            return $true
        }
    }
    catch {
        return $false
    }
}

# Function to get .NET SDK version
function Get-DotNetVersion {
    try {
        $version = dotnet --version 2>$null
        return $version
    }
    catch {
        return $null
    }
}

# Check for .NET SDK
Write-Host "Checking for .NET SDK..." -ForegroundColor Yellow
if (-not (Test-Command "dotnet")) {
    Write-Host "❌ .NET SDK is not installed!" -ForegroundColor Red
    Write-Host ""
    Write-Host "Please install .NET 9.0 SDK or later from:" -ForegroundColor Yellow
    Write-Host "  https://dotnet.microsoft.com/download" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Installation instructions:" -ForegroundColor Yellow
    Write-Host "  1. Visit the URL above" -ForegroundColor White
    Write-Host "  2. Download .NET 9.0 SDK (or later)" -ForegroundColor White
    Write-Host "  3. Run the installer" -ForegroundColor White
    Write-Host "  4. Restart your terminal" -ForegroundColor White
    Write-Host "  5. Run this script again" -ForegroundColor White
    Write-Host ""
    exit 1
}

$dotnetVersion = Get-DotNetVersion
Write-Host "✓ .NET SDK is installed (version $dotnetVersion)" -ForegroundColor Green

# Check if version is sufficient (9.0 or higher)
# Handle preview versions like "9.0.0-preview.1" by extracting numeric part only
$versionParts = $dotnetVersion.Split('.')
$majorVersionString = $versionParts[0] -replace '[^0-9].*$', ''
try {
    $majorVersion = [int]$majorVersionString
}
catch {
    $majorVersion = 0
}
if ($majorVersion -lt 9) {
    Write-Host "⚠️  Warning: .NET SDK version $dotnetVersion detected." -ForegroundColor Yellow
    Write-Host "   This project requires .NET 9.0 or later." -ForegroundColor Yellow
    Write-Host "   Please update your .NET SDK from: https://dotnet.microsoft.com/download" -ForegroundColor Yellow
    Write-Host ""
    $continue = Read-Host "Do you want to continue anyway? (y/N)"
    if ($continue -ne "y" -and $continue -ne "Y") {
        exit 1
    }
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  Installing Dependencies" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Navigate to project directory
$projectDir = Join-Path $PSScriptRoot "AvorionLike"
if (-not (Test-Path $projectDir)) {
    Write-Host "❌ Project directory not found: $projectDir" -ForegroundColor Red
    exit 1
}

Set-Location $projectDir

# Restore NuGet packages
Write-Host "Restoring NuGet packages..." -ForegroundColor Yellow
dotnet restore
if ($LASTEXITCODE -ne 0) {
    Write-Host "❌ Failed to restore NuGet packages!" -ForegroundColor Red
    exit 1
}
Write-Host "✓ NuGet packages restored successfully" -ForegroundColor Green
Write-Host ""

# Build the project
Write-Host "Building the project..." -ForegroundColor Yellow
dotnet build
if ($LASTEXITCODE -ne 0) {
    Write-Host "❌ Build failed!" -ForegroundColor Red
    exit 1
}
Write-Host "✓ Project built successfully" -ForegroundColor Green
Write-Host ""

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  C# Prototype Setup Complete!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "You can now run the C# prototype using:" -ForegroundColor Yellow
Write-Host "  cd AvorionLike" -ForegroundColor Cyan
Write-Host "  dotnet run" -ForegroundColor Cyan
Write-Host ""
Write-Host "Or build and run in release mode:" -ForegroundColor Yellow
Write-Host "  cd AvorionLike" -ForegroundColor Cyan
Write-Host "  dotnet run --configuration Release" -ForegroundColor Cyan
Write-Host ""

# ================================================================
# C++ Engine Build (Visual Studio)
# ================================================================
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  C++ Engine (Visual Studio)" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

# Check for Visual Studio MSBuild
$msbuildPath = $null
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
if (Test-Path $vswhere) {
    $vsPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath 2>$null
    if ($vsPath) {
        $msbuildPath = Join-Path $vsPath "MSBuild\Current\Bin\MSBuild.exe"
        if (-not (Test-Path $msbuildPath)) { $msbuildPath = $null }
    }
}

if ($msbuildPath) {
    Write-Host "✓ Visual Studio C++ toolchain found" -ForegroundColor Green
    Write-Host "  MSBuild: $msbuildPath" -ForegroundColor Gray
    Write-Host ""

    Write-Host "Building C++ engine (SubspaceEngine)..." -ForegroundColor Yellow
    & $msbuildPath "$PSScriptRoot\AvorionLike.sln" /p:Configuration=Debug /p:Platform=x64 /t:SubspaceEngine /v:minimal 2>&1
    if ($LASTEXITCODE -eq 0) {
        Write-Host "✓ C++ engine built successfully" -ForegroundColor Green
    } else {
        Write-Host "⚠️  C++ engine build had issues (this is optional)" -ForegroundColor Yellow
    }

    Write-Host ""
    Write-Host "Building C++ tests (SubspaceTests)..." -ForegroundColor Yellow
    & $msbuildPath "$PSScriptRoot\AvorionLike.sln" /p:Configuration=Debug /p:Platform=x64 /t:SubspaceTests /v:minimal 2>&1
    if ($LASTEXITCODE -eq 0) {
        Write-Host "✓ C++ tests built successfully" -ForegroundColor Green
        Write-Host ""
        Write-Host "Running C++ tests..." -ForegroundColor Yellow
        $testExe = "$PSScriptRoot\out\Debug\SubspaceTests.exe"
        if (Test-Path $testExe) {
            & $testExe
        }
    } else {
        Write-Host "⚠️  C++ test build had issues (this is optional)" -ForegroundColor Yellow
    }
} else {
    Write-Host "⚠️  Visual Studio C++ toolchain not found." -ForegroundColor Yellow
    Write-Host "   The C++ engine requires Visual Studio 2022 with the" -ForegroundColor Yellow
    Write-Host "   'Desktop development with C++' workload installed." -ForegroundColor Yellow
    Write-Host ""
    Write-Host "   The C# prototype will still work fine without it." -ForegroundColor Yellow
    Write-Host ""
    Write-Host "   To install:" -ForegroundColor White
    Write-Host "   1. Open Visual Studio Installer" -ForegroundColor White
    Write-Host "   2. Click 'Modify' on your VS 2022 installation" -ForegroundColor White
    Write-Host "   3. Check 'Desktop development with C++'" -ForegroundColor White
    Write-Host "   4. Click 'Modify' to install" -ForegroundColor White
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  All Done!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Open AvorionLike.sln in Visual Studio to see both projects:" -ForegroundColor Yellow
Write-Host "  - C# Prototype (AvorionLike) — existing gameplay prototype" -ForegroundColor Cyan
Write-Host "  - C++ Engine (SubspaceEngine) — new block-based ship engine" -ForegroundColor Cyan
Write-Host "  - C++ Game   (SubspaceGame)   — engine executable" -ForegroundColor Cyan
Write-Host "  - C++ Tests  (SubspaceTests)  — 118 engine unit tests" -ForegroundColor Cyan
Write-Host ""

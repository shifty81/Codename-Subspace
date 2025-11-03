#!/usr/bin/env pwsh
# AvorionLike - Automated Setup Script for Windows/PowerShell
# This script checks for prerequisites and sets up the project automatically

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  AvorionLike Setup Script" -ForegroundColor Cyan
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
$versionParts = $dotnetVersion.Split('.')
$majorVersion = [int]$versionParts[0]
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
Write-Host "  Setup Complete!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "You can now run the application using:" -ForegroundColor Yellow
Write-Host "  cd AvorionLike" -ForegroundColor Cyan
Write-Host "  dotnet run" -ForegroundColor Cyan
Write-Host ""
Write-Host "Or build and run in release mode:" -ForegroundColor Yellow
Write-Host "  cd AvorionLike" -ForegroundColor Cyan
Write-Host "  dotnet run --configuration Release" -ForegroundColor Cyan
Write-Host ""

#!/usr/bin/env pwsh
# AvorionLike - Prerequisites Verification Script
# This script checks if all prerequisites are met without installing anything

Write-Host "========================================" -ForegroundColor Cyan
Write-Host "  AvorionLike Prerequisites Check" -ForegroundColor Cyan
Write-Host "========================================" -ForegroundColor Cyan
Write-Host ""

$allOk = $true

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
    Write-Host "❌ .NET SDK is not installed" -ForegroundColor Red
    $allOk = $false
}
else {
    $dotnetVersion = Get-DotNetVersion
    $versionParts = $dotnetVersion.Split('.')
    $majorVersion = [int]$versionParts[0]
    
    if ($majorVersion -ge 9) {
        Write-Host "✓ .NET SDK $dotnetVersion (compatible)" -ForegroundColor Green
    }
    else {
        Write-Host "⚠️  .NET SDK $dotnetVersion (requires 9.0+)" -ForegroundColor Yellow
        $allOk = $false
    }
}

# Check for git (optional but recommended)
Write-Host "Checking for Git..." -ForegroundColor Yellow
if (-not (Test-Command "git")) {
    Write-Host "⚠️  Git is not installed (optional, but recommended)" -ForegroundColor Yellow
}
else {
    $gitVersion = (git --version).Split(' ')[-1]
    Write-Host "✓ Git $gitVersion" -ForegroundColor Green
}

# Check project files
Write-Host ""
Write-Host "Checking project files..." -ForegroundColor Yellow
$projectDir = Join-Path $PSScriptRoot "AvorionLike"

if (-not (Test-Path $projectDir)) {
    Write-Host "❌ Project directory not found: $projectDir" -ForegroundColor Red
    $allOk = $false
}
else {
    Write-Host "✓ Project directory found" -ForegroundColor Green
    
    $csprojPath = Join-Path $projectDir "AvorionLike.csproj"
    if (-not (Test-Path $csprojPath)) {
        Write-Host "❌ Project file not found: AvorionLike.csproj" -ForegroundColor Red
        $allOk = $false
    }
    else {
        Write-Host "✓ Project file found" -ForegroundColor Green
    }
}

Write-Host ""
Write-Host "========================================" -ForegroundColor Cyan

if ($allOk) {
    Write-Host "✓ All prerequisites are met!" -ForegroundColor Green
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Ready to build the project. Run:" -ForegroundColor Yellow
    Write-Host "  .\setup.ps1" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Or manually:" -ForegroundColor Yellow
    Write-Host "  cd AvorionLike" -ForegroundColor Cyan
    Write-Host "  dotnet restore" -ForegroundColor Cyan
    Write-Host "  dotnet build" -ForegroundColor Cyan
    Write-Host "  dotnet run" -ForegroundColor Cyan
}
else {
    Write-Host "❌ Some prerequisites are missing" -ForegroundColor Red
    Write-Host "========================================" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Please install missing prerequisites and run this script again." -ForegroundColor Yellow
    Write-Host ""
    Write-Host "To install .NET SDK, visit:" -ForegroundColor Yellow
    Write-Host "  https://dotnet.microsoft.com/download" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Or run the automated setup:" -ForegroundColor Yellow
    Write-Host "  .\setup.ps1" -ForegroundColor Cyan
    exit 1
}

Write-Host ""

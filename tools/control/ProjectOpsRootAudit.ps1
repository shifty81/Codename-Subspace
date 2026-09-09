[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)][string]$Root,
    [switch]$FailOnWarnings
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version 2

$Root = [System.IO.Path]::GetFullPath($Root)
$commonPath = Join-Path $PSScriptRoot 'ProjectOpsCommon.psm1'
if (-not (Test-Path -LiteralPath $commonPath -PathType Leaf)) {
    throw "ProjectOps common module missing: $commonPath"
}
Import-Module $commonPath -Force -ErrorAction Stop

$contract = Get-ProjectOpsContract -Root $Root
if ($null -eq $contract.projectOps -or $null -eq $contract.projectOps.rootPolicy) {
    throw 'project.control.json does not define projectOps.rootPolicy.'
}
$policy = $contract.projectOps.rootPolicy

$requiredFiles = @($policy.requiredFiles | ForEach-Object { [string]$_ })
$allowedFiles = @($policy.allowedFiles | ForEach-Object { [string]$_ })
$allowedDirectories = @($policy.allowedDirectories | ForEach-Object { [string]$_ })
$legacyFiles = @($policy.legacyFiles | ForEach-Object { [string]$_ })
$legacyDirectories = @($policy.legacyDirectories)
$forbiddenGeneratedRoots = @($policy.forbiddenGeneratedRoots | ForEach-Object { [string]$_ })

$critical = [System.Collections.Generic.List[string]]::new()
$warnings = [System.Collections.Generic.List[string]]::new()

Write-Host 'PROJECTOPS ROOT AUTHORITY AUDIT' -ForegroundColor Cyan
Write-Host (" Project : {0}" -f $contract.name)
Write-Host (" Root    : {0}" -f $Root)
Write-Host '------------------------------------------------------------------------'

foreach ($required in $requiredFiles) {
    $path = Join-Path $Root $required
    if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
        $critical.Add("Missing required root authority: $required") | Out-Null
    }
}

foreach ($forbidden in $forbiddenGeneratedRoots) {
    $path = Join-Path $Root $forbidden
    if (Test-Path -LiteralPath $path) {
        $critical.Add("Forbidden generated/retired root exists outside canonical policy: $forbidden") | Out-Null
    }
}

foreach ($legacy in $legacyFiles) {
    $path = Join-Path $Root $legacy
    if (Test-Path -LiteralPath $path -PathType Leaf) {
        $critical.Add("Legacy root file was not migrated: $legacy") | Out-Null
    }
}

$entries = @(Get-ChildItem -LiteralPath $Root -Force -ErrorAction Stop)
foreach ($entry in $entries) {
    if ($entry.PSIsContainer) {
        if ($allowedDirectories -contains $entry.Name) { continue }
        $warnings.Add("Unexpected root directory: $($entry.Name)") | Out-Null
        continue
    }

    if ($allowedFiles -contains $entry.Name) { continue }
    if ($legacyFiles -contains $entry.Name) {
        $critical.Add("Legacy root file remains live: $($entry.Name)") | Out-Null
        continue
    }
    $warnings.Add("Unexpected root file: $($entry.Name)") | Out-Null
}

foreach ($legacyDirectory in $legacyDirectories) {
    $pathValue = [string]$legacyDirectory.path
    if ([string]::IsNullOrWhiteSpace($pathValue)) { continue }
    $path = Join-Path $Root $pathValue
    if (Test-Path -LiteralPath $path -PathType Container) {
        $mode = [string]$legacyDirectory.mode
        Write-Host ("[LEGACY] {0} ({1}) - local reference only; excluded from source/repository authority." -f $pathValue,$mode) -ForegroundColor DarkGray
    }
}

foreach ($item in $critical) {
    Write-Host ("[FAIL] " + $item) -ForegroundColor Red
}
foreach ($item in $warnings) {
    Write-Host ("[WARN] " + $item) -ForegroundColor Yellow
}

if ($critical.Count -gt 0) {
    Write-Host ("RESULT: FAIL ({0} critical root-authority defect(s))" -f $critical.Count) -ForegroundColor Red
    exit 1
}
if ($warnings.Count -gt 0 -and $FailOnWarnings) {
    Write-Host ("RESULT: FAIL ({0} warning(s), FailOnWarnings enabled)" -f $warnings.Count) -ForegroundColor Red
    exit 1
}
if ($warnings.Count -gt 0) {
    Write-Host ("RESULT: PASS WITH WARNINGS ({0})" -f $warnings.Count) -ForegroundColor Yellow
    exit 0
}

Write-Host 'RESULT: PASS - root contains only current project authority plus explicitly allowed local/reference directories.' -ForegroundColor Green
exit 0

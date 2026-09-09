[CmdletBinding()]
param(
    [Parameter(Mandatory=$true)][string]$Root,
    [ValidateSet('Repair','Caches','Audit')][string]$Action = 'Repair'
)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version 2

$Root = [System.IO.Path]::GetFullPath($Root)
$commonPath = Join-Path $PSScriptRoot 'ProjectOpsCommon.psm1'
if (-not (Test-Path -LiteralPath $commonPath -PathType Leaf)) {
    throw "ProjectOps common module missing: $commonPath"
}
Import-Module $commonPath -Force -ErrorAction Stop

if ($Action -in @('Repair','Caches')) {
    $removed = @(Remove-ProjectOpsTransientCaches -Root $Root)
    Write-Host ("Transient cache directories removed: {0}" -f $removed.Count) -ForegroundColor $(if($removed.Count -gt 0){'Yellow'}else{'Green'})
    foreach ($item in $removed) { Write-Host ("  - " + $item) -ForegroundColor DarkGray }
}

if ($Action -in @('Repair','Audit')) {
    $rootAudit = Join-Path $PSScriptRoot 'ProjectOpsRootAudit.ps1'
    & $rootAudit -Root $Root
    if ($LASTEXITCODE -ne 0) { throw "ProjectOps root audit failed with exit code $LASTEXITCODE." }

    $registry = Join-Path $PSScriptRoot 'ProjectCommandRegistry.ps1'
    & $registry -Root $Root -Action Validate
    if ($LASTEXITCODE -ne 0) { throw "ProjectOps command registry validation failed with exit code $LASTEXITCODE." }

    $python = Resolve-ProjectOpsPython
    $sourceTool = Join-Path $Root 'tools\control\ProjectSourceAuthority.py'
    $pythonArgs = @($python.Prefix) + @($sourceTool,'--root',$Root,'validate-contract')
    & $python.Path @pythonArgs
    if ($LASTEXITCODE -ne 0) { throw "ProjectOps source authority validation failed with exit code $LASTEXITCODE." }
}

Write-Host 'PROJECTOPS ROOT TOOLING REPAIR: PASS' -ForegroundColor Green

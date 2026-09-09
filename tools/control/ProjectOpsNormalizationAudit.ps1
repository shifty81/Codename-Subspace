[CmdletBinding()]
param([Parameter(Mandatory=$true)][string]$Root)

$ErrorActionPreference = 'Stop'
Set-StrictMode -Version 2
$Root = [System.IO.Path]::GetFullPath($Root)

$commonPath = Join-Path $PSScriptRoot 'ProjectOpsCommon.psm1'
if (-not (Test-Path -LiteralPath $commonPath -PathType Leaf)) { throw "ProjectOps common module missing: $commonPath" }
Import-Module $commonPath -Force -ErrorAction Stop

$contract = Get-ProjectOpsContract -Root $Root
$artifactRoot = Get-ProjectOpsArtifactRoot -Root $Root
$auditRoot = Join-Path $artifactRoot 'audits\normalization'
New-Item -ItemType Directory -Force -Path $auditRoot | Out-Null
$stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
$jsonPath = Join-Path $auditRoot ("projectops-normalization-{0}.json" -f $stamp)
$textPath = Join-Path $auditRoot ("projectops-normalization-{0}.txt" -f $stamp)

$checks = [System.Collections.Generic.List[object]]::new()
function Add-Check([string]$Name,[bool]$Pass,[string]$Detail='') {
    $checks.Add([pscustomobject]@{ name=$Name; pass=$Pass; detail=$Detail }) | Out-Null
}

$requiredAuthorities = @(
    'tools\control\ProjectOpsCommon.psm1',
    'tools\control\ProjectOpsRootAudit.ps1',
    'tools\control\ProjectOpsMaintenance.ps1',
    'tools\control\ProjectOpsNormalizationAudit.ps1',
    'tools\control\ProjectCommandRegistry.ps1',
    'tools\control\ProjectOpsCli.py',
    'tools\control\ProjectSourceAuthority.py',
    'tools\control\ProjectShell.py',
    'tools\control\RepairGitWorkingCopy.py',
    'tools\control\UniversalTreeStage.psm1'
)
foreach($relative in $requiredAuthorities) {
    Add-Check ("authority: " + $relative) (Test-Path -LiteralPath (Join-Path $Root $relative) -PathType Leaf)
}

# Donor code must be absorbed, not copied into a parallel Havenwild root stack.
foreach($retired in @(
    'tools\control\HavenwildTools.ps1',
    'tools\control\HavenwildGateAuthority.py',
    'tools\control\PackageProject.ps1',
    'tools\control\WorkspaceMaintenance.ps1'
)) {
    Add-Check ("parallel donor authority absent: " + $retired) (-not (Test-Path -LiteralPath (Join-Path $Root $retired)))
}

$cacheFiles = @(Get-ChildItem -LiteralPath (Join-Path $Root 'tools\control') -Recurse -File -Force -ErrorAction SilentlyContinue |
    Where-Object { $_.Extension -in @('.pyc','.pyo') -or $_.FullName -match '[\\/]__pycache__[\\/]' })
Add-Check 'generated Python cache absent from control tooling' ($cacheFiles.Count -eq 0) ("count={0}" -f $cacheFiles.Count)

try {
    $commands = @(Get-ProjectOpsCommandRegistry -Root $Root)
    Add-Check 'project.control command registry valid' ($commands.Count -gt 0) ("commands={0}" -f $commands.Count)
} catch { Add-Check 'project.control command registry valid' $false $_.Exception.Message }

try {
    $snapshot = Get-ProjectOpsSourceAuthoritySnapshot -Root $Root
    Add-Check 'filesystem source authority valid' ($snapshot.cleanCheckoutContract -eq 'PASS') ("paths={0}; fingerprint={1}" -f $snapshot.pathCount,$snapshot.fingerprint)
} catch { Add-Check 'filesystem source authority valid' $false $_.Exception.Message }

try {
    $rootAudit = Join-Path $PSScriptRoot 'ProjectOpsRootAudit.ps1'
    & $rootAudit -Root $Root
    Add-Check 'root policy audit' ($LASTEXITCODE -eq 0) ("exit={0}" -f $LASTEXITCODE)
} catch { Add-Check 'root policy audit' $false $_.Exception.Message }

$rootTools = Get-Content -LiteralPath (Join-Path $Root 'SubspaceTools.ps1') -Raw
$normalizer = Get-Content -LiteralPath (Join-Path $Root 'tools\control\NormalizeGitHubAuthority.ps1') -Raw
Add-Check 'source rollup consumes governed ProjectOps stage' ($rootTools.Contains('Invoke-ProjectOpsGovernedTreeStage'))
Add-Check 'repository staging consumes governed ProjectOps stage' ($normalizer.Contains('Invoke-ProjectOpsGovernedTreeStage'))
Add-Check 'repository staging has no parallel root source lists' (-not ($normalizer.Contains('$rootFiles') -or $normalizer.Contains('$rootDirs') -or $normalizer.Contains('Copy-NormalizedDirectory')))
Add-Check 'root host avoids PowerShell automatic args assignment' (-not [regex]::IsMatch($rootTools,'(?m)^\s*\$args\s*='))
Add-Check 'root host avoids invalid LASTEXITCODE colon interpolation' (-not $rootTools.Contains(('$LASTEXITCODE' + ':')))

$failed = @($checks | Where-Object { -not $_.pass })
$payload = [ordered]@{
    schema='projectops.normalization-audit.v1'
    generatedUtc=(Get-Date).ToUniversalTime().ToString('o')
    project=[string]$contract.id
    root=$Root
    result=if($failed.Count -eq 0){'PASS'}else{'FAIL'}
    checkCount=$checks.Count
    failureCount=$failed.Count
    checks=$checks.ToArray()
}
Write-ProjectOpsAtomicJson -Path $jsonPath -Payload $payload

$lines = [System.Collections.Generic.List[string]]::new()
$lines.Add('PROJECTOPS NORMALIZATION AUDIT') | Out-Null
$lines.Add(('Project : {0}' -f $contract.name)) | Out-Null
$lines.Add(('Result  : {0}' -f $payload.result)) | Out-Null
$lines.Add(('Checks  : {0}; failures={1}' -f $checks.Count,$failed.Count)) | Out-Null
$lines.Add('------------------------------------------------------------------------') | Out-Null
foreach($check in $checks) {
    $lines.Add(('[{0}] {1}{2}' -f $(if($check.pass){'PASS'}else{'FAIL'}),$check.name,$(if($check.detail){' - '+$check.detail}else{''}))) | Out-Null
}
[System.IO.File]::WriteAllLines($textPath,$lines.ToArray(),(New-Object System.Text.UTF8Encoding($false)))

foreach($line in $lines) {
    if($line.StartsWith('[FAIL]')) { Write-Host $line -ForegroundColor Red }
    elseif($line.StartsWith('[PASS]')) { Write-Host $line -ForegroundColor Green }
    else { Write-Host $line }
}
Write-Host ("Audit JSON: {0}" -f $jsonPath) -ForegroundColor DarkGray
Write-Host ("Audit text: {0}" -f $textPath) -ForegroundColor DarkGray
if($failed.Count -gt 0){ exit 1 }
exit 0

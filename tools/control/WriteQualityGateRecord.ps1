param(
    [string]$Root=(Get-Location).Path,
    [ValidateSet('FULL','FAST')][string]$GateKind='FULL',
    [ValidateSet('PASS','FAIL')][string]$Result='FAIL',
    [string]$PassLabel='Unknown',
    [string]$SessionLog='',
    [string]$Configuration='Debug',
    [string]$ExecutionMode='incremental-authoritative'
)
$ErrorActionPreference='Stop'
$Root=[System.IO.Path]::GetFullPath($Root)
. (Join-Path $PSScriptRoot 'ControlCenterCommon.ps1')
$state=Join-Path $Root '.subspace'
$history=Join-Path $Root 'artifacts\gates\quality'
New-Item -ItemType Directory -Force -Path $history | Out-Null
$stamp=Get-Date -Format 'yyyyMMdd-HHmmss'
$gateId=("QG-{0}-{1}-{2}" -f $stamp,$GateKind.ToLowerInvariant(),([guid]::NewGuid().ToString('N').Substring(0,8)))
$gitHead=$null
$gitBranch=$null
$gitFingerprint=$null
$gitManifest=$null
$sourceSnapshot=Get-ProjectSourceAuthoritySnapshot -Root $Root -IncludePaths
$gitState=Get-ProjectGitRepositoryState -Root $Root
if ($gitState.initialized) {
    $gitBranch=[string]$gitState.branch
    if ($gitState.hasHead) {
        $gitHead=[string]$gitState.head
        $gitManifest=Get-ProjectOpsCertifiableGitManifest -Root $Root
        $gitFingerprint=if ($null -ne $gitManifest) { [string]$gitManifest.fingerprint } else { $null }
    }
}
$gitManifestVersion=if ($null -ne $gitManifest) { [int]$gitManifest.schemaVersion } else { 0 }
$gitManifestPathCount=if ($null -ne $gitManifest) { [int]$gitManifest.pathCount } else { 0 }
$gitManifestEntries=if ($null -ne $gitManifest) { @($gitManifest.entries) } else { @() }

$record=[pscustomobject]@{
    schemaVersion=4
    gateId=$gateId
    timestamp=(Get-Date).ToString('o')
    result=$Result
    kind=$GateKind
    pass=$PassLabel
    configuration=$Configuration
    executionMode=$ExecutionMode
    sessionLog=$SessionLog
    gitInitialized=[bool]$gitState.initialized
    gitHasHead=[bool]$gitState.hasHead
    gitHead=$gitHead
    gitBranch=$gitBranch
    gitFingerprint=$gitFingerprint
    gitManifestVersion=$gitManifestVersion
    gitManifestPathCount=$gitManifestPathCount
    gitManifest=$gitManifestEntries
    gitProbeAuthority='ProjectOps System.Diagnostics.Process'
    sourceFingerprint=[string]$sourceSnapshot.fingerprint
    sourcePathCount=[int]$sourceSnapshot.pathCount
    sourceManifest=@($sourceSnapshot.files)
    sourceAuthorityId=[string]$sourceSnapshot.authorityId
    requiredBootstrapPaths=@($sourceSnapshot.requiredBootstrapPaths)
    cleanCheckoutContract=[string]$sourceSnapshot.cleanCheckoutContract
    runtimeAuthority='native C++ only'
}
$path=Join-Path $history ($gateId + '.json')
$record | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $path -Encoding UTF8
if ($Result -eq 'PASS' -and $GateKind -eq 'FULL') {
    $record | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath (Join-Path $state 'last-green-quality-gate.json') -Encoding UTF8
}
Write-Host "QUALITY GATE RECORD: $path"
Write-Output $path

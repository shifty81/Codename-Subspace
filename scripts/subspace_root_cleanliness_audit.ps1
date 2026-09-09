[CmdletBinding()]
param(
    [string]$Root = (Get-Location).Path,
    [switch]$FailOnWarnings
)
$ErrorActionPreference = 'Stop'
# Historical gate compatibility marker; enforcement now lives in projectOps.rootPolicy:
# Legacy generated-output root exists outside artifacts
# Stale root-binding artifact exists

# Subspace compatibility wrapper. Root policy lives in project.control.json and
# execution lives in the project-neutral ProjectOpsRootAudit.ps1 authority.
$authority = Join-Path (Join-Path $Root 'tools\control') 'ProjectOpsRootAudit.ps1'
if (-not (Test-Path -LiteralPath $authority -PathType Leaf)) {
    throw "ProjectOps root-audit authority missing: $authority"
}
& $authority -Root $Root -FailOnWarnings:$FailOnWarnings
exit $LASTEXITCODE

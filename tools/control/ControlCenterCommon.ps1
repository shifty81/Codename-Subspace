Set-StrictMode -Version 2

# Compatibility adapter for existing Subspace control scripts.
# Generic root/tooling behavior lives in ProjectOpsCommon.psm1 so Cortex/Forge
# and other projects can consume the same primitives without Subspace naming.

$projectOpsCommon = Join-Path $PSScriptRoot 'ProjectOpsCommon.psm1'
if (-not (Test-Path -LiteralPath $projectOpsCommon -PathType Leaf)) {
    throw "ProjectOps common module missing: $projectOpsCommon"
}
Import-Module $projectOpsCommon -Force -ErrorAction Stop

function Resolve-SubspaceRoot {
    param([string]$Root)
    return Resolve-ProjectOpsRoot -Root $Root
}

function Get-SubspaceRelativePath {
    param([string]$Root,[string]$Path)
    return Get-ProjectOpsRelativePath -Root $Root -Path $Path
}

function Get-Sha256Text {
    param([string]$Text)
    return Get-ProjectOpsSha256Text -Text $Text
}

function Test-GeneratedRelativePath {
    param([string]$Relative)
    return Test-ProjectOpsGeneratedRelativePath -Root $Root -Relative $Relative
}

function Get-ProjectGitRepositoryState {
    param([string]$Root)
    return Get-ProjectOpsGitRepositoryState -Root $Root
}

function Get-CertifiableGitStatusLines {
    param([string]$Root)
    return @(Get-ProjectOpsCertifiableGitStatusLines -Root $Root)
}

function Get-CertifiableGitFingerprint {
    param([string]$Root)
    return Get-ProjectOpsCertifiableGitFingerprint -Root $Root
}

function Get-ManagedProjectFiles {
    param([string]$Root)
    return @(Get-ProjectOpsManagedFiles -Root $Root)
}

function Get-CommandVersionLine {
    param([string]$Command,[string[]]$Arguments=@('--version'))
    return Get-ProjectOpsCommandVersionLine -Command $Command -Arguments $Arguments
}

function Get-ProjectSourceAuthoritySnapshot {
    param([string]$Root,[switch]$IncludePaths)
    return Get-ProjectOpsSourceAuthoritySnapshot -Root $Root -IncludePaths:$IncludePaths
}

function Get-CertifiableSourceFingerprint {
    param([string]$Root)
    return Get-ProjectOpsCertifiableSourceFingerprint -Root $Root
}

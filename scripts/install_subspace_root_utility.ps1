param(
    [string]$Root = (Get-Location).Path,
    [string]$SourceRoot,
    [switch]$Force
)

$ErrorActionPreference = "Stop"

function Resolve-FullPath {
    param([string]$Path)
    return [System.IO.Path]::GetFullPath((Resolve-Path -LiteralPath $Path -ErrorAction Stop).Path)
}

if (-not $SourceRoot -or $SourceRoot.Trim().Length -eq 0) {
    $SourceRoot = Split-Path -Parent $PSScriptRoot
}

$Root = [System.IO.Path]::GetFullPath($Root)
$SourceRoot = [System.IO.Path]::GetFullPath($SourceRoot)

Write-Host "========================================================================"
Write-Host " SUBSPACE ROOT UTILITY INSTALLER"
Write-Host "========================================================================"
Write-Host " Root      : $Root"
Write-Host " Source    : $SourceRoot"
Write-Host "------------------------------------------------------------------------"

if (-not (Test-Path -LiteralPath $Root)) {
    throw "Root does not exist: $Root"
}

$rootLooksValid = (Test-Path -LiteralPath (Join-Path $Root "engine")) -or (Test-Path -LiteralPath (Join-Path $Root "README.md"))
if (-not $rootLooksValid -and -not $Force) {
    throw "Target does not look like a Subspace repo root. Re-run with -Force if intentional."
}

$dirs = @(
    ".subspace", ".subspace\recovery", ".subspace\control-center",
    "artifacts",
    "artifacts\debug",
    "artifacts\source",
    "artifacts\assets",
    "artifacts\patches",
    "artifacts\snapshots\source",
    "artifacts\gates\quality",
    "artifacts\gates\certifications",
    "artifacts\logs\sessions",
    "artifacts\logs\builds",
    "artifacts\logs\diagnostics",
    "artifacts\logs\updates",
    "artifacts\logs\packaging",
    "artifacts\baselines",
    "artifacts\audits",
    "artifacts\repository",
    "artifacts\root-utility",
    "artifacts\temp",
    "updates", "updates\inbox", "updates\applied", "updates\failed",
    "updates\superseded", "updates\staging", "updates\backups",
    "updates\transactions", "updates\undo",
    "scripts", "tools\control", "content\architecture"
)
foreach ($dir in $dirs) {
    New-Item -ItemType Directory -Force -Path (Join-Path $Root $dir) | Out-Null
}

$copies = @(
    @{ Source = "SubspaceTools.ps1"; Destination = "SubspaceTools.ps1" },
    @{ Source = "SubspaceTools.cmd"; Destination = "SubspaceTools.cmd" },
    @{ Source = "InstallSubspaceTools.cmd"; Destination = "InstallSubspaceTools.cmd" },

    @{ Source = "scripts\install_subspace_root_utility.ps1"; Destination = "scripts\install_subspace_root_utility.ps1" },
    @{ Source = "scripts\subspace_apply_update_inbox.ps1"; Destination = "scripts\subspace_apply_update_inbox.ps1" },
    @{ Source = "scripts\subspace_root_cleanliness_audit.ps1"; Destination = "scripts\subspace_root_cleanliness_audit.ps1" },

    @{ Source = "tools\control\ProjectOpsCommon.psm1"; Destination = "tools\control\ProjectOpsCommon.psm1" },
    @{ Source = "tools\control\ProjectCommandRegistry.ps1"; Destination = "tools\control\ProjectCommandRegistry.ps1" },
    @{ Source = "tools\control\ProjectOpsCli.py"; Destination = "tools\control\ProjectOpsCli.py" },
    @{ Source = "tools\control\ProjectSourceAuthority.py"; Destination = "tools\control\ProjectSourceAuthority.py" },
    @{ Source = "tools\control\ProjectOpsRootAudit.ps1"; Destination = "tools\control\ProjectOpsRootAudit.ps1" },
    @{ Source = "tools\control\ProjectOpsMaintenance.ps1"; Destination = "tools\control\ProjectOpsMaintenance.ps1" },
    @{ Source = "tools\control\ProjectOpsNormalizationAudit.ps1"; Destination = "tools\control\ProjectOpsNormalizationAudit.ps1" },
    @{ Source = "tools\control\ProjectOpsStaticCertification.cmake"; Destination = "tools\control\ProjectOpsStaticCertification.cmake" },
    @{ Source = "tools\control\ProjectShell.py"; Destination = "tools\control\ProjectShell.py" },
    @{ Source = "tools\control\RepairGitWorkingCopy.py"; Destination = "tools\control\RepairGitWorkingCopy.py" },
    @{ Source = "tools\control\UniversalTreeStage.psm1"; Destination = "tools\control\UniversalTreeStage.psm1" },
    @{ Source = "tools\control\ControlCenterCommon.ps1"; Destination = "tools\control\ControlCenterCommon.ps1" },
    @{ Source = "tools\control\SubspaceControlCenter.ps1"; Destination = "tools\control\SubspaceControlCenter.ps1" },
    @{ Source = "tools\control\NormalizeGitHubAuthority.ps1"; Destination = "tools\control\NormalizeGitHubAuthority.ps1" },
    @{ Source = "tools\control\AuditRoot.ps1"; Destination = "tools\control\AuditRoot.ps1" },
    @{ Source = "tools\control\InvokeRootPatchIntake.ps1"; Destination = "tools\control\InvokeRootPatchIntake.ps1" },
    @{ Source = "tools\control\WriteQualityGateRecord.ps1"; Destination = "tools\control\WriteQualityGateRecord.ps1" },
    @{ Source = "tools\control\BuildArtifactIndex.ps1"; Destination = "tools\control\BuildArtifactIndex.ps1" },
    @{ Source = "tools\control\CompareQualityGates.ps1"; Destination = "tools\control\CompareQualityGates.ps1" },
    @{ Source = "tools\control\GetDependencyStatus.ps1"; Destination = "tools\control\GetDependencyStatus.ps1" },
    @{ Source = "tools\control\InvokePatchUndo.ps1"; Destination = "tools\control\InvokePatchUndo.ps1" },

    @{ Source = "project.control.json"; Destination = "project.control.json" },
    @{ Source = "content\architecture\project_control_center_v2.json"; Destination = "content\architecture\project_control_center_v2.json" },
    @{ Source = "content\architecture\root_patch_intake_contract_v1.json"; Destination = "content\architecture\root_patch_intake_contract_v1.json" },
    @{ Source = "content\architecture\universal_tree_stage_v1.json"; Destination = "content\architecture\universal_tree_stage_v1.json" },
    @{ Source = "content\architecture\universal_artifact_layout_v1.json"; Destination = "content\architecture\universal_artifact_layout_v1.json" },
    @{ Source = "content\architecture\projectops_source_authority_v1.json"; Destination = "content\architecture\projectops_source_authority_v1.json" },
    @{ Source = "content\architecture\projectops_havenwild_absorption_v2.json"; Destination = "content\architecture\projectops_havenwild_absorption_v2.json" },
    @{ Source = "content\architecture\projectops_certification_acceleration_v1.json"; Destination = "content\architecture\projectops_certification_acceleration_v1.json" },
    @{ Source = "content\schemas\root_patch_manifest.schema.v1.json"; Destination = "content\schemas\root_patch_manifest.schema.v1.json" }
)

$installed = New-Object System.Collections.Generic.List[string]

$staticGateSource = Join-Path $SourceRoot "tools\control\static-gates"
$staticGateDestination = Join-Path $Root "tools\control\static-gates"
if (Test-Path -LiteralPath $staticGateSource -PathType Container) {
    New-Item -ItemType Directory -Force -Path $staticGateDestination | Out-Null
    Copy-Item -LiteralPath (Join-Path $staticGateSource '*') -Destination $staticGateDestination -Recurse -Force
    $installed.Add("tools\control\static-gates\*") | Out-Null
    Write-Host "[PASS] Installed tools\control\static-gates"
}

foreach ($copy in $copies) {
    $src = Join-Path $SourceRoot $copy.Source
    if (Test-Path -LiteralPath $src) {
        $dst = Join-Path $Root $copy.Destination
        New-Item -ItemType Directory -Force -Path (Split-Path -Parent $dst) | Out-Null
        Copy-Item -LiteralPath $src -Destination $dst -Force
        $installed.Add($copy.Destination) | Out-Null
        Write-Host "[PASS] Installed $($copy.Destination)"
    }
    else {
        Write-Host "[WARN] Source utility file missing from patch: $($copy.Source)" -ForegroundColor Yellow
    }
}

Get-ChildItem -LiteralPath $Root -Recurse -Include *.ps1,*.cmd -File -ErrorAction SilentlyContinue | ForEach-Object {
    try { Unblock-File -LiteralPath $_.FullName -ErrorAction SilentlyContinue } catch {}
}

$statusPath = Join-Path $Root "artifacts\root-utility\LATEST_ROOT_UTILITY_INSTALL.txt"
@(
    "Codename Subspace root utility install",
    "Timestamp: $(Get-Date -Format o)",
    "Root: $Root",
    "Source: $SourceRoot",
    "Installed files:",
    ($installed | ForEach-Object { "- $_" }),
    "",
    "Update inbox: updates\inbox",
    "Generated artifacts: artifacts\",
    "Root-drop patch ZIP workflow:",
    "1. Drop overwrite patch ZIPs into updates\inbox",
    "2. Run build/full gate normally; queued ZIPs auto-apply before CMake",
    "Preview queued patches: .\SubspaceTools.cmd -Action preview-inbox",
    "Manual apply:           .\SubspaceTools.cmd -Action apply-inbox",
    "Full gate (GREEN):      .\SubspaceTools.cmd -Action full-gate",
    "Clean-room Full gate:   .\SubspaceTools.cmd -Action clean-full-gate"
) | Set-Content -LiteralPath $statusPath

Write-Host "------------------------------------------------------------------------"
Write-Host "RESULT: PASS" -ForegroundColor Green
Write-Host "Status file: $statusPath"
Write-Host "Drop patch ZIPs into updates\inbox, then run: .\SubspaceTools.cmd -Action full-gate"
Write-Host "========================================================================"

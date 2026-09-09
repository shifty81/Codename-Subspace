<#
.SYNOPSIS
    Codename Subspace root control utility.

.DESCRIPTION
    Compact Windows-first project control script for the current Subspace source tree.
    It wraps the common project tasks from one root-level entry point:
      - status / preflight
      - clean headless build + tests
      - clean render build + tests
      - incremental render build
      - test-only runs
      - root cleanliness audit
      - log cleanup
      - debug bundle packaging
      - code/source rollup packaging
      - separate asset/content rollup packaging

    Run interactively:
      .\SubspaceTools.ps1

    Run direct actions:
      .\SubspaceTools.ps1 -Action status
      .\SubspaceTools.ps1 -Action build-headless -Clean
      .\SubspaceTools.ps1 -Action build-render -Clean
      .\SubspaceTools.ps1 -Action debug-bundle
      .\SubspaceTools.ps1 -Action source-rollup
      .\SubspaceTools.ps1 -Action asset-rollup
      .\SubspaceTools.ps1 -Action run-game
      .\SubspaceTools.ps1 -Action full-gate
      .\SubspaceTools.ps1 -Action clean-full-gate
      .\SubspaceTools.ps1 -Action project-status
      .\SubspaceTools.ps1 -Action conversion-status
      .\SubspaceTools.ps1 -Action install-root-utility
      .\SubspaceTools.ps1 -Action preview-inbox
      .\SubspaceTools.ps1 -Action apply-inbox
      .\SubspaceTools.ps1 -Action patch-status
      .\SubspaceTools.ps1 -Action fetch-shipyard

    Build actions automatically apply root-drop patch ZIPs queued in updates\inbox before configuring CMake.
    The authoritative gameplay shell is subspace_game.exe. Legacy C# is reference-only and is never built or launched by this utility.
    Use -NoAutoApplyUpdates only when deliberately diagnosing the updater itself.
#>

param(
    [ValidateSet(
        "menu",
        "status",
        "setup",
        "build-headless",
        "build-render",
        "build",
        "test",
        "clean",
        "clean-logs",
        "debug-bundle",
        "source-rollup",
        "asset-rollup",
        "root-audit",
        "verify",
        "normalize-plan",
        "normalization-status",
        "home-direction-status",
        "project-status",
        "conversion-status",
        "pass90-99-status",
        "install-root-utility",
        "preview-inbox",
        "apply-inbox",
        "patch-status",
        "fetch-shipyard",
        "supply-chain-gate",
        "source-snapshot",
        "audit-shipyard-references",
        "full-gate",
        "clean-full-gate",
        "fast-gate",
        "artifact-index",
        "compare-gates",
        "dependency-status",
        "undo-last-patch",
        "cpp-port-audit",
        "open-logs",
        "run-game",
        "run-shipyard",
        "run-smoke",
        "run-loop",
        "health",
        "control-center-self-test",
        "environment-status",
        "project-audit-package",
        "capture-baseline",
        "incremental-handoff",
        "warning-summary",
        "git-status",
        "git-init",
        "git-history",
        "git-commit-green",
        "git-push",
        "git-remote",
        "repo-authority-audit",
        "repo-authority-prepare",
        "repo-authority-publish",
        "open-project",
        "open-latest-debug",
        "open-gate-history",
        "open-latest-gate",
        "open-latest-failed-update",
        "pass-continuity",
        "source-authority",
        "projectops-inspect",
        "projectops-commands",
        "project-shell",
        "projectops-repair",
        "projectops-normalization-audit",
        "git-repair",
        "recover-pass655-674"
    )]
    [string]$Action = "menu",

    [ValidateSet("Debug", "Release", "RelWithDebInfo", "MinSizeRel")]
    [string]$Configuration = "Debug",

    [switch]$Clean,
    [switch]$SkipTests,
    [switch]$NoPause,
    [switch]$NoAutoApplyUpdates,
    [switch]$ReenteredAfterUpdate
)

$ErrorActionPreference = "Stop"

$Global:SubspaceRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$Global:EngineRoot = Join-Path $Global:SubspaceRoot "engine"
$Global:ArtifactRoot = Join-Path $Global:SubspaceRoot "artifacts"
$Global:LogsRoot = Join-Path $Global:ArtifactRoot "logs"
$Global:SessionsRoot = Join-Path $Global:LogsRoot "sessions"
$Global:BuildLogsRoot = Join-Path $Global:LogsRoot "builds"
$Global:DebugBundleRoot = Join-Path $Global:ArtifactRoot "debug"
$Global:SourceBundleRoot = Join-Path $Global:ArtifactRoot "source"
$Global:AssetBundleRoot = Join-Path $Global:ArtifactRoot "assets"
$Global:SourceSnapshotRoot = Join-Path $Global:ArtifactRoot "snapshots\source"
$Global:GateArtifactRoot = Join-Path $Global:ArtifactRoot "gates\certifications"
$Global:QualityGateHistoryRoot = Join-Path $Global:ArtifactRoot "gates\quality"
$Global:PatchArtifactRoot = Join-Path $Global:ArtifactRoot "patches"
$Global:BaselineArtifactRoot = Join-Path $Global:ArtifactRoot "baselines"
$Global:AuditArtifactRoot = Join-Path $Global:ArtifactRoot "audits"
$Global:RepositoryArtifactRoot = Join-Path $Global:ArtifactRoot "repository"
$Global:RootUtilityArtifactRoot = Join-Path $Global:ArtifactRoot "root-utility"
$Global:TempRoot = Join-Path $Global:ArtifactRoot "temp"
$Global:StateRoot = Join-Path $Global:SubspaceRoot ".subspace"
$Global:RecoveryRoot = Join-Path $Global:StateRoot "recovery"
$Global:ControlCenterStateRoot = Join-Path $Global:StateRoot "control-center"
$Global:PackagingStateRoot = $Global:BaselineArtifactRoot
$Global:SessionStart = Get-Date
$Global:Timestamp = Get-Date -Format "yyyyMMdd-HHmmss"
$Global:SessionLog = Join-Path $Global:SessionsRoot "subspace-tools-$Global:Timestamp.log"
$Global:Utf8NoBom = New-Object System.Text.UTF8Encoding -ArgumentList $false
$Global:LastActionExitCode = 0
$Global:LastDebugBundlePath = ""
$Global:LastDebugSummaryPath = ""
$Global:LastInteractiveOperation = ""
$Global:LastInteractiveOperationStatus = ""
$Global:GateExecutionMode = "none"
$Global:UiDefaultForeground = "Gray"
$Global:UiTitleForeground = "Cyan"
$Global:UiGoodForeground = "Green"
$Global:UiWarnForeground = "Yellow"
$Global:UiBadForeground = "Red"
$Global:UiMutedForeground = "DarkGray"
$Global:UiOptionForeground = "White"

function Reset-SubspaceConsolePalette {
    # Child tools and native build utilities may emit terminal reset sequences.
    # Reassert the Control Center foreground on every interactive return.
    try { $Host.UI.RawUI.ForegroundColor = [System.ConsoleColor]::$Global:UiDefaultForeground } catch {}
    try { [Console]::ForegroundColor = [System.ConsoleColor]::$Global:UiDefaultForeground } catch {}
}

function Remove-SubspaceAnsiSequences {
    param([AllowEmptyString()][string]$Text)
    if ($null -eq $Text) { return "" }
    $esc = [string][char]27
    return [regex]::Replace($Text, ($esc + '\[[0-?]*[ -/]*[@-~]'), '')
}

function Write-ChildOutputLine {
    param([AllowEmptyString()][string]$Text)
    $line = Remove-SubspaceAnsiSequences $Text
    $color = $Global:UiDefaultForeground
    if ($line -match '(?i)(\[FAIL\]|\[ERROR\]|^FAIL:|fatal error|FAILED:|tests failed|error C\d+)') {
        $color = $Global:UiBadForeground
    }
    elseif ($line -match '(?i)(\[WARN\]|^WARN:|warning:)') {
        $color = $Global:UiWarnForeground
    }
    elseif ($line -match '(?i)(\[PASS\]|^PASS:|RESULT:\s*PASS|\bREADY\b)') {
        $color = $Global:UiGoodForeground
    }
    elseif ($line -match '(?i)(\[STEP\]|^RUN:|^CMD:|QUALITY GATE|CONTROL CENTER|PROJECT HEALTH|SOURCE CONTROL|PACKAGING|MAINTENANCE|LOGS & HELP)') {
        $color = $Global:UiTitleForeground
    }
    elseif ($line -match '^[=-]{24,}$') {
        $color = $Global:UiMutedForeground
    }
    Write-Host $line -ForegroundColor $color
}

function Write-ControlStatusRow {
    param(
        [string]$Label,
        [string]$Value,
        [string]$ValueColor = "Gray"
    )
    Write-Host (" {0,-11}: " -f $Label) -ForegroundColor $Global:UiMutedForeground -NoNewline
    Write-Host $Value -ForegroundColor $ValueColor
}

function Write-MenuTitle {
    param([string]$Text)
    Write-Host $Text -ForegroundColor $Global:UiTitleForeground
    Write-Host "------------------------------------------------------------------------" -ForegroundColor $Global:UiMutedForeground
}

function Repair-StaleRootBindingResidue {
    # Pass745R3/R4 briefly created <repo>\-Root\artifacts because "-Root"
    # was bound as a positional value. Delete only the exact known residue.
    $staleRoot = Join-Path $Global:SubspaceRoot "-Root"
    if (-not (Test-Path -LiteralPath $staleRoot -PathType Container)) { return }

    $allowedFiles = @(
        "artifacts\ARTIFACT_INDEX.txt",
        "artifacts\ARTIFACT_INDEX.json"
    )
    $unexpected = @()
    foreach ($file in @(Get-ChildItem -LiteralPath $staleRoot -Recurse -File -Force -ErrorAction SilentlyContinue)) {
        $relative = [System.IO.Path]::GetRelativePath($staleRoot, $file.FullName).Replace('/', '\')
        if ($allowedFiles -notcontains $relative) { $unexpected += $relative }
    }

    if ($unexpected.Count -eq 0) {
        Remove-Item -LiteralPath $staleRoot -Recurse -Force
        Write-Log "Removed stale -Root/artifacts residue from the earlier control-script binding defect." "PASS"
    }
    else {
        Write-Log ("Refused automatic cleanup of -Root because unexpected content exists: " + ($unexpected -join ", ")) "WARN"
    }
}

function Initialize-UtilityFolders {
    foreach ($path in @(
        $Global:LogsRoot,
        $Global:SessionsRoot,
        $Global:BuildLogsRoot,
        $Global:DebugBundleRoot,
        $Global:SourceBundleRoot,
        $Global:AssetBundleRoot,
        $Global:ArtifactRoot,
        $Global:SourceSnapshotRoot,
        $Global:GateArtifactRoot,
        $Global:StateRoot,
        $Global:QualityGateHistoryRoot,
        $Global:RecoveryRoot,
        $Global:ControlCenterStateRoot,
        $Global:PackagingStateRoot,
        $Global:PatchArtifactRoot,
        $Global:BaselineArtifactRoot,
        $Global:AuditArtifactRoot,
        $Global:RepositoryArtifactRoot,
        $Global:RootUtilityArtifactRoot,
        (Join-Path $Global:LogsRoot "diagnostics"),
        (Join-Path $Global:LogsRoot "updates"),
        (Join-Path $Global:LogsRoot "packaging"),
        $Global:TempRoot,
        (Join-Path $Global:SubspaceRoot "updates"),
        (Join-Path $Global:SubspaceRoot "updates\inbox"),
        (Join-Path $Global:SubspaceRoot "updates\applied"),
        (Join-Path $Global:SubspaceRoot "updates\failed"),
        (Join-Path $Global:SubspaceRoot "updates\staging"),
        (Join-Path $Global:SubspaceRoot "updates\backups"),
        (Join-Path $Global:SubspaceRoot "updates\logs")
    )) {
        New-Item -ItemType Directory -Force -Path $path | Out-Null
    }
}

Initialize-UtilityFolders

$Global:UniversalTreeStageModule = Join-Path $Global:SubspaceRoot "tools\control\UniversalTreeStage.psm1"
if (-not (Test-Path -LiteralPath $Global:UniversalTreeStageModule)) {
    throw "Universal ProjectOps TreeStage module missing: $Global:UniversalTreeStageModule"
}
Import-Module $Global:UniversalTreeStageModule -Force -ErrorAction Stop

$Global:ProjectOpsCommonModule = Join-Path $Global:SubspaceRoot "tools\control\ProjectOpsCommon.psm1"
if (-not (Test-Path -LiteralPath $Global:ProjectOpsCommonModule -PathType Leaf)) {
    throw "ProjectOps common root-tooling module missing: $Global:ProjectOpsCommonModule"
}
Import-Module $Global:ProjectOpsCommonModule -Force -ErrorAction Stop

function Write-Log {
    param([string]$Message, [string]$Level = "INFO")
    $line = "[{0}] [{1}] {2}" -f (Get-Date -Format "yyyy-MM-dd HH:mm:ss"), $Level, $Message
    $color = switch ($Level.ToUpperInvariant()) {
        "PASS" { "Green" }
        "FAIL" { "Red" }
        "ERROR" { "Red" }
        "WARN" { "Yellow" }
        "STEP" { "Cyan" }
        default { "Gray" }
    }
    Write-Host $line -ForegroundColor $color
    [System.IO.File]::AppendAllText($Global:SessionLog, $line + [Environment]::NewLine, $Global:Utf8NoBom)
}

function Invoke-ProjectTreeStage {
    param(
        [Parameter(Mandatory=$true)][string]$Source,
        [Parameter(Mandatory=$true)][string]$Destination,
        [string[]]$ExcludeDirectories = @(),
        [string[]]$ExcludeFiles = @(),
        [string]$Label = "ProjectOps TreeStage"
    )

    Write-Log ("{0}: {1} -> {2}" -f $Label, $Source, $Destination) "INFO"
    $sink = {
        param($line)
        $clean = Remove-SubspaceAnsiSequences ([string]$line)
        Write-ChildOutputLine $clean
        [System.IO.File]::AppendAllText($Global:SessionLog, $clean + [Environment]::NewLine, $Global:Utf8NoBom)
    }
    $result = Invoke-UniversalTreeStage -Source $Source -Destination $Destination `
        -ExcludeDirectories $ExcludeDirectories -ExcludeFiles $ExcludeFiles `
        -RetryCount 1 -RetryWaitSeconds 1 -OnOutput $sink
    Write-Log ("{0} backend={1} exit={2}" -f $Label, $result.Backend, $result.ExitCode) "PASS"
    return $result
}


function Move-LegacyArtifactTree {
    param(
        [Parameter(Mandatory=$true)][string]$Source,
        [Parameter(Mandatory=$true)][string]$Destination,
        [string]$Label
    )
    if (-not (Test-Path -LiteralPath $Source -PathType Container)) { return }
    if ([System.IO.Path]::GetFullPath($Source).TrimEnd('\') -ieq [System.IO.Path]::GetFullPath($Destination).TrimEnd('\')) { return }

    New-Item -ItemType Directory -Force -Path $Destination | Out-Null
    try {
        $result = Invoke-UniversalTreeStage -Source $Source -Destination $Destination `
            -RetryCount 1 -RetryWaitSeconds 1 -Quiet
        if (-not $result.Success) { throw "TreeStage reported failure." }
        Remove-Item -LiteralPath $Source -Recurse -Force
        Write-Log ("Migrated legacy artifact tree: {0} -> {1}" -f $Source,$Destination) "PASS"
    }
    catch {
        Write-Log ("Legacy artifact migration retained source '{0}': {1}" -f $Source,$_.Exception.Message) "WARN"
    }
}

function Invoke-LegacyArtifactLayoutMigration {
    # Pass746R5 canonicalizes every user-facing generated output under artifacts\.
    $legacyDist = Join-Path $Global:SubspaceRoot "dist"
    $legacyLogs = Join-Path $Global:SubspaceRoot "logs"

    Move-LegacyArtifactTree -Source (Join-Path $legacyDist "debug") -Destination $Global:DebugBundleRoot -Label "debug"
    Move-LegacyArtifactTree -Source (Join-Path $legacyDist "source") -Destination $Global:SourceBundleRoot -Label "source"
    Move-LegacyArtifactTree -Source (Join-Path $legacyDist "assets") -Destination $Global:AssetBundleRoot -Label "assets"
    Move-LegacyArtifactTree -Source (Join-Path $legacyDist "root-utility") -Destination $Global:RootUtilityArtifactRoot -Label "root-utility"
    Move-LegacyArtifactTree -Source $legacyLogs -Destination $Global:LogsRoot -Label "logs"

    Move-LegacyArtifactTree -Source (Join-Path $Global:ArtifactRoot "source-snapshots") -Destination $Global:SourceSnapshotRoot -Label "source snapshots"
    Move-LegacyArtifactTree -Source (Join-Path $Global:ArtifactRoot "gate") -Destination $Global:GateArtifactRoot -Label "gate certifications"
    Move-LegacyArtifactTree -Source (Join-Path $Global:StateRoot "quality-gates") -Destination $Global:QualityGateHistoryRoot -Label "quality gate history"
    Move-LegacyArtifactTree -Source (Join-Path $Global:StateRoot "packaging") -Destination $Global:BaselineArtifactRoot -Label "packaging baseline"

    $legacyTemp = Join-Path $legacyDist "_temp"
    if (Test-Path -LiteralPath $legacyTemp) {
        Remove-Item -LiteralPath $legacyTemp -Recurse -Force -ErrorAction SilentlyContinue
    }

    # Preserve any unclassified historical dist output rather than deleting it.
    if (Test-Path -LiteralPath $legacyDist -PathType Container) {
        $remaining = @(Get-ChildItem -LiteralPath $legacyDist -Force -ErrorAction SilentlyContinue)
        if ($remaining.Count -gt 0) {
            Move-LegacyArtifactTree -Source $legacyDist -Destination (Join-Path $Global:ArtifactRoot "legacy\dist") -Label "legacy dist residue"
        }
        elseif (Test-Path -LiteralPath $legacyDist) {
            Remove-Item -LiteralPath $legacyDist -Force -ErrorAction SilentlyContinue
        }
    }
}

Reset-SubspaceConsolePalette
Invoke-LegacyArtifactLayoutMigration
Repair-StaleRootBindingResidue

$rootResidueMoved = @(Invoke-ProjectOpsRootPolicyMigration -Root $Global:SubspaceRoot)
foreach ($item in $rootResidueMoved) {
    Write-Log ("Migrated retired root authority into artifacts\legacy\root-residue: {0}" -f $item) "PASS"
}
$transientCachesRemoved = @(Remove-ProjectOpsTransientCaches -Root $Global:SubspaceRoot)
if ($transientCachesRemoved.Count -gt 0) {
    Write-Log ("Removed {0} transient Python/test cache directorie(s) from governed tooling roots." -f $transientCachesRemoved.Count) "PASS"
}

$Global:StepResults = [System.Collections.Generic.List[object]]::new()

function Add-StepResult {
    param(
        [string]$Name,
        [string]$Status,
        [string]$Message = ""
    )
    $Global:StepResults.Add([pscustomobject]@{ Name = $Name; Status = $Status; Message = $Message }) | Out-Null
}

function Write-StepSummary {
    Write-Host ""
    Write-Host "========================================================================"
    Write-Host " SUBSPACE ACTION SUMMARY"
    Write-Host "========================================================================"
    $failures = 0
    foreach ($result in $Global:StepResults) {
        $color = if ($result.Status -eq "PASS") { "Green" } elseif ($result.Status -eq "WARN") { "Yellow" } else { "Red" }
        if ($result.Status -eq "FAIL") { $failures++ }
        Write-Host ("[{0}] {1}" -f $result.Status, $result.Name) -ForegroundColor $color
        if ($result.Message) { Write-Host ("      {0}" -f $result.Message) -ForegroundColor DarkGray }
    }
    Write-Host "------------------------------------------------------------------------"
    if ($failures -eq 0) {
        Write-Host "RESULT: PASS" -ForegroundColor Green
    }
    else {
        Write-Host ("RESULT: FAIL ({0} failed step(s))" -f $failures) -ForegroundColor Red
    }
    Write-Host "Session log: $Global:SessionLog"
    Write-Host "========================================================================"
    return $failures
}


function Show-GateCertificationBanner {
    param(
        [ValidateSet("FULL","FAST")][string]$Kind,
        [int]$Failures
    )

    $passed = @($Global:StepResults | Where-Object { $_.Status -eq "PASS" }).Count
    $warned = @($Global:StepResults | Where-Object { $_.Status -eq "WARN" }).Count
    $failedRows = @($Global:StepResults | Where-Object { $_.Status -eq "FAIL" })
    $status = if ($Failures -eq 0) { "PASS" } else { "FAIL" }
    $color = if ($Failures -eq 0) { "Green" } else { "Red" }

    Write-Host ""
    Write-Host "========================================================================" -ForegroundColor $color
    Write-Host (" CODENAME SUBSPACE {0} QUALITY GATE - {1}" -f $Kind,$status) -ForegroundColor $color
    Write-Host "========================================================================" -ForegroundColor $color
    Write-Host (" Passed steps : {0}" -f $passed)
    Write-Host (" Warnings     : {0}" -f $warned)
    Write-Host (" Failed steps : {0}" -f $Failures) -ForegroundColor $color

    if ($failedRows.Count -gt 0) {
        Write-Host "------------------------------------------------------------------------"
        Write-Host " FAILED STAGE(S)" -ForegroundColor Red
        foreach ($row in $failedRows) {
            Write-Host (" [FAIL] {0}" -f $row.Name) -ForegroundColor Red
            if ($row.Message) {
                Write-Host ("        {0}" -f $row.Message) -ForegroundColor DarkGray
            }
        }
    }

    Write-Host "------------------------------------------------------------------------"
    Write-Host (" Session log  : {0}" -f $Global:SessionLog)
    if ($Global:LastDebugSummaryPath) {
        Write-Host (" Debug summary: {0}" -f $Global:LastDebugSummaryPath)
    }
    if ($Global:LastDebugBundlePath) {
        Write-Host (" Debug bundle : {0}" -f $Global:LastDebugBundlePath)
    }

    $interactiveContext = ($Action -eq "menu" -or $env:SUBSPACE_INTERACTIVE_PARENT -eq "1")
    if ($interactiveContext) {
        Write-Host "------------------------------------------------------------------------"
        if ($Failures -eq 0) {
            Write-Host " The interactive Control Center remains open. Press Enter in the parent window to continue." -ForegroundColor Green
        }
        else {
            Write-Host " The Control Center will remain open so this failure can be reviewed." -ForegroundColor Yellow
            Write-Host " Press Enter in the parent Control Center to return to Build & Verify." -ForegroundColor Yellow
        }
    }
    else {
        Write-Host "------------------------------------------------------------------------"
        Write-Host (" Direct/CI exit code: {0}" -f $(if ($Failures -eq 0) { 0 } else { 1 }))
    }

    Write-Host "========================================================================" -ForegroundColor $color
}

function Invoke-UtilityStep {
    param(
        [string]$Name,
        [scriptblock]$ScriptBlock,
        [switch]$ContinueOnError
    )
    Write-Log "START: $Name" "STEP"
    try {
        & $ScriptBlock
        Add-StepResult -Name $Name -Status "PASS"
        Write-Log "PASS: $Name" "PASS"
    }
    catch {
        $message = $_.Exception.Message
        Add-StepResult -Name $Name -Status "FAIL" -Message $message
        Write-Log "FAIL: $Name - $message" "FAIL"
        if (-not $ContinueOnError) { throw }
    }
}

function Get-LastGreenGateLabel {
    $path = Join-Path $Global:StateRoot "last-green-quality-gate.json"
    if (-not (Test-Path -LiteralPath $path)) { return "None" }
    try {
        $gate = Get-Content -LiteralPath $path -Raw | ConvertFrom-Json
        return ("{0} {1}" -f $gate.result, $gate.gateId)
    }
    catch { return "Unreadable" }
}

function Invoke-ControlScript {
    param([string]$RelativePath, [string[]]$Arguments = @())

    # Use the same external PowerShell argument path as project scripts.
    # Direct array splatting here binds "-Root" as a positional VALUE and can
    # produce paths such as <repo>\-Root\artifacts.
    Invoke-ProjectScript -RelativePath $RelativePath -Arguments $Arguments | Out-Null
}

function Invoke-GateRecord {
    param([ValidateSet("FULL","FAST")][string]$Kind, [ValidateSet("PASS","FAIL")][string]$Result)
    $script = Join-Path $Global:SubspaceRoot "tools\control\WriteQualityGateRecord.ps1"
    if (-not (Test-Path -LiteralPath $script)) { throw "Quality-gate recorder missing: $script" }
    & $script -Root $Global:SubspaceRoot -GateKind $Kind -Result $Result -PassLabel (Get-CurrentPassLabel) -SessionLog $Global:SessionLog -Configuration $Configuration -ExecutionMode $Global:GateExecutionMode | Out-Null
    if (-not $?) { throw "Quality-gate record failed." }
}

function Invoke-ArtifactIndex {
    Invoke-ControlScript -RelativePath "tools\control\BuildArtifactIndex.ps1" -Arguments @("-Root", $Global:SubspaceRoot)
}

function Invoke-CompareQualityGates {
    Invoke-ControlScript -RelativePath "tools\control\CompareQualityGates.ps1" -Arguments @("-Root", $Global:SubspaceRoot)
}

function Invoke-DependencyStatus {
    Invoke-ControlScript -RelativePath "tools\control\GetDependencyStatus.ps1" -Arguments @("-Root", $Global:SubspaceRoot)
}

function Invoke-UndoLastPatch {
    Invoke-ControlScript -RelativePath "tools\control\InvokePatchUndo.ps1" -Arguments @("-Root", $Global:SubspaceRoot)
}


function Invoke-StandardControlAction {
    param(
        [Parameter(Mandatory=$true)][string]$ControlAction,
        [string[]]$Arguments = @(),
        [switch]$ContinueOnError
    )
    $Global:LastInteractiveOperation = $ControlAction
    $Global:LastInteractiveOperationStatus = "RUNNING"
    $controlArgs = @("-Root", $Global:SubspaceRoot, "-Action", $ControlAction) + $Arguments
    try {
        $code = Invoke-ProjectScript -RelativePath "tools\control\SubspaceControlCenter.ps1" -Arguments $controlArgs -ContinueOnError:$ContinueOnError
        $Global:LastInteractiveOperationStatus = $(if ($code -eq 0) { "PASS" } else { "FAIL" })
        return $code
    }
    catch {
        $Global:LastInteractiveOperationStatus = "FAIL"
        throw
    }
}

function Get-QuickGitLabel {
    if (-not (Test-Path -LiteralPath (Join-Path $Global:SubspaceRoot ".git"))) { return "Not initialized" }
    $git = Get-ToolPath "git"
    if (-not $git) { return "Git unavailable" }
    try {
        Push-Location $Global:SubspaceRoot
        $branch = [string](& $git branch --show-current 2>$null)
        $dirty = @(& $git status --porcelain 2>$null)
        Pop-Location
        if ([string]::IsNullOrWhiteSpace($branch)) { $branch = "detached" }
        return ("{0} / {1}" -f $branch, $(if ($dirty.Count -gt 0) { "Modified" } else { "Clean" }))
    }
    catch {
        try { Pop-Location } catch {}
        return "Unreadable"
    }
}

function Get-QuickBaselineLabel {
    $baseline = Join-Path $Global:PackagingStateRoot "baseline.json"
    if (Test-Path -LiteralPath $baseline) {
        try {
            $data = Get-Content -LiteralPath $baseline -Raw | ConvertFrom-Json
            return ("Ready {0}" -f $data.timestamp)
        }
        catch { return "Unreadable" }
    }
    return "Missing"
}

function Get-QuickPendingUpdateCount {
    try {
        return @((Get-QueuedUpdateZips)).Count
    }
    catch { return 0 }
}

function Open-ProjectFolder {
    if ($IsWindows -or $env:OS -eq "Windows_NT") { Start-Process explorer.exe -ArgumentList $Global:SubspaceRoot | Out-Null }
    else { Write-Host $Global:SubspaceRoot }
}

function Get-LatestDebugBundlePath {
    if ($Global:LastDebugBundlePath -and (Test-Path -LiteralPath $Global:LastDebugBundlePath)) {
        return $Global:LastDebugBundlePath
    }

    foreach ($pointer in @(
        (Join-Path $Global:DebugBundleRoot "LATEST_DEBUG_BUNDLE.txt"),
        (Join-Path $Global:SubspaceRoot "LATEST_DEBUG_BUNDLE.txt")
    )) {
        if (Test-Path -LiteralPath $pointer) {
            $first = Get-Content -LiteralPath $pointer -ErrorAction SilentlyContinue | Select-Object -First 1
            if ($first -match 'Latest artifact:\s*(.+)$') {
                $candidate = $Matches[1].Trim()
                if (Test-Path -LiteralPath $candidate) { return $candidate }
            }
        }
    }

    return Get-ChildItem -LiteralPath $Global:DebugBundleRoot -Filter "Subspace_DebugBundle_*.zip" -File -ErrorAction SilentlyContinue |
        Sort-Object LastWriteTimeUtc -Descending | Select-Object -First 1 -ExpandProperty FullName
}

function Open-DebugHandoffFolder {
    param([string]$BundlePath = "")
    $candidate = $BundlePath
    if (-not $candidate) { $candidate = Get-LatestDebugBundlePath }
    if (-not $candidate -or -not (Test-Path -LiteralPath $candidate)) {
        Write-Host "No debug bundle is available to open." -ForegroundColor $Global:UiWarnForeground
        return
    }

    Write-Host ""
    Write-Host "DEBUG HANDOFF READY" -ForegroundColor $Global:UiWarnForeground
    Write-Host "Explorer is opening with the current debug ZIP selected." -ForegroundColor $Global:UiWarnForeground
    Write-Host "Drag that ZIP directly into the ChatGPT conversation." -ForegroundColor $Global:UiWarnForeground
    Write-Host $candidate -ForegroundColor $Global:UiMutedForeground

    if ($IsWindows -or $env:OS -eq "Windows_NT") {
        Start-Process explorer.exe -ArgumentList "/select,`"$candidate`"" | Out-Null
    }
    else {
        Write-Host (Split-Path -Parent $candidate)
    }
}

function Open-LatestDebugArtifact {
    Open-DebugHandoffFolder
}

function Open-GateHistoryFolder {
    New-Item -ItemType Directory -Force -Path $Global:QualityGateHistoryRoot | Out-Null
    if ($IsWindows -or $env:OS -eq "Windows_NT") { Start-Process explorer.exe -ArgumentList $Global:QualityGateHistoryRoot | Out-Null }
    else { Write-Host $Global:QualityGateHistoryRoot }
}

function Open-LatestGateRecord {
    $candidate = Get-ChildItem -LiteralPath $Global:QualityGateHistoryRoot -Filter "QG-*.json" -File -ErrorAction SilentlyContinue |
        Sort-Object LastWriteTimeUtc -Descending | Select-Object -First 1 -ExpandProperty FullName
    if (-not $candidate) { Write-Host "No quality-gate record is available."; return }
    if ($IsWindows -or $env:OS -eq "Windows_NT") { Start-Process notepad.exe -ArgumentList "`"$candidate`"" | Out-Null }
    else { Get-Content -LiteralPath $candidate }
}

function Open-LatestFailedUpdate {
    $latest = Join-Path $Global:SubspaceRoot "updates\LATEST_UPDATE_APPLY.txt"
    if (Test-Path -LiteralPath $latest) {
        if ($IsWindows -or $env:OS -eq "Windows_NT") { Start-Process notepad.exe -ArgumentList "`"$latest`"" | Out-Null }
        else { Get-Content -LiteralPath $latest }
        return
    }
    Write-Host "No update result is available."
}

function Write-Header {
    Reset-SubspaceConsolePalette
    Clear-Host
    Reset-SubspaceConsolePalette

    $nativeReady = Test-Path -LiteralPath (Join-Path $Global:EngineRoot "CMakeLists.txt")
    $gitLabel = Get-QuickGitLabel
    $baselineLabel = Get-QuickBaselineLabel
    $gateLabel = Get-LastGreenGateLabel
    $pending = Get-QuickPendingUpdateCount

    Write-Host "========================================================================" -ForegroundColor $Global:UiMutedForeground
    Write-Host " CODENAME SUBSPACE PROJECT CONTROL CENTER" -ForegroundColor $Global:UiTitleForeground
    Write-Host "========================================================================" -ForegroundColor $Global:UiMutedForeground

    Write-ControlStatusRow -Label "Repository" -Value $Global:SubspaceRoot
    $gitColor = if ($gitLabel -match '(?i)clean') { $Global:UiGoodForeground } elseif ($gitLabel -match '(?i)modified|not initialized|unavailable') { $Global:UiWarnForeground } else { $Global:UiDefaultForeground }
    Write-ControlStatusRow -Label "Git" -Value $gitLabel -ValueColor $gitColor
    Write-ControlStatusRow -Label "Native C++" -Value $(if ($nativeReady) { "Ready" } else { "Missing" }) -ValueColor $(if ($nativeReady) { $Global:UiGoodForeground } else { $Global:UiBadForeground })
    Write-ControlStatusRow -Label "Baseline" -Value $baselineLabel -ValueColor $(if ($baselineLabel -eq "Ready") { $Global:UiGoodForeground } else { $Global:UiWarnForeground })
    Write-ControlStatusRow -Label "Gate" -Value $gateLabel -ValueColor $(if ($gateLabel -match '^PASS\\b') { $Global:UiGoodForeground } elseif ($gateLabel -eq "None") { $Global:UiWarnForeground } else { $Global:UiBadForeground })
    Write-ControlStatusRow -Label "Updates" -Value ("{0} pending" -f $pending) -ValueColor $(if ($pending -eq 0) { $Global:UiGoodForeground } else { $Global:UiWarnForeground })
    Write-ControlStatusRow -Label "Config" -Value $Configuration -ValueColor $Global:UiTitleForeground
    Write-ControlStatusRow -Label "Active log" -Value $Global:SessionLog -ValueColor $Global:UiMutedForeground
    Write-Host "------------------------------------------------------------------------" -ForegroundColor $Global:UiMutedForeground
}

function Pause-ForUser {
    Reset-SubspaceConsolePalette
    if (-not $NoPause) {
        Write-Host ""
        Write-Host "Press Enter to continue" -ForegroundColor $Global:UiMutedForeground -NoNewline
        [void](Read-Host ":")
    }
    Reset-SubspaceConsolePalette
}

function Get-ToolPath {
    param([string]$Name)
    $cmd = Get-Command $Name -ErrorAction SilentlyContinue
    if ($null -eq $cmd) { return $null }
    return $cmd.Source
}

function Invoke-LoggedCommand {
    param(
        [string]$Label,
        [string]$FilePath,
        [string[]]$Arguments = @(),
        [string]$WorkingDirectory = $Global:SubspaceRoot,
        [switch]$ContinueOnError
    )

    Write-Log "RUN: $Label"
    Write-Log "CMD: $FilePath $($Arguments -join ' ')"

    # Resolve the executable while the utility's normal terminating-error policy
    # is still active. After that, native stderr is treated as process output.
    # CMake, compilers, linkers and CTest legitimately write warnings/status to
    # stderr even when they exit 0; those lines must never fail a Full Gate.
    $resolvedCommand = Get-Command $FilePath -ErrorAction Stop

    $previousErrorActionPreference = $ErrorActionPreference
    $nativePreferenceExists = Test-Path variable:PSNativeCommandUseErrorActionPreference
    $previousNativePreference = $null
    if ($nativePreferenceExists) {
        $previousNativePreference = $PSNativeCommandUseErrorActionPreference
    }

    $exitCode = $null
    Push-Location $WorkingDirectory
    try {
        $ErrorActionPreference = "Continue"
        if ($nativePreferenceExists) {
            $PSNativeCommandUseErrorActionPreference = $false
        }

        & $resolvedCommand.Source @Arguments 2>&1 | ForEach-Object {
            $outputLine = Remove-SubspaceAnsiSequences ([string]$_)
            Write-ChildOutputLine $outputLine
            [System.IO.File]::AppendAllText($Global:SessionLog, $outputLine + [Environment]::NewLine, $Global:Utf8NoBom)
        }
        $exitCode = $LASTEXITCODE
    }
    finally {
        $ErrorActionPreference = $previousErrorActionPreference
        if ($nativePreferenceExists) {
            $PSNativeCommandUseErrorActionPreference = $previousNativePreference
        }
        Pop-Location
        Reset-SubspaceConsolePalette
    }

    if ($null -eq $exitCode) { $exitCode = 0 }
    Write-Log "$Label exit code: $exitCode"

    if ($exitCode -ne 0 -and -not $ContinueOnError) {
        throw "$Label failed with exit code $exitCode. See $Global:SessionLog"
    }

    return $exitCode
}


function Get-SubspaceParallelJobs {
    $override = [string]$env:SUBSPACE_PARALLEL_JOBS
    if ($override -match '^\d+$') {
        $requested = [int]$override
        if ($requested -ge 1 -and $requested -le 64) { return $requested }
    }

    $detected = [Environment]::ProcessorCount
    if ($detected -lt 1) { $detected = 1 }

    # A moderate cap avoids pathological RAM pressure on large MSVC translation
    # units while still allowing Visual Studio/MSBuild and CTest to parallelize.
    return [Math]::Max(1, [Math]::Min($detected, 16))
}

function Invoke-CTestWithoutCrashDialogs {
    param(
        [string]$Label,
        [string]$BuildDirectory,
        [switch]$ContinueOnError
    )

    $restoreErrorMode = $null
    $windowsHost = ($IsWindows -or $env:OS -eq "Windows_NT")

    if ($windowsHost) {
        if (-not ("Subspace.Tools.NativeErrorMode" -as [type])) {
            Add-Type -TypeDefinition @"
using System;
using System.Runtime.InteropServices;

namespace Subspace.Tools
{
    public static class NativeErrorMode
    {
        [DllImport("kernel32.dll")]
        public static extern uint GetErrorMode();

        [DllImport("kernel32.dll")]
        public static extern uint SetErrorMode(uint uMode);
    }
}
"@
        }

        # Child processes inherit the process error mode unless explicitly
        # launched with CREATE_DEFAULT_ERROR_MODE. CTest and the test EXEs
        # therefore inherit SEM_NOGPFAULTERRORBOX and cannot strand Full Gate
        # behind the Windows "has stopped working" fault dialog.
        $SEM_FAILCRITICALERRORS  = [uint32]0x0001
        $SEM_NOGPFAULTERRORBOX   = [uint32]0x0002
        $SEM_NOOPENFILEERRORBOX = [uint32]0x8000
        $restoreErrorMode = [Subspace.Tools.NativeErrorMode]::GetErrorMode()
        $testErrorMode = [uint32]($restoreErrorMode -bor $SEM_FAILCRITICALERRORS -bor $SEM_NOGPFAULTERRORBOX -bor $SEM_NOOPENFILEERRORBOX)
        [void][Subspace.Tools.NativeErrorMode]::SetErrorMode($testErrorMode)
        Write-Log ("CTest crash-dialog suppression active (error mode 0x{0:X8})." -f $testErrorMode) "INFO"
    }

    try {
        $parallelJobs = Get-SubspaceParallelJobs
        Write-Log ("CTest parallelism: {0} worker(s)." -f $parallelJobs) "INFO"
        return Invoke-LoggedCommand -Label $Label -FilePath "ctest" -Arguments @(
            "--test-dir", $BuildDirectory,
            "-C", $Configuration,
            "--parallel", $parallelJobs,
            "--output-on-failure",
            "--timeout", "120"
        ) -WorkingDirectory $Global:SubspaceRoot -ContinueOnError:$ContinueOnError
    }
    finally {
        if ($windowsHost -and $null -ne $restoreErrorMode) {
            [void][Subspace.Tools.NativeErrorMode]::SetErrorMode([uint32]$restoreErrorMode)
        }
    }
}

function Invoke-ProjectScript {
    param(
        [string]$RelativePath,
        [string[]]$Arguments = @(),
        [switch]$ContinueOnError
    )

    $script = Join-Path $Global:SubspaceRoot $RelativePath
    if (-not (Test-Path -LiteralPath $script)) {
        throw "Missing project script: $RelativePath"
    }

    $runner = Get-ToolPath "pwsh"
    if (-not $runner) { $runner = Get-ToolPath "powershell" }
    if (-not $runner) { throw "PowerShell runner not found." }

    $scriptArgs = @("-NoProfile", "-ExecutionPolicy", "Bypass", "-File", $script) + $Arguments
    Invoke-LoggedCommand -Label $RelativePath -FilePath $runner -Arguments $scriptArgs -WorkingDirectory $Global:SubspaceRoot -ContinueOnError:$ContinueOnError
}


function Test-IsPatchZipName {
    param([string]$Name)
    if ([string]::IsNullOrWhiteSpace($Name)) { return $false }
    if ($Name -notmatch '(?i)\.zip$') { return $false }
    # Never auto-apply generated debug/source artifacts as patch payloads.
    if ($Name -match '(?i)FullSource|SourceRollup|DebugBundle|BuildRollup|CompleteSource|Full_Source') { return $false }
    # Project-standard handoff names. These are root-overwrite patch/update ZIPs.
    # Accept both historical names (RootDrop) and the explicit project-standard
    # ROOT_DROP / ROOT_DROP_PATCH spellings used by current handoff ZIPs.
    if ($Name -match '(?i)^Subspace_(Pass|Patch|Hotfix|Root[_-]?Drop(?:[_-]?Patch)?|Update|Rollup).*\.zip$') { return $true }
    if ($Name -match '(?i)^Codename_Subspace_(Pass|Patch|Hotfix|Root[_-]?Drop(?:[_-]?Patch)?|Update).*\.zip$') { return $true }
    return $false
}

function Get-InboxUpdateZips {
    $inbox = Join-Path $Global:SubspaceRoot "updates\inbox"
    if (-not (Test-Path -LiteralPath $inbox)) { return @() }
    return @(Get-ChildItem -LiteralPath $inbox -Filter *.zip -File -ErrorAction SilentlyContinue |
        Where-Object { Test-IsPatchZipName $_.Name } |
        Sort-Object Name)
}

function Get-RootDropUpdateZips {
    return @(Get-ChildItem -LiteralPath $Global:SubspaceRoot -Filter *.zip -File -ErrorAction SilentlyContinue |
        Where-Object { Test-IsPatchZipName $_.Name } |
        Sort-Object Name)
}

function Get-QueuedUpdateZips {
    return @(Get-InboxUpdateZips)
}

function Write-PatchHandoffStatus {
    param([string]$Reason = "status")
    Initialize-UtilityFolders
    $rootDrops = @(Get-RootDropUpdateZips)
    $inbox = @(Get-InboxUpdateZips)
    Write-Host ""
    Write-Host "PATCH HANDOFF CHECK" -ForegroundColor Cyan
    Write-Host "  Reason       : $Reason"
    Write-Host "  Root drop    : $Global:SubspaceRoot"
    Write-Host "  Inbox        : $(Join-Path $Global:SubspaceRoot 'updates\inbox')"
    Write-Host "  Root ZIPs    : $($rootDrops.Count)"
    Write-Host "  Inbox ZIPs   : $($inbox.Count)"
    if ($rootDrops.Count -gt 0) {
        Write-Host "  Root-drop patches that will be queued before build:" -ForegroundColor Yellow
        foreach ($patch in $rootDrops) { Write-Host ("    - {0}" -f $patch.Name) -ForegroundColor Yellow }
    }
    if ($inbox.Count -gt 0) {
        Write-Host "  Inbox patches that will be applied before build:" -ForegroundColor Yellow
        foreach ($patch in $inbox) { Write-Host ("    - {0}" -f $patch.Name) -ForegroundColor Yellow }
    }
    if ($rootDrops.Count -eq 0 -and $inbox.Count -eq 0) {
        Write-Host "  No patch ZIPs queued." -ForegroundColor Green
    }
}

function Invoke-AutoApplyUpdateInbox {
    param([string]$Reason = "build")

    Initialize-UtilityFolders
    Write-PatchHandoffStatus -Reason $Reason

    if ($NoAutoApplyUpdates) {
        Write-Log "Auto patch handoff skipped by -NoAutoApplyUpdates." "WARN"
        return
    }

    $rootDrops = @(Get-RootDropUpdateZips)
    $inboxQueued = @(Get-InboxUpdateZips)
    if ($rootDrops.Count -eq 0 -and $inboxQueued.Count -eq 0) {
        Write-Log "No root-drop or inbox patch ZIPs queued." "PASS"
        return
    }

    $script = Join-Path $Global:SubspaceRoot "scripts\subspace_apply_update_inbox.ps1"
    if (-not (Test-Path -LiteralPath $script)) {
        throw "Patch ZIPs are queued, but the root-drop update processor is missing: $script"
    }

    $runner = Get-ToolPath "pwsh"
    if (-not $runner) { $runner = Get-ToolPath "powershell" }
    if (-not $runner) { throw "PowerShell runner not found for patch handoff apply." }

    $updateArgs = @("-NoProfile", "-ExecutionPolicy", "Bypass", "-File", $script, "-Root", $Global:SubspaceRoot)
    Invoke-LoggedCommand -Label "Auto apply root-drop/inbox patch handoff" -FilePath $runner -Arguments $updateArgs -WorkingDirectory $Global:SubspaceRoot | Out-Null

    $remainingRoot = @(Get-RootDropUpdateZips)
    $remainingInbox = @(Get-InboxUpdateZips)
    if ($remainingRoot.Count -gt 0 -or $remainingInbox.Count -gt 0) {
        throw "Patch handoff still contains $($remainingRoot.Count) root ZIP(s) and $($remainingInbox.Count) inbox ZIP(s) after apply. Build stopped before CMake."
    }

    Write-Log "Root-drop/inbox patch handoff applied before $Reason." "PASS"
}

function Get-BuildDirectory {
    param([switch]$Headless)
    if ($Headless) { return (Join-Path $Global:EngineRoot "build-headless") }
    return (Join-Path $Global:EngineRoot "build")
}

function Invoke-Status {
    Write-Header
    Write-Log "Running status / preflight."

    $tools = @("git", "cmake", "ctest", "msbuild", "pwsh", "powershell")
    foreach ($tool in $tools) {
        $path = Get-ToolPath $tool
        if ($path) { Write-Log ("OK      {0} -> {1}" -f $tool, $path) }
        else { Write-Log ("MISSING {0}" -f $tool) "WARN" }
    }

    if (Test-Path -LiteralPath (Join-Path $Global:SubspaceRoot ".git")) {
        Invoke-LoggedCommand -Label "git status" -FilePath "git" -Arguments @("status", "--short") -WorkingDirectory $Global:SubspaceRoot -ContinueOnError | Out-Null
        Invoke-LoggedCommand -Label "git branch" -FilePath "git" -Arguments @("branch", "--show-current") -WorkingDirectory $Global:SubspaceRoot -ContinueOnError | Out-Null
    }
    else {
        Write-Log "Git repository metadata not found. This is okay for a downloaded source rollup." "WARN"
    }

    foreach ($dir in @((Get-BuildDirectory -Headless), (Get-BuildDirectory))) {
        if (Test-Path -LiteralPath $dir) { Write-Log "Build dir exists: $dir" }
        else { Write-Log "Build dir missing: $dir" }
    }

    Invoke-ProjectScript -RelativePath "scripts\subspace_root_cleanliness_audit.ps1" -Arguments @("-Root", $Global:SubspaceRoot) -ContinueOnError | Out-Null
}

function Clear-Logs {
    param([switch]$PreserveCurrent)

    if (-not (Test-Path -LiteralPath $Global:LogsRoot)) {
        Initialize-UtilityFolders
        return
    }

    Write-Log "Cleaning logs folder."
    Get-ChildItem -LiteralPath $Global:LogsRoot -Force -ErrorAction SilentlyContinue | ForEach-Object {
        if ($PreserveCurrent -and $_.FullName -eq $Global:SessionLog) { return }
        if ($PreserveCurrent -and $_.PSIsContainer -and $_.FullName -eq $Global:SessionsRoot) {
            Get-ChildItem -LiteralPath $_.FullName -Force -ErrorAction SilentlyContinue | Where-Object { $_.FullName -ne $Global:SessionLog } | Remove-Item -Recurse -Force -ErrorAction SilentlyContinue
            return
        }
        Remove-Item -LiteralPath $_.FullName -Recurse -Force -ErrorAction SilentlyContinue
    }

    Initialize-UtilityFolders
    if (-not (Test-Path -LiteralPath $Global:SessionLog)) {
        New-Item -ItemType File -Force -Path $Global:SessionLog | Out-Null
    }
}

function Invoke-CleanBuildOutputs {
    Write-Header
    Write-Log "Cleaning build outputs."

    $paths = @(
        (Join-Path $Global:EngineRoot "build"),
        (Join-Path $Global:EngineRoot "build-headless"),
        (Join-Path $Global:SubspaceRoot "build"),
        (Join-Path $Global:SubspaceRoot "out"),
        (Join-Path $Global:SubspaceRoot ".vs")
    )

    foreach ($path in $paths) {
        if (Test-Path -LiteralPath $path) {
            Write-Log "Removing $path"
            Remove-Item -LiteralPath $path -Recurse -Force -ErrorAction SilentlyContinue
        }
    }

    $avorion = Join-Path $Global:SubspaceRoot "AvorionLike"
    if (Test-Path -LiteralPath $avorion) {
        Get-ChildItem -LiteralPath $avorion -Recurse -Directory -Force -ErrorAction SilentlyContinue |
            Where-Object { $_.Name -in @("bin", "obj") } |
            ForEach-Object {
                Write-Log "Removing $($_.FullName)"
                Remove-Item -LiteralPath $_.FullName -Recurse -Force -ErrorAction SilentlyContinue
            }
    }
}

function Invoke-CMakeBuild {
    param([switch]$Headless, [switch]$CleanFirst, [switch]$TestsOnly)

    if (-not (Test-Path -LiteralPath $Global:EngineRoot)) {
        throw "Engine folder missing: $Global:EngineRoot"
    }

    $buildDir = Get-BuildDirectory -Headless:$Headless
    if ($CleanFirst -and (Test-Path -LiteralPath $buildDir)) {
        Write-Log "Removing build directory: $buildDir"
        Remove-Item -LiteralPath $buildDir -Recurse -Force
    }
    New-Item -ItemType Directory -Force -Path $buildDir | Out-Null

    if (-not $TestsOnly) {
        Write-Host ""
        Write-Host "BUILD SCOPE" -ForegroundColor Cyan
        Write-Host "  Active C++ CMake source : $Global:EngineRoot"
        Write-Host "  Build directory          : $buildDir"
        Write-Host "  Expected C++ targets     : subspace_engine, subspace_game, subspace_tests, subspace_milestone_tests, subspace_production_tests, subspace_forward_tests"
        Write-Host "  Not compiled by CMake    : docs/, content assets, legacy C# reference archive (not compiled)"
        Write-Host "  Full-project checks      : use -Action full-gate or project-status"
        Write-Host ""

        $headlessValue = if ($Headless) { "ON" } else { "OFF" }
        $openGlValue = if ($Headless) { "OFF" } else { "ON" }

        Invoke-LoggedCommand -Label "CMake configure" -FilePath "cmake" -Arguments @(
            "-S", $Global:EngineRoot,
            "-B", $buildDir,
            "-DSUBSPACE_HEADLESS=$headlessValue",
            "-DSUBSPACE_BUILD_OPENGL=$openGlValue",
            "-DSUBSPACE_BUILD_TESTS=ON"
        ) -WorkingDirectory $Global:SubspaceRoot | Out-Null

        $parallelJobs = Get-SubspaceParallelJobs
        Write-Log ("Native build parallelism: {0} worker(s)." -f $parallelJobs) "INFO"
        Invoke-LoggedCommand -Label "CMake build" -FilePath "cmake" -Arguments @(
            "--build", $buildDir,
            "--config", $Configuration,
            "--parallel", $parallelJobs
        ) -WorkingDirectory $Global:SubspaceRoot | Out-Null
    }

    if (-not $SkipTests) {
        Invoke-CTestWithoutCrashDialogs -Label "CTest" -BuildDirectory $buildDir | Out-Null
    }

    Write-Log "C++ CMake build/check completed for $buildDir" "PASS"
}

function Invoke-BuildHeadless {
    Write-Header
    Clear-Logs -PreserveCurrent
    $Global:StepResults.Clear()
    Invoke-UtilityStep -Name "Auto-apply root/inbox patches before build" -ScriptBlock { Invoke-AutoApplyUpdateInbox -Reason "headless build" }
    Invoke-UtilityStep -Name "Headless C++ configure/build/test" -ScriptBlock { Invoke-CMakeBuild -Headless -CleanFirst:$Clean }
    [void](Write-StepSummary)
}

function Invoke-BuildRender {
    Write-Header
    Clear-Logs -PreserveCurrent
    $Global:StepResults.Clear()
    Invoke-UtilityStep -Name "Auto-apply root/inbox patches before build" -ScriptBlock { Invoke-AutoApplyUpdateInbox -Reason "render build" }
    Invoke-UtilityStep -Name "Pass/source continuity audit" -ScriptBlock { Invoke-PassContinuityAudit }
    Invoke-UtilityStep -Name "Render C++ configure/build/test" -ScriptBlock { Invoke-CMakeBuild -CleanFirst:$Clean }
    if ($IsWindows -or $env:OS -eq "Windows_NT") {
        Invoke-UtilityStep -Name "Native C++ game executable present" -ScriptBlock {
            $exe = Find-SubspaceGameExecutable
            if (-not $exe) { throw "subspace_game.exe was not found after render build." }
            Write-Log "Native C++ game found: $exe" "PASS"
        }
    }
    [void](Write-StepSummary)
}

function Invoke-TestsOnly {
    Write-Header
    $headlessBuild = Get-BuildDirectory -Headless
    $renderBuild = Get-BuildDirectory

    if (Test-Path -LiteralPath $headlessBuild) {
        Invoke-CTestWithoutCrashDialogs -Label "CTest headless" -BuildDirectory $headlessBuild -ContinueOnError | Out-Null
    }
    else {
        Write-Log "Headless build directory not found: $headlessBuild" "WARN"
    }

    if (Test-Path -LiteralPath $renderBuild) {
        Invoke-CTestWithoutCrashDialogs -Label "CTest render" -BuildDirectory $renderBuild -ContinueOnError | Out-Null
    }
    else {
        Write-Log "Render build directory not found: $renderBuild" "WARN"
    }
}

function Find-SubspaceGameExecutable {
    $candidates = @(
        (Join-Path (Get-BuildDirectory) "subspace_game.exe"),
        (Join-Path (Get-BuildDirectory) "$Configuration\subspace_game.exe"),
        (Join-Path (Get-BuildDirectory -Headless) "subspace_game.exe"),
        (Join-Path (Get-BuildDirectory -Headless) "$Configuration\subspace_game.exe")
    )

    foreach ($candidate in $candidates) {
        if (Test-Path -LiteralPath $candidate) { return $candidate }
    }

    $found = Get-ChildItem -LiteralPath $Global:EngineRoot -Filter "subspace_game.exe" -Recurse -File -ErrorAction SilentlyContinue |
        Where-Object { $_.FullName -notmatch "CMakeFiles" } |
        Sort-Object LastWriteTime -Descending |
        Select-Object -First 1
    if ($found) { return $found.FullName }

    return $null
}

function Invoke-RunSubspaceGame {
    param([string[]]$GameArguments = @())

    Write-Header
    $exe = Find-SubspaceGameExecutable
    if (-not $exe) {
        throw "subspace_game.exe was not found. Run build-render or build-headless first."
    }

    Write-Log "Running game executable: $exe"
    Write-Log "Game args: $($GameArguments -join ' ')"
    Invoke-LoggedCommand -Label "subspace_game" -FilePath $exe -Arguments $GameArguments -WorkingDirectory (Split-Path -Parent $exe) | Out-Null
}


function Test-ZipArchiveReadable {
    param(
        [string]$ArchivePath,
        [switch]$ReadEveryEntry
    )

    if (-not (Test-Path -LiteralPath $ArchivePath)) {
        throw "Archive is missing: $ArchivePath"
    }

    Add-Type -AssemblyName System.IO.Compression -ErrorAction SilentlyContinue
    Add-Type -AssemblyName System.IO.Compression.FileSystem -ErrorAction SilentlyContinue

    $archive = $null
    $entryCount = 0
    $expandedBytes = [int64]0
    $maxEntryLength = 0
    $buffer = New-Object byte[] (1024 * 1024)

    try {
        $archive = [System.IO.Compression.ZipFile]::OpenRead($ArchivePath)
        foreach ($entry in $archive.Entries) {
            $entryCount++
            $expandedBytes += [int64]$entry.Length
            if ($entry.FullName.Length -gt $maxEntryLength) {
                $maxEntryLength = $entry.FullName.Length
            }

            # ZIP's portable path separator is '/'. Backslash entry names were
            # tolerated by many tools but made the rollups less portable.
            if ($entry.FullName -match '\\') {
                throw "Archive contains a non-portable backslash entry path: $($entry.FullName)"
            }

            if ($ReadEveryEntry -and $entry.Length -gt 0) {
                $stream = $null
                try {
                    $stream = $entry.Open()
                    while ($stream.Read($buffer, 0, $buffer.Length) -gt 0) { }
                }
                finally {
                    if ($stream) { $stream.Dispose() }
                }
            }
        }
    }
    catch {
        throw "Archive validation failed for '$ArchivePath': $($_.Exception.Message)"
    }
    finally {
        if ($archive) { $archive.Dispose() }
    }

    if ($entryCount -le 0) {
        throw "Archive contains no entries: $ArchivePath"
    }

    $zipInfo = Get-Item -LiteralPath $ArchivePath
    Write-Log ("Archive verified: {0} entries, {1:N1} MiB ZIP, {2:N1} MiB expanded, max entry path {3} chars." -f `
        $entryCount, ($zipInfo.Length / 1MB), ($expandedBytes / 1MB), $maxEntryLength) "PASS"
}

function Compress-StagedFolder {
    param(
        [string]$StagePath,
        [string]$DestinationZip,
        [switch]$IncludeBaseDirectory,
        [switch]$CleanupOnSuccess
    )

    if (-not (Test-Path -LiteralPath $StagePath)) {
        throw "Staging folder missing: $StagePath"
    }

    $destinationDir = Split-Path -Parent $DestinationZip
    New-Item -ItemType Directory -Force -Path $destinationDir | Out-Null
    if (Test-Path -LiteralPath $DestinationZip) {
        Remove-Item -LiteralPath $DestinationZip -Force
    }

    Add-Type -AssemblyName System.IO.Compression -ErrorAction SilentlyContinue
    Add-Type -AssemblyName System.IO.Compression.FileSystem -ErrorAction SilentlyContinue

    $archive = $null
    $fileStream = $null
    try {
        $stageItem = Get-Item -LiteralPath $StagePath -Force
        $basePrefix = ""
        if ($IncludeBaseDirectory) {
            $basePrefix = $stageItem.Name.TrimEnd([char[]]'\/') + "/"
        }

        $fileStream = [System.IO.File]::Open(
            $DestinationZip,
            [System.IO.FileMode]::CreateNew,
            [System.IO.FileAccess]::ReadWrite,
            [System.IO.FileShare]::None
        )
        $archive = New-Object System.IO.Compression.ZipArchive(
            $fileStream,
            [System.IO.Compression.ZipArchiveMode]::Create,
            $false
        )

        $seenEntries = @{}
        foreach ($file in @(Get-ChildItem -LiteralPath $StagePath -Recurse -File -Force -ErrorAction Stop | Sort-Object FullName)) {
            $relative = $file.FullName.Substring($stageItem.FullName.Length).TrimStart([char[]]'\/')
            $entryName = ($basePrefix + $relative).Replace('\', '/')

            if ($seenEntries.ContainsKey($entryName.ToLowerInvariant())) {
                throw "Duplicate ZIP entry after path normalization: $entryName"
            }
            $seenEntries[$entryName.ToLowerInvariant()] = $true

            [void][System.IO.Compression.ZipFileExtensions]::CreateEntryFromFile(
                $archive,
                $file.FullName,
                $entryName,
                [System.IO.Compression.CompressionLevel]::Optimal
            )
        }

        if ($seenEntries.Count -eq 0) {
            throw "Staging folder contains no files: $StagePath"
        }
    }
    catch {
        Write-Log "Zip creation failed. Staging folder preserved for inspection: $StagePath" "ERROR"
        throw
    }
    finally {
        if ($archive) { $archive.Dispose() }
        if ($fileStream) { $fileStream.Dispose() }
    }

    if (-not (Test-Path -LiteralPath $DestinationZip)) {
        throw "Zip creation reported success but output is missing: $DestinationZip"
    }

    $zipInfo = Get-Item -LiteralPath $DestinationZip
    if ($zipInfo.Length -le 0) {
        throw "Zip creation produced an empty file: $DestinationZip"
    }

    # Fully stream every entry once before declaring a rollup good. This catches
    # truncated/broken archives immediately instead of discovering them later in Explorer.
    Test-ZipArchiveReadable -ArchivePath $DestinationZip -ReadEveryEntry

    if ($CleanupOnSuccess) {
        Remove-Item -LiteralPath $StagePath -Recurse -Force -ErrorAction SilentlyContinue
    }

    return $DestinationZip
}


function Write-Utf8TextFile {
    param(
        [string]$Path,
        [object[]]$Lines
    )

    $parent = Split-Path -Parent $Path
    if ($parent) { New-Item -ItemType Directory -Force -Path $parent | Out-Null }

    # Windows PowerShell 5.1 can throw "Argument types do not match" when
    # generic collections are implicitly rebound to WriteAllLines overloads.
    # Normalize explicitly to a real System.String[] first.
    [string[]]$normalizedLines = @($Lines | ForEach-Object { [string]$_ })
    [System.IO.File]::WriteAllLines($Path, $normalizedLines, $Global:Utf8NoBom)
}

function Write-LatestArtifactPointer {
    param(
        [string]$PointerFile,
        [string]$ArtifactPath,
        [string]$SummaryPath = ""
    )

    [object[]]$lines = @()
    $lines += "Latest artifact: $ArtifactPath"
    $lines += "Timestamp: $Global:Timestamp"
    $lines += "Root: $Global:SubspaceRoot"
    if ($SummaryPath) {
        $lines += "Summary artifact: $SummaryPath"
        if (Test-Path -LiteralPath $SummaryPath) {
            $lines += ""
            $lines += "========================================================================"
            $lines += " EMBEDDED DEBUG CERTIFICATION SUMMARY"
            $lines += "========================================================================"
            foreach ($line in @(Get-Content -LiteralPath $SummaryPath -ErrorAction SilentlyContinue)) {
                $lines += [string]$line
            }
        }
    }
    Write-Utf8TextFile -Path $PointerFile -Lines $lines
}

function Get-PreferredDebugLastTestLog {
    $candidates = @(
        (Join-Path $Global:EngineRoot "build\Testing\Temporary\LastTest.log"),
        (Join-Path $Global:EngineRoot "build-headless\Testing\Temporary\LastTest.log")
    )
    foreach ($candidate in $candidates) {
        if (Test-Path -LiteralPath $candidate) { return $candidate }
    }
    return $null
}

function Get-DebugTestSummaryRecords {
    [object[]]$records = @()
    $logPath = Get-PreferredDebugLastTestLog
    if (-not $logPath) { return @() }

    foreach ($line in @(Get-Content -LiteralPath $logPath -ErrorAction SilentlyContinue)) {
        if ([string]$line -match '===\s*(?:(?<Name>.+?)\s+)?Summary:\s*(?<Passed>\d+)\s+passed,\s*(?<Failed>\d+)\s+failed\s*===') {
            $summaryName = if ($Matches['Name']) { $Matches['Name'].Trim() } else { "Historical regression" }
            $records += [pscustomobject]@{
                Name = $summaryName
                Passed = [int]$Matches['Passed']
                Failed = [int]$Matches['Failed']
                Source = $logPath
            }
        }
    }

    foreach ($record in $records) { Write-Output $record }
}

function Get-DebugCTestRecord {
    $passed = $null
    $failed = $null
    $total = $null
    if (Test-Path -LiteralPath $Global:SessionLog) {
        foreach ($line in @(Get-Content -LiteralPath $Global:SessionLog -ErrorAction SilentlyContinue)) {
            if ([string]$line -match '(?<Percent>\d+)% tests passed,\s*(?<Failed>\d+) tests failed out of (?<Total>\d+)') {
                $failed = [int]$Matches['Failed']
                $total = [int]$Matches['Total']
                $passed = $total - $failed
            }
        }
    }

    if ($null -eq $total) {
        $lastTest = Get-PreferredDebugLastTestLog
        if ($lastTest) {
            $content = @(Get-Content -LiteralPath $lastTest -ErrorAction SilentlyContinue)
            $passed = @($content | Where-Object { [string]$_ -match '^Test Passed\.$' }).Count
            $failed = @($content | Where-Object { [string]$_ -match '^Test Failed\.$' }).Count
            $total = $passed + $failed
        }
    }

    return [pscustomobject]@{ Passed = $passed; Failed = $failed; Total = $total }
}

function Get-DebugStepStatus {
    param([string]$Name)
    $match = @($Global:StepResults | Where-Object { $_.Name -eq $Name } | Select-Object -Last 1)
    if ($match.Count -eq 0) { return "NOT RUN" }
    return [string]$match[0].Status
}

function Get-CurrentPassLabel {
    $maxPass = 0
    $docsRoot = Join-Path $Global:SubspaceRoot "docs"
    if (Test-Path -LiteralPath $docsRoot) {
        foreach ($file in @(Get-ChildItem -LiteralPath $docsRoot -Recurse -File -ErrorAction SilentlyContinue)) {
            # Match the same bounded pass-range grammar used by the update-inbox
            # authority. PASS497_506 means Pass506; date suffixes such as
            # PASS49_20260822 must not become impossible project pass numbers.
            foreach ($match in [regex]::Matches($file.Name, '(?i)PASS(?<First>\d+)(?:[_-](?<Last>\d+))?')) {
                $first = [int64]$match.Groups['First'].Value
                if ($first -gt 0 -and $first -le 9999 -and $first -gt $maxPass) {
                    $maxPass = [int]$first
                }
                if ($match.Groups['Last'].Success) {
                    $last = [int64]$match.Groups['Last'].Value
                    if ($last -gt 0 -and $last -le 9999 -and $last -gt $maxPass) {
                        $maxPass = [int]$last
                    }
                }
            }
        }
    }
    if ($maxPass -gt 0) { return "Pass$maxPass" }
    return "UNKNOWN"
}

function New-DebugCertificationSummary {
    param(
        [string]$OutputPath,
        [string]$FinalZipPath
    )

    [object[]]$lines = @()
    $testRecords = @(Get-DebugTestSummaryRecords)
    $ctest = Get-DebugCTestRecord
    $gameExe = Find-SubspaceGameExecutable
    $failedSteps = @($Global:StepResults | Where-Object { $_.Status -eq "FAIL" })
    $buildStepStatus = Get-DebugStepStatus "Render C++ configure/build/test"
    $testEvidenceScope = if ($Global:StepResults.Count -eq 0) {
        "LAST AVAILABLE (standalone bundle; not tied to an in-memory gate)"
    } elseif ($buildStepStatus -eq "PASS") {
        "CURRENT FULL GATE"
    } else {
        "NOT CERTIFIED FOR CURRENT GATE (build/test step did not pass)"
    }
    $currentOperation = if (-not [string]::IsNullOrWhiteSpace($Global:LastInteractiveOperation)) {
        $Global:LastInteractiveOperation
    } else {
        $Action
    }
    $currentOperationStatus = if (-not [string]::IsNullOrWhiteSpace($Global:LastInteractiveOperationStatus)) {
        $Global:LastInteractiveOperationStatus
    } elseif ($failedSteps.Count -gt 0) {
        "FAIL"
    } elseif ($Action -in @("full-gate","fast-gate") -and $Global:StepResults.Count -gt 0) {
        "PASS"
    } else {
        "UNKNOWN"
    }
    $overall = $currentOperationStatus

    $latestGreenLabel = "NONE"
    $latestGreenPath = Join-Path $Global:StateRoot "last-green-quality-gate.json"
    if (Test-Path -LiteralPath $latestGreenPath) {
        try {
            $latestGreen = Get-Content -LiteralPath $latestGreenPath -Raw | ConvertFrom-Json
            $fp = if ($latestGreen.gitFingerprint) { "git-fingerprinted" } else { "pre-git-fingerprint" }
            $latestGreenLabel = ("{0} {1} ({2})" -f $latestGreen.result, $latestGreen.gateId, $fp)
        } catch {
            $latestGreenLabel = "UNREADABLE"
        }
    }

    $lines += "CODENAME SUBSPACE DEBUG CERTIFICATION SUMMARY"
    $lines += "========================================================================"
    $lines += "Timestamp              : $Global:Timestamp"
    $lines += "Repository root        : $Global:SubspaceRoot"
    $lines += "Configuration          : $Configuration"
    $lines += "Detected pass          : $(Get-CurrentPassLabel)"
    $lines += "Current operation      : $currentOperation"
    $lines += "Overall gate status    : $overall"
    $lines += "Latest GREEN full gate : $latestGreenLabel"
    $lines += "Session log            : $Global:SessionLog"
    $lines += "Debug bundle           : $FinalZipPath"
    $lines += "Runtime authority      : native C++ only"
    $lines += "Legacy C#              : inert reference archive; never shipping fallback"
    $lines += ""

    $lines += "FULL GATE STEPS"
    $lines += "------------------------------------------------------------------------"
    if ($Global:StepResults.Count -eq 0) {
        $lines += "No in-memory gate step results are available in this invocation."
    } else {
        foreach ($result in $Global:StepResults) {
            $message = if ($result.Message) { " - $($result.Message)" } else { "" }
            $lines += ("[{0}] {1}{2}" -f $result.Status, $result.Name, $message)
        }
    }
    $lines += ""

    $lines += "BUILD / AUDIT QUICK STATUS"
    $lines += "------------------------------------------------------------------------"
    $lines += ("Project scope/status           : {0}" -f (Get-DebugStepStatus "Project scope/status"))
    $lines += ("Root cleanliness audit         : {0}" -f (Get-DebugStepStatus "Root cleanliness audit"))
    $lines += ("C++ conversion status          : {0}" -f (Get-DebugStepStatus "C++ conversion status"))
    $lines += ("Native runtime guard           : {0}" -f (Get-DebugStepStatus "Native runtime regression guard"))
    $lines += ("Render configure/build/test    : {0}" -f (Get-DebugStepStatus "Render C++ configure/build/test"))
    $lines += ("Native game executable         : {0}" -f (Get-DebugStepStatus "Native C++ game executable present"))
    $lines += ("Native runtime smoke           : {0}" -f (Get-DebugStepStatus "Native C++ runtime smoke"))
    $lines += ("Native executable path         : {0}" -f $(if ($gameExe) { $gameExe } else { "NOT FOUND" }))
    $lines += ""

    $lines += "NATIVE TEST ASSERTIONS"
    $lines += "------------------------------------------------------------------------"
    $lines += "Evidence scope: $testEvidenceScope"
    $lastTestPath = Get-PreferredDebugLastTestLog
    if ($lastTestPath) {
        $lastTestInfo = Get-Item -LiteralPath $lastTestPath
        $lines += "LastTest.log: $lastTestPath"
        $lines += "LastTest timestamp: $($lastTestInfo.LastWriteTime.ToString('yyyy-MM-dd HH:mm:ss'))"
    }
    $combinedPassed = 0
    $combinedFailed = 0
    if ($testRecords.Count -eq 0) {
        $lines += "No assertion summary markers were found in LastTest.log."
    } else {
        foreach ($record in $testRecords) {
            $lines += ("{0,-36} {1,6} passed / {2} failed" -f $record.Name, $record.Passed, $record.Failed)
            $combinedPassed += $record.Passed
            $combinedFailed += $record.Failed
        }
        $lines += "------------------------------------------------------------------------"
        $lines += ("Combined assertions                  {0,6} passed / {1} failed" -f $combinedPassed, $combinedFailed)
    }
    $lines += ""

    $lines += "CTEST"
    $lines += "------------------------------------------------------------------------"
    $lines += "Evidence scope: $testEvidenceScope"
    if ($null -eq $ctest.Total) {
        $lines += "CTest summary unavailable."
    } else {
        $lines += ("CTest targets                       {0} passed / {1} failed / {2} total" -f $ctest.Passed, $ctest.Failed, $ctest.Total)
    }
    $lines += ""

    $lines += "LATEST PATCH HANDOFF"
    $lines += "------------------------------------------------------------------------"
    $latestApply = Join-Path $Global:SubspaceRoot "updates\LATEST_UPDATE_APPLY.txt"
    if (Test-Path -LiteralPath $latestApply) {
        foreach ($line in @(Get-Content -LiteralPath $latestApply -ErrorAction SilentlyContinue | Select-Object -First 40)) {
            $lines += [string]$line
        }
    } else {
        $lines += "No updates\\LATEST_UPDATE_APPLY.txt was present."
    }
    $lines += ""

    $lines += "RUNTIME / RENDERER EVIDENCE"
    $lines += "------------------------------------------------------------------------"
    $evidence = @()
    if (Test-Path -LiteralPath $Global:SessionLog) {
        $evidence = @(Get-Content -LiteralPath $Global:SessionLog -ErrorAction SilentlyContinue | Where-Object {
            [string]$_ -match 'native modular ship visual library|GLSL material shader|runtime smoke completed|subspace_game exit code'
        } | Select-Object -Last 20)
    }
    if ($evidence.Count -eq 0) {
        $lines += "No renderer/runtime evidence lines were detected in the session log."
    } else {
        foreach ($line in $evidence) { $lines += [string]$line }
    }
    $lines += ""

    $lines += "FAILURE / ERROR EXCERPTS"
    $lines += "------------------------------------------------------------------------"
    $diagnostics = @()
    if (Test-Path -LiteralPath $Global:SessionLog) {
        $diagnostics = @(Get-Content -LiteralPath $Global:SessionLog -ErrorAction SilentlyContinue | Where-Object {
            $text = [string]$_
            ($text -match '\[FAIL\]|\[ERROR\]|fatal error|error C\d+|FAILED:|Assertion failed|tests failed') -and
            ($text -notmatch '0 tests failed')
        } | Select-Object -Last 80)
    }
    if ($diagnostics.Count -eq 0) {
        $lines += "None detected in the current session log."
    } else {
        foreach ($line in $diagnostics) { $lines += [string]$line }
    }
    $lines += ""

    $lines += "HANDOFF"
    $lines += "------------------------------------------------------------------------"
    $lines += "This summary is intentionally plain UTF-8 text and is duplicated in:"
    $lines += "  1. the archive root as DEBUG_SUMMARY.txt"
    $lines += "  2. artifacts\\debug as Subspace_DebugSummary_<timestamp>.txt"
    $lines += "  3. LATEST_DEBUG_BUNDLE.txt as an embedded certification summary"
    $lines += "Upload LATEST_DEBUG_BUNDLE.txt with the ZIP when possible; the pointer now carries enough evidence for immediate AI audit."

    Write-Utf8TextFile -Path $OutputPath -Lines $lines
}
function New-DebugBundleIndex {
    param(
        [string]$StagePath,
        [string]$OutputPath
    )

    [object[]]$lines = @()
    $lines += "CODENAME SUBSPACE DEBUG BUNDLE INDEX"
    $lines += "========================================================================"
    $lines += "Timestamp: $Global:Timestamp"
    $lines += "Archive layout: files are rooted directly at ZIP root; no timestamp wrapper directory."
    $lines += ""
    $lines += "RELATIVE PATH | BYTES | SHA-256"
    $lines += "------------------------------------------------------------------------"

    foreach ($file in @(Get-ChildItem -LiteralPath $StagePath -Recurse -File -ErrorAction SilentlyContinue | Sort-Object FullName)) {
        if ($file.FullName -eq $OutputPath) { continue }
        $relative = $file.FullName.Substring($StagePath.Length).TrimStart([char[]]'\/')
        $hash = (Get-FileHash -Algorithm SHA256 -LiteralPath $file.FullName).Hash.ToLowerInvariant()
        $lines += ("{0} | {1} | {2}" -f $relative, $file.Length, $hash)
    }
    $lines += ""
    $lines += "This index intentionally excludes its own hash to avoid a self-referential manifest."
    Write-Utf8TextFile -Path $OutputPath -Lines $lines
}
function Test-DebugBundleArchiveContract {
    param([string]$ArchivePath)

    if (-not (Test-Path -LiteralPath $ArchivePath)) {
        throw "Debug bundle archive is missing: $ArchivePath"
    }

    Add-Type -AssemblyName System.IO.Compression.FileSystem -ErrorAction SilentlyContinue
    $archive = [System.IO.Compression.ZipFile]::OpenRead($ArchivePath)
    try {
        $names = @($archive.Entries | ForEach-Object { $_.FullName })
        foreach ($required in @("DEBUG_SUMMARY.txt", "DEBUG_BUNDLE_INDEX.txt")) {
            if ($names -notcontains $required) {
                throw "Debug bundle contract missing archive-root file: $required"
            }
        }
        $wrapper = @($names | Where-Object { $_ -match '^(?i)debug-bundle-\d{8}-\d{6}[\\/]' })
        if ($wrapper.Count -gt 0) {
            throw "Debug bundle still contains a timestamp wrapper directory instead of archive-root evidence."
        }
        $sessionEvidence = @($names | Where-Object { $_ -match '^(?i)logs[\\/]sessions[\\/]subspace-tools-.*\.log$' })
        if ($sessionEvidence.Count -eq 0) {
            throw "Debug bundle contract missing session log evidence under logs/sessions/."
        }
    }
    finally {
        $archive.Dispose()
    }
}


function Copy-DebugPathRobust {
    param(
        [string]$Source,
        [string]$Destination
    )

    if (-not (Test-Path -LiteralPath $Source)) { return }

    $sourceItem = Get-Item -LiteralPath $Source -Force -ErrorAction Stop
    if (-not $sourceItem.PSIsContainer) {
        $destParent = Split-Path -Parent $Destination
        if ($destParent) { [System.IO.Directory]::CreateDirectory($destParent) | Out-Null }
        [System.IO.File]::Copy($sourceItem.FullName, $Destination, $true)
        return
    }

    [System.IO.Directory]::CreateDirectory($Destination) | Out-Null

    foreach ($dir in [System.IO.Directory]::EnumerateDirectories($sourceItem.FullName, "*", [System.IO.SearchOption]::AllDirectories)) {
        $relative = $dir.Substring($sourceItem.FullName.Length).TrimStart([char[]]'\/')
        $destDir = if ($relative) { Join-Path $Destination $relative } else { $Destination }
        [System.IO.Directory]::CreateDirectory($destDir) | Out-Null
    }

    foreach ($file in [System.IO.Directory]::EnumerateFiles($sourceItem.FullName, "*", [System.IO.SearchOption]::AllDirectories)) {
        $relative = $file.Substring($sourceItem.FullName.Length).TrimStart([char[]]'\/')
        $destFile = Join-Path $Destination $relative
        $destParent = Split-Path -Parent $destFile
        if ($destParent) { [System.IO.Directory]::CreateDirectory($destParent) | Out-Null }
        try {
            [System.IO.File]::Copy($file, $destFile, $true)
        }
        catch {
            Write-Log ("Debug evidence copy skipped file: {0} ({1})" -f $file, $_.Exception.Message) "WARN"
        }
    }
}

function Invoke-DebugBundle {
    param([switch]$PreserveScreen)
    if (-not $PreserveScreen) { Write-Header }
    Write-Log "Packaging self-describing debug bundle."

    $stage = Join-Path $Global:TempRoot "debug-bundle-$Global:Timestamp"
    $zip = Join-Path $Global:DebugBundleRoot "Subspace_DebugBundle_$Global:Timestamp.zip"
    $summarySibling = Join-Path $Global:DebugBundleRoot "Subspace_DebugSummary_$Global:Timestamp.txt"
    $latestPointer = Join-Path $Global:DebugBundleRoot "LATEST_DEBUG_BUNDLE.txt"

    if (Test-Path -LiteralPath $stage) { Remove-Item -LiteralPath $stage -Recurse -Force }
    New-Item -ItemType Directory -Force -Path $stage | Out-Null

    $copyTargets = @(
        @{ Source = Join-Path $Global:EngineRoot "CMakeLists.txt"; Name = "engine-CMakeLists.txt" },
        @{ Source = Join-Path $Global:EngineRoot "tests"; Name = "engine-tests" },
        @{ Source = Join-Path $Global:SubspaceRoot "project.control.json"; Name = "project.control.json" },
        @{ Source = Join-Path $Global:SubspaceRoot "tools\control"; Name = "tools-control" },
        @{ Source = Join-Path $Global:EngineRoot "build\CMakeCache.txt"; Name = "engine-build-CMakeCache.txt" },
        @{ Source = Join-Path $Global:EngineRoot "build-headless\CMakeCache.txt"; Name = "engine-build-headless-CMakeCache.txt" },
        @{ Source = Join-Path $Global:EngineRoot "build\Testing\Temporary\LastTest.log"; Name = "engine-build-LastTest.log" },
        @{ Source = Join-Path $Global:EngineRoot "build-headless\Testing\Temporary\LastTest.log"; Name = "engine-build-headless-LastTest.log" },
        @{ Source = Join-Path $Global:SubspaceRoot "docs\build"; Name = "docs-build" },
        @{ Source = Join-Path $Global:SubspaceRoot "scripts"; Name = "scripts" },
        @{ Source = Join-Path $Global:SubspaceRoot "SubspaceTools.ps1"; Name = "SubspaceTools.ps1" },
        @{ Source = Join-Path $Global:SubspaceRoot "SubspaceTools.cmd"; Name = "SubspaceTools.cmd" },
        @{ Source = Join-Path $Global:SubspaceRoot "updates\logs"; Name = "updates-logs" },
        @{ Source = Join-Path $Global:SubspaceRoot "updates\LATEST_UPDATE_APPLY.txt"; Name = "LATEST_UPDATE_APPLY.txt" },
        @{ Source = $Global:LogsRoot; Name = "logs" }
    )

    foreach ($target in $copyTargets) {
        $source = $target.Source
        if (Test-Path -LiteralPath $source) {
            $dest = Join-Path $stage $target.Name
            Write-Log "Adding debug item: $source"
            Copy-DebugPathRobust -Source $source -Destination $dest
            Write-Log "Debug item staged: $($target.Name)" "PASS"
        }
    }

    $summary = Join-Path $stage "DEBUG_SUMMARY.txt"
    Write-Log "Building debug certification summary."
    New-DebugCertificationSummary -OutputPath $summary -FinalZipPath $zip
    Write-Log "Debug certification summary staged." "PASS"

    Write-Log "Copying sibling debug summary."
    [System.IO.File]::Copy($summary, $summarySibling, $true)
    Write-Log "Sibling debug summary copied." "PASS"

    $index = Join-Path $stage "DEBUG_BUNDLE_INDEX.txt"
    Write-Log "Building debug bundle index."
    New-DebugBundleIndex -StagePath $stage -OutputPath $index
    Write-Log "Debug bundle index staged." "PASS"

    # Debug bundles are intentionally flat at archive root so DEBUG_SUMMARY.txt and
    # DEBUG_BUNDLE_INDEX.txt are immediately discoverable by file ingestion systems.
    $createdZip = Compress-StagedFolder -StagePath $stage -DestinationZip $zip -CleanupOnSuccess
    Test-DebugBundleArchiveContract -ArchivePath $createdZip
    Write-Log "Debug bundle self-describing archive contract verified." "PASS"
    Write-LatestArtifactPointer -PointerFile $latestPointer -ArtifactPath $createdZip -SummaryPath $summarySibling
    Write-Log "Debug summary written: $summarySibling" "PASS"
    Write-Log "Debug bundle written: $createdZip" "PASS"
    $Global:LastDebugSummaryPath = $summarySibling
    $Global:LastDebugBundlePath = $createdZip
    Write-Host ""
    Write-Host "DEBUG SUMMARY READY: $summarySibling"
    Write-Host "DEBUG BUNDLE READY:  $createdZip"
}

function Copy-RollupDirectory {
    param(
        [string]$RelativePath,
        [string]$StageRepo
    )

    $source = Join-Path $Global:SubspaceRoot $RelativePath
    if (-not (Test-Path -LiteralPath $source -PathType Container)) {
        return $false
    }

    $destination = Join-Path $StageRepo $RelativePath
    Invoke-ProjectTreeStage -Source $source -Destination $destination `
        -Label ("asset/content stage " + $RelativePath) | Out-Null
    return $true
}

function Write-RollupInventory {
    param(
        [string]$StageRepo,
        [string]$ManifestName,
        [string]$RollupKind,
        [string[]]$Notes
    )

    $files = @(Get-ChildItem -LiteralPath $StageRepo -Recurse -File -Force -ErrorAction SilentlyContinue)
    $totalBytes = [int64](($files | Measure-Object -Property Length -Sum).Sum)
    if ($null -eq $totalBytes) { $totalBytes = 0 }

    [object[]]$lines = @()
    $lines += "CODENAME SUBSPACE $RollupKind ROLLUP"
    $lines += "========================================================================"
    $lines += "Timestamp: $Global:Timestamp"
    $lines += "Repository root: $Global:SubspaceRoot"
    $lines += "Files staged: $($files.Count)"
    $lines += ("Expanded bytes: {0}" -f $totalBytes)
    $lines += ("Expanded MiB: {0:N1}" -f ($totalBytes / 1MB))
    $lines += ""
    foreach ($note in $Notes) { $lines += $note }

    Write-Utf8TextFile -Path (Join-Path $StageRepo $ManifestName) -Lines $lines
}

function Invoke-SourceRollup {
    Write-Header
    Write-Log "Packaging CODE/SOURCE rollup (runtime/third-party asset payloads excluded)."

    $stageRoot = Join-Path $Global:TempRoot "source-rollup-$Global:Timestamp"
    $stageRepo = Join-Path $stageRoot "Codename-Subspace-main"
    $zip = Join-Path $Global:SourceBundleRoot "Codename_Subspace_Source_$Global:Timestamp.zip"

    if (Test-Path -LiteralPath $stageRoot) { Remove-Item -LiteralPath $stageRoot -Recurse -Force }
    New-Item -ItemType Directory -Force -Path $stageRoot | Out-Null

    $sink = {
        param($line)
        $clean = Remove-SubspaceAnsiSequences ([string]$line)
        Write-ChildOutputLine $clean
        [System.IO.File]::AppendAllText($Global:SessionLog,$clean + [Environment]::NewLine,$Global:Utf8NoBom)
    }
    Write-Log ("governed source stage: {0} -> {1}" -f $Global:SubspaceRoot,$stageRepo) "INFO"
    $stageResult = Invoke-ProjectOpsGovernedTreeStage -Root $Global:SubspaceRoot `
        -Destination $stageRepo -OnOutput $sink
    Write-Log ("governed source stage backend={0} exit={1}; authority={2}; files={3}" -f `
        $stageResult.Backend,$stageResult.ExitCode,$stageResult.SourceAuthorityId,$stageResult.SourcePathCount) "PASS"


    Write-RollupInventory -StageRepo $stageRepo -ManifestName "SOURCE_ROLLUP_MANIFEST.txt" -RollupKind "SOURCE" -Notes @(
        "Purpose: code/configuration/text metadata handoff.",
        "Asset payloads are intentionally excluded.",
        "Use Advanced -> Package asset/content rollup when binary runtime content is required.",
        "The source and asset rollups share the same Codename-Subspace-main wrapper so they can be overlaid when needed.",
        "Generated update history is excluded; the current source tree is authoritative."
    )

    $createdZip = Compress-StagedFolder -StagePath $stageRepo -DestinationZip $zip -IncludeBaseDirectory -CleanupOnSuccess
    if (Test-Path -LiteralPath $stageRoot) {
        Remove-Item -LiteralPath $stageRoot -Recurse -Force -ErrorAction SilentlyContinue
    }

    $hash = (Get-FileHash -Algorithm SHA256 -LiteralPath $createdZip).Hash.ToLowerInvariant()
    Write-LatestArtifactPointer -PointerFile (Join-Path $Global:SourceBundleRoot "LATEST_SOURCE_ROLLUP.txt") -ArtifactPath $createdZip
    Add-Content -LiteralPath (Join-Path $Global:SourceBundleRoot "LATEST_SOURCE_ROLLUP.txt") -Value ("SHA-256: " + $hash)

    Write-Log "Source-only rollup written: $createdZip" "PASS"
    Write-Host ""
    Write-Host "SOURCE ROLLUP READY: $createdZip"
    Write-Host "SHA-256: $hash"
    Write-Host "Runtime/third-party assets were NOT included."
}

function Invoke-AssetRollup {
    Write-Header
    Write-Log "Packaging separate runtime/third-party ASSET rollup."

    $stageRoot = Join-Path $Global:TempRoot "asset-rollup-$Global:Timestamp"
    $stageRepo = Join-Path $stageRoot "Codename-Subspace-main"
    $zip = Join-Path $Global:AssetBundleRoot "Codename_Subspace_Assets_$Global:Timestamp.zip"

    if (Test-Path -LiteralPath $stageRoot) { Remove-Item -LiteralPath $stageRoot -Recurse -Force }
    New-Item -ItemType Directory -Force -Path $stageRepo | Out-Null

    $assetRoots = @(
        "assets",
        "content\third_party",
        "content\derived",
        "content\imports",
        "content\imported",
        "content\downloads",
        "content\generated"
    )

    $copied = @()
    foreach ($relative in $assetRoots) {
        if (Copy-RollupDirectory -RelativePath $relative -StageRepo $stageRepo) {
            $copied += $relative
        }
    }

    foreach ($rootFile in @("CREDITS.md", "LICENSE")) {
        $source = Join-Path $Global:SubspaceRoot $rootFile
        if (Test-Path -LiteralPath $source) {
            [System.IO.File]::Copy($source, (Join-Path $stageRepo $rootFile), $true)
        }
    }

    if ($copied.Count -eq 0) {
        throw "No asset payload roots were found. Nothing was packaged."
    }

    Write-RollupInventory -StageRepo $stageRepo -ManifestName "ASSET_ROLLUP_MANIFEST.txt" -RollupKind "ASSET" -Notes @(
        "Purpose: binary/runtime/third-party content handoff.",
        ("Included payload roots: " + ($copied -join ", ")),
        "Code and build outputs are intentionally not duplicated here.",
        "Licensing/provenance files inside the asset trees are preserved.",
        "CREDITS.md and LICENSE are copied when present.",
        "The source and asset rollups share the same Codename-Subspace-main wrapper so they can be overlaid when needed."
    )

    $createdZip = Compress-StagedFolder -StagePath $stageRepo -DestinationZip $zip -IncludeBaseDirectory -CleanupOnSuccess
    if (Test-Path -LiteralPath $stageRoot) {
        Remove-Item -LiteralPath $stageRoot -Recurse -Force -ErrorAction SilentlyContinue
    }

    $hash = (Get-FileHash -Algorithm SHA256 -LiteralPath $createdZip).Hash.ToLowerInvariant()
    Write-LatestArtifactPointer -PointerFile (Join-Path $Global:AssetBundleRoot "LATEST_ASSET_ROLLUP.txt") -ArtifactPath $createdZip
    Add-Content -LiteralPath (Join-Path $Global:AssetBundleRoot "LATEST_ASSET_ROLLUP.txt") -Value ("SHA-256: " + $hash)

    Write-Log "Asset rollup written: $createdZip" "PASS"
    Write-Host ""
    Write-Host "ASSET ROLLUP READY: $createdZip"
    Write-Host "SHA-256: $hash"
}


function Invoke-RootAudit {
    Write-Header
    Invoke-ProjectScript -RelativePath "scripts\subspace_root_cleanliness_audit.ps1" -Arguments @("-Root", $Global:SubspaceRoot) -ContinueOnError | Out-Null
}

function Invoke-NormalizePlan {
    Write-Header
    Invoke-ProjectScript -RelativePath "scripts\subspace_content_normalize_plan.ps1" -Arguments @("-Root", $Global:SubspaceRoot) -ContinueOnError | Out-Null
}

function Invoke-NormalizationStatus {
    Write-Header
    Invoke-ProjectScript -RelativePath "scripts\subspace_normalization_status.ps1" -Arguments @("-Root", $Global:SubspaceRoot) -ContinueOnError | Out-Null
}



function Invoke-HomeDirectionStatus {
    Write-Header
    Invoke-ProjectScript -RelativePath "scripts\subspace_home_direction_status.ps1" -Arguments @("-Root", $Global:SubspaceRoot) -ContinueOnError | Out-Null
}

function Invoke-ProjectStatus {
    Write-Header
    Invoke-ProjectScript -RelativePath "scripts\subspace_project_scope_status.ps1" -Arguments @("-Root", $Global:SubspaceRoot) -ContinueOnError | Out-Null
}

function Invoke-ConversionStatus {
    Write-Header
    Invoke-ProjectScript -RelativePath "scripts\subspace_cpp_conversion_status.ps1" -Arguments @("-Root", $Global:SubspaceRoot) -ContinueOnError | Out-Null
}

function Invoke-NativeRuntimeGuard {
    Write-Header
    Invoke-ProjectScript -RelativePath "scripts\subspace_native_runtime_guard.ps1" -Arguments @("-Root", $Global:SubspaceRoot) -ContinueOnError | Out-Null
}

function Test-ShipyardContentReady {
    $ready = Join-Path $Global:SubspaceRoot "content\derived\greyoxide_shipyard_v07\SHIPYARD_READY.txt"
    $certified = Join-Path $Global:SubspaceRoot "content\derived\greyoxide_shipyard_v07\certified\SHIPYARD_CERTIFIED_R5.txt"
    $catalog = Join-Path $Global:SubspaceRoot "content\derived\greyoxide_shipyard_v07\certified\certified_module_catalog.csv"
    if (-not (Test-Path -LiteralPath $ready) -or -not (Test-Path -LiteralPath $certified) -or -not (Test-Path -LiteralPath $catalog)) { return $false }
    $markerText = Get-Content -LiteralPath $certified -Raw -ErrorAction SilentlyContinue
    if ($markerText -notmatch 'policy=PRESERVE_AUTHORED_OBJECTS') { return $false }
    $rows = @(Get-Content -LiteralPath $catalog -ErrorAction SilentlyContinue | Where-Object { -not [string]::IsNullOrWhiteSpace($_) })
    if ($rows.Count -le 1) { return $false }
    return $rows[0] -match 'preserved_authored_object'
}

function Invoke-EnsureApprovedContentDependencies {
    Write-Log "Verifying pinned approved kitbash source manifest and governed local content cache." "INFO"
    Invoke-ProjectScript -RelativePath "scripts\subspace_ensure_kitbash_sources.ps1" -Arguments @(
        "-Root", $Global:SubspaceRoot,
        "-Mode", "AUTO"
    )
    if (-not (Test-ShipyardContentReady)) {
        throw "Approved kitbash source verification completed without producing a valid R5 Shipyard canonical marker/catalog state."
    }
    Write-Log "Approved kitbash sources are certified; Shipyard R5 corpus is ready." "PASS"
}


function Invoke-SupplyChainGate {
    param(
        [ValidateSet("AUTO", "VERIFY_ONLY", "CACHE_ONLY", "OFFLINE")]
        [string]$Mode = "AUTO"
    )
    Write-Header
    Write-Log "Running project-wide pinned source / dependency / asset supply-chain gate ($Mode)." "INFO"
    Invoke-ProjectScript -RelativePath "scripts\subspace_supply_chain_gate.ps1" -Arguments @(
        "-Root", $Global:SubspaceRoot,
        "-Mode", $Mode
    )
    Write-Log "Project-wide source/dependency gate certified." "PASS"
}

function Invoke-SourceSafetySnapshot {
    param(
        [string]$Label = "MANUAL",
        [switch]$OnlyIfUpdates
    )
    $rootUpdates = @(Get-RootDropUpdateZips)
    $inboxUpdates = @(Get-InboxUpdateZips)
    if ($OnlyIfUpdates -and ($rootUpdates.Count + $inboxUpdates.Count) -eq 0) {
        Write-Log "No queued update handoff; pre-patch source safety snapshot not required." "PASS"
        return
    }

    Write-Log "Creating source safety snapshot [$Label] before project state changes." "INFO"
    Invoke-SourceRollup
    $sourceZip = Join-Path $Global:SourceBundleRoot "Codename_Subspace_Source_$Global:Timestamp.zip"
    if (-not (Test-Path -LiteralPath $sourceZip)) { throw "Source rollup did not produce expected snapshot input: $sourceZip" }
    New-Item -ItemType Directory -Force -Path $Global:SourceSnapshotRoot | Out-Null
    $safeLabel = ($Label -replace '[^A-Za-z0-9_-]', '_').ToUpperInvariant()
    $snapshot = Join-Path $Global:SourceSnapshotRoot "Codename_Subspace_${safeLabel}_SourceSnapshot_$Global:Timestamp.zip"
    Copy-Item -LiteralPath $sourceZip -Destination $snapshot -Force
    $hash = (Get-FileHash -Algorithm SHA256 -LiteralPath $snapshot).Hash.ToLowerInvariant()
    @(
        "Codename Subspace source safety snapshot",
        "Label: $safeLabel",
        "Timestamp: $Global:Timestamp",
        "Artifact: $snapshot",
        "SHA-256: $hash",
        "Assets/dependency payloads excluded by design; governed source manifests and gate evidence remain in source."
    ) | Set-Content -LiteralPath ($snapshot + ".txt") -Encoding UTF8

    $keep = 12
    $old = @(Get-ChildItem -LiteralPath $Global:SourceSnapshotRoot -Filter '*.zip' -File -ErrorAction SilentlyContinue | Sort-Object LastWriteTime -Descending)
    if ($old.Count -gt $keep) {
        foreach ($item in $old[$keep..($old.Count-1)]) {
            Remove-Item -LiteralPath $item.FullName -Force -ErrorAction SilentlyContinue
            Remove-Item -LiteralPath ($item.FullName + ".txt") -Force -ErrorAction SilentlyContinue
        }
    }
    Write-Log "Source safety snapshot stored: $snapshot" "PASS"
}

function Find-SubspaceBlender {
    if ($env:BLENDER_EXE -and (Test-Path -LiteralPath $env:BLENDER_EXE)) { return $env:BLENDER_EXE }
    $cmd = Get-Command blender -ErrorAction SilentlyContinue
    if ($cmd) { return $cmd.Source }
    $base = 'C:\Program Files\Blender Foundation'
    if (Test-Path -LiteralPath $base) {
        $candidate = Get-ChildItem -LiteralPath $base -Directory -ErrorAction SilentlyContinue |
            Sort-Object Name -Descending |
            ForEach-Object { Join-Path $_.FullName 'blender.exe' } |
            Where-Object { Test-Path -LiteralPath $_ } |
            Select-Object -First 1
        if ($candidate) { return $candidate }
    }
    return $null
}

function Invoke-AuditShipyardReferences {
    Write-Header
    Invoke-EnsureApprovedContentDependencies
    $blender = Find-SubspaceBlender
    if (-not $blender) { throw 'Blender was not found. Install Blender or set BLENDER_EXE.' }
    $script = Join-Path $Global:SubspaceRoot 'tools\blender\shipyard_reference_usage_audit.py'
    if (-not (Test-Path -LiteralPath $script)) { throw "Reference audit script missing: $script" }
    $roots = @(
        (Join-Path $Global:SubspaceRoot 'content\third_party\shipyard_strikes_back_v1'),
        (Join-Path $Global:SubspaceRoot 'content\derived\shipyard_strikes_back_v1'),
        $Global:SubspaceRoot
    )
    $glb = $null
    foreach ($candidateRoot in $roots) {
        if (-not (Test-Path -LiteralPath $candidateRoot)) { continue }
        $glb = Get-ChildItem -LiteralPath $candidateRoot -Recurse -File -Filter 'shipyard_strikes_back.glb' -ErrorAction SilentlyContinue | Select-Object -First 1
        if ($glb) { break }
    }
    if (-not $glb) { throw 'shipyard_strikes_back.glb was not found in the governed project content trees.' }
    Invoke-LoggedCommand -Label 'Blender authored Shipyard reference audit' -FilePath $blender -Arguments @(
        '--background','--python',$script,'--','--project-root',$Global:SubspaceRoot,'--input',$glb.FullName
    ) -WorkingDirectory $Global:SubspaceRoot | Out-Null
    $summary = Join-Path $Global:SubspaceRoot 'content\derived\shipyard_strikes_back_v1\reference_assemblies\REFERENCE_AUDIT_SUMMARY.json'
    if (-not (Test-Path -LiteralPath $summary)) { throw 'Blender completed without producing the reference audit summary.' }
    Write-Log "Authored Shipyard reference evidence ready: $summary" 'PASS'
}


function Invoke-PassContinuityAudit {
    Invoke-ProjectScript -RelativePath "scripts\subspace_pass_continuity_audit.ps1" -Arguments @("-Root", $Global:SubspaceRoot) | Out-Null
}

function Invoke-RecoverPass655674 {
    Write-Header
    $script = Join-Path $Global:SubspaceRoot "scripts\subspace_recover_pass655_674.ps1"
    if (-not (Test-Path -LiteralPath $script)) { throw "Missing recovery script: $script" }
    & $script -Root $Global:SubspaceRoot
    if ($LASTEXITCODE -ne 0) { throw "Pass655-674 recovery failed." }
}

function Invoke-FullGate {
    param([switch]$CleanRoom)

    $Global:GateExecutionMode = if ($CleanRoom) { "clean-room" } else { "incremental-authoritative" }
    Write-Log ("Full Gate execution mode: {0}" -f $Global:GateExecutionMode) "INFO"
    Write-Header
    Clear-Logs -PreserveCurrent
    $Global:StepResults.Clear()

    # A patch can overwrite SubspaceTools.ps1 while this PowerShell process is
    # already executing the old in-memory function definitions. Hash the root
    # utility around update apply and re-enter Full Gate from the new on-disk
    # script whenever it changed. This makes dependency/bootstrap additions in
    # the same root-drop effective immediately instead of requiring option 13 or
    # a second manual invocation.
    $utilityPath = $PSCommandPath
    $utilityHashBefore = if (Test-Path -LiteralPath $utilityPath) { (Get-FileHash -Algorithm SHA256 -LiteralPath $utilityPath).Hash } else { '' }
    Invoke-UtilityStep -Name "Pre-patch source safety snapshot" -ScriptBlock { Invoke-SourceSafetySnapshot -Label "PREPATCH" -OnlyIfUpdates }
    Invoke-UtilityStep -Name "Auto-apply root/inbox patches before full gate" -ScriptBlock { Invoke-AutoApplyUpdateInbox -Reason "full gate" }
    $utilityHashAfter = if (Test-Path -LiteralPath $utilityPath) { (Get-FileHash -Algorithm SHA256 -LiteralPath $utilityPath).Hash } else { '' }
    if (-not $ReenteredAfterUpdate -and $utilityHashBefore -and $utilityHashAfter -and $utilityHashBefore -ne $utilityHashAfter) {
        Write-Log "Root control utility changed during patch apply; re-entering Full Gate from the updated script before dependency/bootstrap work." "INFO"
        $runner = Get-ToolPath "pwsh"
        if (-not $runner) { $runner = Get-ToolPath "powershell" }
        if (-not $runner) { throw "PowerShell runner not found for Full Gate self-update re-entry." }
        $reentryArgs = @("-NoProfile", "-ExecutionPolicy", "Bypass", "-File", $utilityPath, "-Action", "full-gate", "-Configuration", $Configuration, "-ReenteredAfterUpdate")
        if ($CleanRoom -or $Clean) { $reentryArgs += "-Clean" }
        if ($SkipTests) { $reentryArgs += "-SkipTests" }
        if ($NoAutoApplyUpdates) { $reentryArgs += "-NoAutoApplyUpdates" }
        $previousInteractiveParent = $env:SUBSPACE_INTERACTIVE_PARENT
        if ($Action -eq "menu") { $env:SUBSPACE_INTERACTIVE_PARENT = "1" }
        try {
            & $runner @reentryArgs
            $reentryExitCode = $LASTEXITCODE
        }
        finally {
            if ($null -eq $previousInteractiveParent) {
                Remove-Item Env:SUBSPACE_INTERACTIVE_PARENT -ErrorAction SilentlyContinue
            }
            else {
                $env:SUBSPACE_INTERACTIVE_PARENT = $previousInteractiveParent
            }
        }

        Reset-SubspaceConsolePalette
        $Global:LastActionExitCode = $reentryExitCode
        if ($Action -eq "menu") {
            Write-Log ("Updated Full Gate child process completed with exit code {0}; returning control to the interactive menu." -f $reentryExitCode) $(if ($reentryExitCode -eq 0) { "PASS" } else { "FAIL" })
            return
        }
        exit $reentryExitCode
    }

    $gateException = $null
    try {
    Invoke-UtilityStep -Name "Supply-chain source / asset / dependency gate" -ScriptBlock { Invoke-SupplyChainGate -Mode "AUTO" }
    Invoke-UtilityStep -Name "Retire stale conversion build artifacts" -ScriptBlock { Invoke-ProjectScript -RelativePath "scripts\subspace_legacy_artifact_cleanup.ps1" -Arguments @("-Root", $Global:SubspaceRoot) }
    Invoke-UtilityStep -Name "Project scope/status" -ScriptBlock { Invoke-ProjectScript -RelativePath "scripts\subspace_project_scope_status.ps1" -Arguments @("-Root", $Global:SubspaceRoot) } -ContinueOnError
    Invoke-UtilityStep -Name "Root cleanliness audit" -ScriptBlock { Invoke-ProjectScript -RelativePath "scripts\subspace_root_cleanliness_audit.ps1" -Arguments @("-Root", $Global:SubspaceRoot) } -ContinueOnError
    Invoke-UtilityStep -Name "C++ conversion status" -ScriptBlock { Invoke-ProjectScript -RelativePath "scripts\subspace_cpp_conversion_status.ps1" -Arguments @("-Root", $Global:SubspaceRoot) } -ContinueOnError
    Invoke-UtilityStep -Name "Native runtime regression guard" -ScriptBlock { Invoke-ProjectScript -RelativePath "scripts\subspace_native_runtime_guard.ps1" -Arguments @("-Root", $Global:SubspaceRoot) }
    Invoke-UtilityStep -Name "Pass/source continuity audit" -ScriptBlock { Invoke-PassContinuityAudit }
    if ($CleanRoom) {
        Write-Log "Clean-room Full Gate requested; rebuilding the native object graph from an empty build directory." "INFO"
    }
    else {
        Write-Log "Authoritative incremental Full Gate: preserving the CMake build tree and relying on CMake/MSBuild dependency invalidation." "INFO"
    }
    Invoke-UtilityStep -Name "Render C++ configure/build/test" -ScriptBlock { Invoke-CMakeBuild -CleanFirst:$CleanRoom }
    Invoke-UtilityStep -Name "Shipyard Pass501-506 live authoring gate" -ScriptBlock { Invoke-ProjectScript -RelativePath "scripts\subspace_pass501_506_live_gate.ps1" -Arguments @("-Root", $Global:SubspaceRoot) }
    if ($IsWindows -or $env:OS -eq "Windows_NT") {
        Invoke-UtilityStep -Name "Native C++ game executable present" -ScriptBlock {
            $exe = Find-SubspaceGameExecutable
            if (-not $exe) { throw "subspace_game.exe was not found after render build." }
            Write-Log "Native C++ game found: $exe" "PASS"
        }
        Invoke-UtilityStep -Name "Native C++ runtime smoke" -ScriptBlock {
            Invoke-RunSubspaceGame -GameArguments @("--runtime-smoke")
            Write-Log "Native C++ runtime smoke completed." "PASS"
        }
    }
    Invoke-UtilityStep -Name "ProjectOps governed-source authority" -ScriptBlock { Invoke-ProjectSourceAuthorityCheck }
    $preSnapshotFailures = @($Global:StepResults | Where-Object { $_.Status -eq "FAIL" }).Count
    if ($preSnapshotFailures -eq 0) {
        Invoke-UtilityStep -Name "Certified source safety snapshot" -ScriptBlock { Invoke-SourceSafetySnapshot -Label "CERTIFIED" }
    }
    }
    catch {
        $gateException = $_
        Write-Log ("Full Gate stopped after a failed stage: {0}" -f $_.Exception.Message) "FAIL"
        if (@($Global:StepResults | Where-Object { $_.Status -eq "FAIL" }).Count -eq 0) {
            Add-StepResult -Name "Full Gate orchestration" -Status "FAIL" -Message $_.Exception.Message
        }
    }

    $stageFailures = @($Global:StepResults | Where-Object { $_.Status -eq "FAIL" }).Count
    $gateResult = if ($stageFailures -eq 0) { "PASS" } else { "FAIL" }
    try {
        Invoke-GateRecord -Kind "FULL" -Result $gateResult
        Add-StepResult -Name "Quality-gate certification record" -Status "PASS" -Message ("Recorded {0} Full Quality Gate authority." -f $gateResult)
    }
    catch {
        Add-StepResult -Name "Quality-gate certification record" -Status "FAIL" -Message $_.Exception.Message
        Write-Log ("Quality-gate certification record failed: {0}" -f $_.Exception.Message) "FAIL"
    }

    $failures = Write-StepSummary
    try { Invoke-ArtifactIndex } catch { Write-Log ("Artifact index refresh failed: {0}" -f $_.Exception.Message) "WARN" }

    if ($failures -gt 0) {
        try { Invoke-DebugBundle -PreserveScreen } catch { Write-Log ("Debug bundle packaging failed during gate finalization: {0}" -f $_.Exception.Message) "ERROR" }
    }
    else {
        Write-Log "GREEN gate: full debug ZIP suppressed for speed; use Packaging -> Package debug bundle when successful diagnostics are explicitly needed." "PASS"
    }

    $Global:LastActionExitCode = if ($failures -eq 0) { 0 } else { 1 }
    Show-GateCertificationBanner -Kind "FULL" -Failures $failures
    if ($failures -gt 0) { Open-DebugHandoffFolder -BundlePath $Global:LastDebugBundlePath }
}

function Invoke-FastDevelopmentGate {
    $Global:GateExecutionMode = "fast-development"
    Write-Header
    $Global:StepResults.Clear()
    $gateException = $null

    try {
        Invoke-UtilityStep -Name "Pre-patch source safety snapshot" -ScriptBlock { Invoke-SourceSafetySnapshot -Label "FAST_PREPATCH" -OnlyIfUpdates }
        Invoke-UtilityStep -Name "Auto-apply root/inbox patches" -ScriptBlock { Invoke-AutoApplyUpdateInbox -Reason "fast development gate" }
        Invoke-UtilityStep -Name "Supply-chain verification (offline-safe)" -ScriptBlock { Invoke-SupplyChainGate -Mode "VERIFY_ONLY" }
        Invoke-UtilityStep -Name "Native runtime regression guard" -ScriptBlock { Invoke-NativeRuntimeGuard }
        Invoke-UtilityStep -Name "Pass/source continuity audit" -ScriptBlock { Invoke-PassContinuityAudit }
        Invoke-UtilityStep -Name "Incremental native configure/build/test" -ScriptBlock { Invoke-CMakeBuild }
    }
    catch {
        $gateException = $_
        Write-Log ("Fast Gate stopped after a failed stage: {0}" -f $_.Exception.Message) "FAIL"
        if (@($Global:StepResults | Where-Object { $_.Status -eq "FAIL" }).Count -eq 0) {
            Add-StepResult -Name "Fast Gate orchestration" -Status "FAIL" -Message $_.Exception.Message
        }
    }

    $stageFailures = @($Global:StepResults | Where-Object { $_.Status -eq "FAIL" }).Count
    $gateResult = if ($stageFailures -eq 0) { "PASS" } else { "FAIL" }
    try {
        Invoke-GateRecord -Kind "FAST" -Result $gateResult
        Add-StepResult -Name "Quality-gate certification record" -Status "PASS" -Message ("Recorded {0} Fast Quality Gate authority." -f $gateResult)
    }
    catch {
        Add-StepResult -Name "Quality-gate certification record" -Status "FAIL" -Message $_.Exception.Message
        Write-Log ("Quality-gate certification record failed: {0}" -f $_.Exception.Message) "FAIL"
    }

    $failures = Write-StepSummary
    try { Invoke-ArtifactIndex } catch { Write-Log ("Artifact index refresh failed: {0}" -f $_.Exception.Message) "WARN" }

    if ($failures -gt 0) {
        try { Invoke-DebugBundle -PreserveScreen } catch { Write-Log ("Debug bundle packaging failed during gate finalization: {0}" -f $_.Exception.Message) "ERROR" }
    }
    else {
        Write-Log "GREEN gate: full debug ZIP suppressed for speed; use Packaging -> Package debug bundle when successful diagnostics are explicitly needed." "PASS"
    }

    $Global:LastActionExitCode = if ($failures -eq 0) { 0 } else { 1 }
    Show-GateCertificationBanner -Kind "FAST" -Failures $failures
    if ($failures -gt 0) { Open-DebugHandoffFolder -BundlePath $Global:LastDebugBundlePath }
}

function Invoke-CppPortAudit {
    Write-Header
    Invoke-ProjectScript -RelativePath "scripts\subspace_generate_cpp_port_matrix.ps1" -Arguments @("-Root", $Global:SubspaceRoot) -ContinueOnError | Out-Null
}


function Invoke-VerifyProject {
    Write-Header
    Invoke-ProjectScript -RelativePath "scripts\verify_project.ps1" -Arguments @("-Root", $Global:SubspaceRoot) -ContinueOnError | Out-Null
}

function Invoke-Setup {
    Write-Header
    if (Test-Path -LiteralPath (Join-Path $Global:SubspaceRoot "scripts\setup.ps1")) {
        Invoke-ProjectScript -RelativePath "scripts\setup.ps1" -Arguments @() -ContinueOnError | Out-Null
    }
    else {
        Write-Log "No setup.ps1 found." "WARN"
    }
}

function Open-LogsFolder {
    Initialize-UtilityFolders
    $explorer = Get-ToolPath "explorer.exe"
    if ($explorer) { Start-Process explorer.exe $Global:LogsRoot }
    else { Write-Log "Logs folder: $Global:LogsRoot" }
}


function Invoke-InstallRootUtility {
    Write-Header
    Invoke-ProjectScript -RelativePath "scripts\install_subspace_root_utility.ps1" -Arguments @("-Root", $Global:SubspaceRoot) -ContinueOnError | Out-Null
}

function Invoke-PreviewUpdateInbox {
    Write-Header
    Invoke-ProjectScript -RelativePath "scripts\subspace_apply_update_inbox.ps1" -Arguments @("-Root", $Global:SubspaceRoot, "-DryRun") -ContinueOnError | Out-Null
}

function Invoke-ApplyUpdateInbox {
    Write-Header
    Invoke-ProjectScript -RelativePath "scripts\subspace_apply_update_inbox.ps1" -Arguments @("-Root", $Global:SubspaceRoot) -ContinueOnError | Out-Null
}



function Invoke-FetchShipyard {
    Write-Header
    Invoke-ProjectScript -RelativePath "scripts\subspace_fetch_shipyard_v07.ps1" -Arguments @("-Root", $Global:SubspaceRoot) -ContinueOnError | Out-Null
}

function Invoke-PatchStatus {
    Write-Header
    Write-PatchHandoffStatus -Reason "manual status"
    $latest = Join-Path $Global:SubspaceRoot "updates\LATEST_UPDATE_APPLY.txt"
    if (Test-Path -LiteralPath $latest) {
        Write-Host ""
        Write-Host "Latest update apply result:" -ForegroundColor Cyan
        Get-Content -LiteralPath $latest -ErrorAction SilentlyContinue | Select-Object -First 40 | ForEach-Object { Write-Host "  $_" }
    }
}

function Get-ProjectOpsPythonInvocation {
    return Resolve-ProjectOpsPython
}

function Invoke-ProjectOpsPythonTool {
    param(
        [Parameter(Mandatory=$true)][string]$RelativePath,
        [string[]]$Arguments=@()
    )

    $result = Invoke-ProjectOpsPython -Root $Global:SubspaceRoot `
        -RelativePath $RelativePath -Arguments $Arguments
    if ($result.ExitCode -ne 0) {
        throw ("ProjectOps tool failed with exit code {0}: {1}" -f $result.ExitCode,$RelativePath)
    }
}

function Invoke-ProjectSourceAuthorityCheck {
    Invoke-ProjectOpsPythonTool -RelativePath 'tools\control\ProjectSourceAuthority.py' `
        -Arguments @('--root',$Global:SubspaceRoot,'snapshot')
}

function Invoke-ProjectOpsInspect {
    Invoke-ProjectOpsPythonTool -RelativePath 'tools\control\ProjectOpsCli.py' `
        -Arguments @('--root',$Global:SubspaceRoot,'inspect')
}

function Invoke-ProjectOpsCommands {
    Invoke-ProjectOpsPythonTool -RelativePath 'tools\control\ProjectOpsCli.py' `
        -Arguments @('--root',$Global:SubspaceRoot,'commands')
}

function Invoke-ProjectOpsCommandRegistryMenu {
    while ($true) {
        Write-Header
        Write-Host " PROJECTOPS REGISTERED COMMANDS" -ForegroundColor $Global:UiTitleForeground
        Write-Host "------------------------------------------------------------------------" -ForegroundColor $Global:UiMutedForeground
        $commands = @(Get-ProjectOpsCommandRegistry -Root $Global:SubspaceRoot)
        for ($index = 0; $index -lt $commands.Count; $index++) {
            $command = $commands[$index]
            $mutationMarker = if ([bool]$command.mutates) { "*" } else { " " }
            Write-Host (" {0,2}. {1} {2,-31} {3}" -f ($index + 1),$mutationMarker,$command.key,$command.label)
        }
        Write-Host ""
        Write-Host " * mutating command; project.control.json confirmation policy remains authoritative." -ForegroundColor $Global:UiMutedForeground
        Write-Host "  0. Back"
        $choice = Read-Host "Select command number"
        if ($choice -eq "0") { return }

        $selectedIndex = 0
        if (-not [int]::TryParse($choice,[ref]$selectedIndex)) {
            Write-Host "Unknown selection." -ForegroundColor $Global:UiWarnForeground
            Start-Sleep -Seconds 1
            continue
        }
        $selectedIndex--
        if ($selectedIndex -lt 0 -or $selectedIndex -ge $commands.Count) {
            Write-Host "Unknown selection." -ForegroundColor $Global:UiWarnForeground
            Start-Sleep -Seconds 1
            continue
        }

        $selected = $commands[$selectedIndex]
        Invoke-ProjectOpsPythonTool -RelativePath 'tools\control\ProjectOpsCli.py' `
            -Arguments @('--root',$Global:SubspaceRoot,'execute',[string]$selected.key)
        Pause-ForUser
    }
}

function Invoke-ProjectShellMenu {
    Invoke-ProjectOpsPythonTool -RelativePath 'tools\control\ProjectShell.py' `
        -Arguments @('--root',$Global:SubspaceRoot,'--action','menu')
}


function Invoke-ProjectOpsNormalizationAudit {
    $auditScript = Join-Path $Global:SubspaceRoot "tools\control\ProjectOpsNormalizationAudit.ps1"
    if (-not (Test-Path -LiteralPath $auditScript -PathType Leaf)) { throw "ProjectOps normalization audit missing: $auditScript" }
    $auditArgs = @("-Root",$Global:SubspaceRoot)
    & $auditScript @auditArgs
    if ($LASTEXITCODE -ne 0) { throw ("ProjectOps normalization audit failed with exit code {0}." -f $LASTEXITCODE) }
}

function Invoke-ProjectOpsRepair {
    Invoke-ProjectScript -RelativePath 'tools\control\ProjectOpsMaintenance.ps1' `
        -Arguments @('-Root',$Global:SubspaceRoot,'-Action','Repair') | Out-Null
}

function Show-AdvancedMenu {
    while ($true) {
        Write-Header
        Write-Host "Advanced / maintenance" -ForegroundColor $Global:UiTitleForeground
        Write-Host " 1. Project status / build scope"
        Write-Host " 2. Patch handoff status"
        Write-Host " 3. Preview root/inbox patch handoff"
        Write-Host " 4. Apply root/inbox patch handoff now"
        Write-Host " 5. Root audit"
        Write-Host " 6. Normalization status"
        Write-Host " 7. C++ conversion status"
        Write-Host " 8. C++ port audit"
        Write-Host " 9. Native runtime guard"
        Write-Host "10. Clean build outputs"
        Write-Host "11. Open logs folder"
        Write-Host "12. Package CODE/SOURCE rollup (assets excluded)"
        Write-Host "13. Package ASSET/CONTENT rollup (separate, potentially large)"
        Write-Host "14. Fetch/refresh Shipyard v0.7 CC0 modules"
        Write-Host "15. Audit authored Shipyard reference ships (Blender)"
        Write-Host "16. ProjectOps machine API inspect"
        Write-Host "17. ProjectOps registered command menu"
        Write-Host "18. Project shell / logged command runner"
        Write-Host "19. Governed source authority snapshot"
        Write-Host "20. Repair/normalize root tooling"
        Write-Host " 0. Back"
        Write-Host ""
        $choice = Read-Host "Select"
        try {
            switch ($choice) {
                "1" { Invoke-ProjectStatus; Pause-ForUser }
                "2" { Invoke-PatchStatus; Pause-ForUser }
                "3" { Invoke-PreviewUpdateInbox; Pause-ForUser }
                "4" { Invoke-ApplyUpdateInbox; Pause-ForUser }
                "5" { Invoke-RootAudit; Pause-ForUser }
                "6" { Invoke-NormalizationStatus; Pause-ForUser }
                "7" { Invoke-ConversionStatus; Pause-ForUser }
                "8" { Invoke-CppPortAudit; Pause-ForUser }
                "9" { Invoke-NativeRuntimeGuard; Pause-ForUser }
                "10" { Invoke-CleanBuildOutputs; Pause-ForUser }
                "11" { Open-LogsFolder; Pause-ForUser }
                "12" { Invoke-SourceRollup; Pause-ForUser }
                "13" { Invoke-AssetRollup; Pause-ForUser }
                "14" { Invoke-FetchShipyard; Pause-ForUser }
                "15" { Invoke-AuditShipyardReferences; Pause-ForUser }
                "16" { Invoke-ProjectOpsInspect; Pause-ForUser }
                "17" { Invoke-ProjectOpsCommandRegistryMenu }
                "18" { Invoke-ProjectShellMenu; Pause-ForUser }
                "19" { Invoke-ProjectSourceAuthorityCheck; Pause-ForUser }
                "20" { Invoke-ProjectOpsRepair; Pause-ForUser }
                "0" { return }
                default { Write-Host "Unknown option."; Start-Sleep -Seconds 1 }
            }
        }
        catch {
            Write-Log $_.Exception.Message "ERROR"
            try {
                Invoke-DebugBundle
                Open-DebugHandoffFolder -BundlePath $Global:LastDebugBundlePath
            } catch { Write-Log $_.Exception.Message "ERROR" }
            Pause-ForUser
        }
    }
}

function Show-BuildVerifyMenu {
    while ($true) {
        Write-Header
        Write-Host " BUILD & VERIFY" -ForegroundColor $Global:UiTitleForeground
        Write-Host "------------------------------------------------------------------------" -ForegroundColor $Global:UiMutedForeground
        Write-Host " 1. Full quality gate / certify GREEN (incremental authoritative)" -ForegroundColor $Global:UiGoodForeground
        Write-Host " 2. Clean-room Full quality gate (fresh rebuild / release)"
        Write-Host " 3. Fast development gate (does not promote GREEN baseline)"
        Write-Host " 4. Build native C++"
        Write-Host " 5. Run tests"
        Write-Host " 6. Project status"
        Write-Host " 7. Root cleanliness audit"
        Write-Host " 0. Back"
        Write-Host " Full GREEN is authoritative without deleting the build tree; use clean-room only for release/toolchain/cache validation." -ForegroundColor $Global:UiMutedForeground
        switch (Read-Host "Select") {
            "1" { Invoke-FullGate; Pause-ForUser }
            "2" { Invoke-FullGate -CleanRoom; Pause-ForUser }
            "3" { Invoke-FastDevelopmentGate; Pause-ForUser }
            "4" { Invoke-BuildRender; Pause-ForUser }
            "5" { Invoke-TestsOnly; Pause-ForUser }
            "6" { Invoke-ProjectStatus; Pause-ForUser }
            "7" { Invoke-RootAudit; Pause-ForUser }
            "0" { return }
        }
    }
}

function Show-RunPlayMenu {
    while ($true) {
        Write-Header
        Write-Host " RUN & PLAY" -ForegroundColor $Global:UiTitleForeground
        Write-Host "------------------------------------------------------------------------" -ForegroundColor $Global:UiMutedForeground
        Write-Host " 1. Launch SHIPYARD Dev Studio"
        Write-Host " 2. Run native game"
        Write-Host " 3. Runtime smoke"
        Write-Host " 0. Back"
        switch (Read-Host "Select") {
            "1" { Invoke-RunSubspaceGame -GameArguments @("--shipyard"); Pause-ForUser }
            "2" { Invoke-RunSubspaceGame -GameArguments @("--loop"); Pause-ForUser }
            "3" { Invoke-RunSubspaceGame -GameArguments @("--runtime-smoke"); Pause-ForUser }
            "0" { return }
        }
    }
}

function Show-AssetAuthorityMenu {
    while ($true) {
        Write-Header
        Write-Host " ASSET AUTHORITY & SUPPLY CHAIN" -ForegroundColor $Global:UiTitleForeground
        Write-Host "------------------------------------------------------------------------" -ForegroundColor $Global:UiMutedForeground
        Write-Host " 1. Full source/dependency/asset gate (AUTO)"
        Write-Host " 2. Verify installed/cache-only source state"
        Write-Host " 3. Dependency/source status"
        Write-Host " 4. Fetch/refresh approved Shipyard source"
        Write-Host " 5. Audit authored Shipyard reference ships"
        Write-Host " 0. Back"
        switch (Read-Host "Select") {
            "1" { Invoke-SupplyChainGate -Mode "AUTO"; Pause-ForUser }
            "2" { Invoke-SupplyChainGate -Mode "VERIFY_ONLY"; Pause-ForUser }
            "3" { Invoke-DependencyStatus; Pause-ForUser }
            "4" { Invoke-FetchShipyard; Pause-ForUser }
            "5" { Invoke-AuditShipyardReferences; Pause-ForUser }
            "0" { return }
        }
    }
}

function Show-PackagingBaselineMenu {
    while ($true) {
        Write-Header
        Write-Host " PACKAGING & BASELINES" -ForegroundColor $Global:UiTitleForeground
        Write-Host "------------------------------------------------------------------------" -ForegroundColor $Global:UiMutedForeground
        Write-Host " 1. Capture packaging baseline"
        Write-Host " 2. Package incremental handoff from baseline"
        Write-Host " 3. Package CODE/SOURCE rollup"
        Write-Host " 4. Package ASSET/CONTENT rollup"
        Write-Host " 5. Create source safety snapshot"
        Write-Host " 6. Package debug bundle"
        Write-Host " 7. Rebuild artifact index"
        Write-Host " 8. Create comprehensive project audit package"
        Write-Host " 0. Back"
        switch (Read-Host "Select") {
            "1" { Invoke-StandardControlAction -ControlAction "capture-baseline"; Pause-ForUser }
            "2" { Invoke-StandardControlAction -ControlAction "incremental-handoff"; Pause-ForUser }
            "3" { Invoke-SourceRollup; Pause-ForUser }
            "4" { Invoke-AssetRollup; Pause-ForUser }
            "5" { Invoke-SourceSafetySnapshot -Label "MANUAL"; Pause-ForUser }
            "6" { Invoke-DebugBundle; Pause-ForUser }
            "7" { Invoke-ArtifactIndex; Pause-ForUser }
            "8" { Invoke-StandardControlAction -ControlAction "audit-package"; Pause-ForUser }
            "0" { return }
        }
    }
}

function Show-MaintenanceDiagnosticsMenu {
    while ($true) {
        Write-Header
        Write-Host " PROJECT MAINTENANCE & DIAGNOSTICS" -ForegroundColor $Global:UiTitleForeground
        Write-Host "------------------------------------------------------------------------" -ForegroundColor $Global:UiMutedForeground
        Write-Host " 1. Project health dashboard"
        Write-Host " 2. Control Center self-test"
        Write-Host " 3. Environment / toolchain inventory"
        Write-Host " 4. Patch handoff status"
        Write-Host " 5. Preview update intake"
        Write-Host " 6. Apply update intake"
        Write-Host " 7. Undo latest committed patch (drift guarded)"
        Write-Host " 8. Native runtime guard"
        Write-Host " 9. Normalization status"
        Write-Host "10. C++ conversion status"
        Write-Host "11. Compiler/build warning summary"
        Write-Host "12. Root cleanliness audit"
        Write-Host "13. Clean build outputs"
        Write-Host "14. Pass/source continuity audit"
        Write-Host "15. Recover skipped Pass655-674 baseline"
        Write-Host " 0. Back"
        switch (Read-Host "Select") {
            "1" { Invoke-StandardControlAction -ControlAction "health"; Pause-ForUser }
            "2" { Invoke-StandardControlAction -ControlAction "self-test"; Pause-ForUser }
            "3" { Invoke-StandardControlAction -ControlAction "environment"; Pause-ForUser }
            "4" { Invoke-PatchStatus; Pause-ForUser }
            "5" { Invoke-PreviewUpdateInbox; Pause-ForUser }
            "6" { Invoke-ApplyUpdateInbox; Pause-ForUser }
            "7" { Invoke-UndoLastPatch; Pause-ForUser }
            "8" { Invoke-NativeRuntimeGuard; Pause-ForUser }
            "9" { Invoke-NormalizationStatus; Pause-ForUser }
            "10" { Invoke-ConversionStatus; Pause-ForUser }
            "11" { Invoke-StandardControlAction -ControlAction "warnings"; Pause-ForUser }
            "12" { Invoke-RootAudit; Pause-ForUser }
            "13" { Invoke-CleanBuildOutputs; Pause-ForUser }
            "14" { Invoke-PassContinuityAudit; Pause-ForUser }
            "15" { Invoke-RecoverPass655674; Pause-ForUser }
            "0" { return }
        }
    }
}

function Show-LogsHelpMenu {
    while ($true) {
        Write-Header
        Write-Host " LOGS & HELP" -ForegroundColor $Global:UiTitleForeground
        Write-Host "------------------------------------------------------------------------" -ForegroundColor $Global:UiMutedForeground
        Write-Host " 1. Open active session log folder"
        Write-Host " 2. Open latest debug bundle"
        Write-Host " 3. Open quality-gate history"
        Write-Host " 4. Compare latest Full/Fast gate records"
        Write-Host " 5. Open latest quality-gate record"
        Write-Host " 6. Compiler/build warning summary"
        Write-Host " 7. Rebuild artifact index"
        Write-Host " 8. Open latest failed update result"
        Write-Host " 9. Package fresh debug bundle"
        Write-Host " 0. Back"
        switch (Read-Host "Select") {
            "1" { Open-LogsFolder; Pause-ForUser }
            "2" { Open-LatestDebugArtifact; Pause-ForUser }
            "3" { Open-GateHistoryFolder; Pause-ForUser }
            "4" { Invoke-CompareQualityGates; Pause-ForUser }
            "5" { Open-LatestGateRecord; Pause-ForUser }
            "6" { Invoke-StandardControlAction -ControlAction "warnings"; Pause-ForUser }
            "7" { Invoke-ArtifactIndex; Pause-ForUser }
            "8" { Open-LatestFailedUpdate; Pause-ForUser }
            "9" { Invoke-DebugBundle; Pause-ForUser }
            "0" { return }
        }
    }
}


function Show-SourceControlMenu {
    while ($true) {
        Write-Header
        Write-Host " SOURCE CONTROL (GIT / GITHUB OPTIONAL)" -ForegroundColor $Global:UiTitleForeground
        Write-Host "------------------------------------------------------------------------" -ForegroundColor $Global:UiMutedForeground
        Write-Host " 1. Status / branch / remotes"
        Write-Host " 2. Prepare normalized GitHub authority (RECOMMENDED FIRST-TIME SETUP)" -ForegroundColor $Global:UiGoodForeground
        Write-Host " 3. Recent commit history"
        Write-Host " 4. Commit current certified GREEN source"
        Write-Host " 5. Push current branch"
        Write-Host " 6. Configure / inspect remote"
        Write-Host " 7. Audit GitHub repository authority (read-only)"
        Write-Host " 8. Repair/adopt Git working history without replacing source"
        Write-Host " 9. Publish last GREEN normalized main (archives old main first)"
        Write-Host "10. Advanced: initialize local Git only"
        Write-Host " 0. Back"
        Write-Host " First publication flow: 2 Prepare -> Full Quality Gate -> 9 Publish" -ForegroundColor $Global:UiMutedForeground
        switch (Read-Host "Select") {
            "1" { Invoke-StandardControlAction -ControlAction "git-status"; Pause-ForUser }
            "2" { Invoke-StandardControlAction -ControlAction "repo-authority-prepare"; Pause-ForUser }
            "3" { Invoke-StandardControlAction -ControlAction "git-history"; Pause-ForUser }
            "4" {
                $code = Invoke-StandardControlAction -ControlAction "git-commit-green" -ContinueOnError
                if ($code -ne 0) {
                    Write-Host "Commit was not performed. If this is first-time setup, use option 2, then run Full Quality Gate." -ForegroundColor $Global:UiWarnForeground
                }
                Pause-ForUser
            }
            "5" { Invoke-StandardControlAction -ControlAction "git-push"; Pause-ForUser }
            "6" { Invoke-StandardControlAction -ControlAction "git-remote"; Pause-ForUser }
            "7" { Invoke-StandardControlAction -ControlAction "repo-authority-audit"; Pause-ForUser }
            "8" { Invoke-StandardControlAction -ControlAction "git-repair"; Pause-ForUser }
            "9" {
                Write-Host ""
                Write-Host "Repository publication is destructive to the current remote main, but the old main will be archived first." -ForegroundColor $Global:UiWarnForeground
                Write-Host "A fresh read-only remote audit will run before the final confirmation." -ForegroundColor $Global:UiMutedForeground

                $auditCode = Invoke-StandardControlAction -ControlAction "repo-authority-audit" -ContinueOnError
                if ($auditCode -ne 0) {
                    Write-Host "Publication cancelled because the remote authority audit failed." -ForegroundColor $Global:UiWarnForeground
                    Pause-ForUser
                    continue
                }

                $publishConfirmation = Read-Host "Type PUBLISH to archive the current remote main and replace it with the certified normalized GREEN source"
                if ($publishConfirmation -cne "PUBLISH") {
                    Write-Host "Publication cancelled. No GitHub changes were made." -ForegroundColor $Global:UiWarnForeground
                    Pause-ForUser
                    continue
                }

                $publishCode = Invoke-StandardControlAction `
                    -ControlAction "repo-authority-publish" `
                    -Arguments @("-PublishConfirmation","PUBLISH") `
                    -ContinueOnError

                if ($publishCode -ne 0) {
                    Write-Host "Repository publication failed. The current debug bundle will identify the exact failing stage." -ForegroundColor $Global:UiWarnForeground
                }
                Pause-ForUser
            }
            "10" { Invoke-StandardControlAction -ControlAction "git-init"; Pause-ForUser }
            "0" { return }
        }
    }
}

function Show-MainMenu {
    while ($true) {
        Write-Header
        Write-Host "Full Gate is the promotion authority. External code/assets/dependencies only enter through" -ForegroundColor $Global:UiMutedForeground
        Write-Host "the governed supply chain; snapshots, transactional updates, gate history and rollback protect recovery." -ForegroundColor $Global:UiMutedForeground
        Write-Host ""
        Write-Host " 1. Build & verify"
        Write-Host " 2. Run & play"
        Write-Host " 3. Shipyard Dev Studio"
        Write-Host " 4. Asset authority & supply chain"
        Write-Host " 5. Project maintenance & diagnostics"
        Write-Host " 6. Packaging & baselines"
        Write-Host " 7. Logs & help"
        Write-Host " 8. Advanced / all registered commands"
        Write-Host " 9. Source control (GitHub optional)"
        Write-Host " 0. Exit"
        Write-Host ""
        try {
            switch (Read-Host "Select") {
                "1" { Show-BuildVerifyMenu }
                "2" { Show-RunPlayMenu }
                "3" { Invoke-RunSubspaceGame -GameArguments @("--shipyard"); Pause-ForUser }
                "4" { Show-AssetAuthorityMenu }
                "5" { Show-MaintenanceDiagnosticsMenu }
                "6" { Show-PackagingBaselineMenu }
                "7" { Show-LogsHelpMenu }
                "8" { Show-AdvancedMenu }
                "9" { Show-SourceControlMenu }
                "0" { return }
            }
        }
        catch {
            Write-Log $_.Exception.Message "ERROR"
            try {
                Invoke-DebugBundle
                Open-DebugHandoffFolder -BundlePath $Global:LastDebugBundlePath
            } catch { Write-Log $_.Exception.Message "ERROR" }
            Pause-ForUser
        }
    }
}


function Invoke-Pass9099Status {
    $script = Join-Path $Global:SubspaceRoot "scripts\subspace_pass90_99_status.ps1"
    if (-not (Test-Path $script)) { throw "Missing Pass90-99 status script: $script" }
    & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $script
    if ($LASTEXITCODE -ne 0) { throw "Pass90-99 status failed." }
    Add-StepResult -Name "Pass90-99 status" -Status "PASS"
    Write-StepSummary | Out-Null
}

try {
    switch ($Action) {
        "menu" { Show-MainMenu }
        "status" { Invoke-Status }
        "setup" { Invoke-Setup }
        "build-headless" { Invoke-BuildHeadless -Clean:$Clean }
        "build-render" { Invoke-BuildRender -Clean:$Clean }
        "build" { Invoke-BuildRender -Clean:$Clean }
        "test" { Invoke-TestsOnly }
        "clean" { Invoke-CleanBuildOutputs }
        "clean-logs" { Clear-Logs -PreserveCurrent }
        "debug-bundle" { Invoke-DebugBundle }
        "source-rollup" { Invoke-SourceRollup }
        "asset-rollup" { Invoke-AssetRollup }
        "root-audit" { Invoke-RootAudit }
        "verify" { Invoke-VerifyProject }
        "normalize-plan" { Invoke-NormalizePlan }
        "normalization-status" { Invoke-NormalizationStatus }
        "home-direction-status" { Invoke-HomeDirectionStatus }
        "project-status" { Invoke-ProjectStatus }
        "conversion-status" { Invoke-ConversionStatus }
        "pass90-99-status" { Invoke-Pass9099Status }
        "install-root-utility" { Invoke-InstallRootUtility }
        "preview-inbox" { Invoke-PreviewUpdateInbox }
        "apply-inbox" { Invoke-ApplyUpdateInbox }
        "patch-status" { Invoke-PatchStatus }
        "fetch-shipyard" { Invoke-FetchShipyard }
        "supply-chain-gate" { Invoke-SupplyChainGate -Mode "AUTO" }
        "source-snapshot" { Invoke-SourceSafetySnapshot -Label "MANUAL" }
        "audit-shipyard-references" { Invoke-AuditShipyardReferences }
        "full-gate" {
            Invoke-FullGate -CleanRoom:$Clean
            if ($Global:LastActionExitCode -ne 0) { exit $Global:LastActionExitCode }
        }
        "clean-full-gate" {
            Invoke-FullGate -CleanRoom
            if ($Global:LastActionExitCode -ne 0) { exit $Global:LastActionExitCode }
        }
        "fast-gate" {
            Invoke-FastDevelopmentGate
            if ($Global:LastActionExitCode -ne 0) { exit $Global:LastActionExitCode }
        }
        "artifact-index" { Invoke-ArtifactIndex }
        "compare-gates" { Invoke-CompareQualityGates }
        "dependency-status" { Invoke-DependencyStatus }
        "undo-last-patch" { Invoke-UndoLastPatch }
        "cpp-port-audit" { Invoke-CppPortAudit }
        "open-logs" { Open-LogsFolder }
        "run-game" { Invoke-RunSubspaceGame -GameArguments @("--frames", "600") }
        "run-shipyard" { Invoke-RunSubspaceGame -GameArguments @("--shipyard") }
        "run-smoke" { Invoke-RunSubspaceGame -GameArguments @("--runtime-smoke") }
        "run-loop" { Invoke-RunSubspaceGame -GameArguments @("--loop") }
        "health" { Invoke-StandardControlAction -ControlAction "health" }
        "control-center-self-test" { Invoke-StandardControlAction -ControlAction "self-test" }
        "environment-status" { Invoke-StandardControlAction -ControlAction "environment" }
        "project-audit-package" { Invoke-StandardControlAction -ControlAction "audit-package" }
        "capture-baseline" { Invoke-StandardControlAction -ControlAction "capture-baseline" }
        "incremental-handoff" { Invoke-StandardControlAction -ControlAction "incremental-handoff" }
        "warning-summary" { Invoke-StandardControlAction -ControlAction "warnings" }
        "git-status" { Invoke-StandardControlAction -ControlAction "git-status" }
        "git-init" { Invoke-StandardControlAction -ControlAction "git-init" }
        "git-history" { Invoke-StandardControlAction -ControlAction "git-history" }
        "git-commit-green" { Invoke-StandardControlAction -ControlAction "git-commit-green" }
        "git-push" { Invoke-StandardControlAction -ControlAction "git-push" }
        "git-remote" { Invoke-StandardControlAction -ControlAction "git-remote" }
        "repo-authority-audit" { Invoke-StandardControlAction -ControlAction "repo-authority-audit" }
        "repo-authority-prepare" { Invoke-StandardControlAction -ControlAction "repo-authority-prepare" }
        "repo-authority-publish" { Invoke-StandardControlAction -ControlAction "repo-authority-publish" }
        "open-project" { Open-ProjectFolder }
        "open-latest-debug" { Open-LatestDebugArtifact }
        "open-gate-history" { Open-GateHistoryFolder }
        "open-latest-gate" { Open-LatestGateRecord }
        "open-latest-failed-update" { Open-LatestFailedUpdate }
        "pass-continuity" { Invoke-PassContinuityAudit }
        "source-authority" { Invoke-ProjectSourceAuthorityCheck }
        "projectops-inspect" { Invoke-ProjectOpsInspect }
        "projectops-commands" { Invoke-ProjectOpsCommands }
        "project-shell" { Invoke-ProjectShellMenu }
        "projectops-repair" { Invoke-ProjectOpsRepair }
        "projectops-normalization-audit" { Invoke-ProjectOpsNormalizationAudit }
        "git-repair" { Invoke-StandardControlAction -ControlAction "git-repair" }
        "recover-pass655-674" { Invoke-RecoverPass655674 }
    }
}
catch {
    Write-Log $_.Exception.Message "ERROR"
    try {
        Invoke-DebugBundle
        Open-DebugHandoffFolder -BundlePath $Global:LastDebugBundlePath
    } catch { Write-Log $_.Exception.Message "ERROR" }
    exit 1
}

[CmdletBinding()]
param(
    [Parameter()]
    [string]$Root = (Get-Location).Path,

    [Parameter()]
    [string]$Snapshot = ""
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Write-Section([string]$Text) {
    Write-Host ""
    Write-Host ("=" * 78)
    Write-Host (" " + $Text)
    Write-Host ("=" * 78)
}

function Resolve-RepoRoot([string]$Candidate) {
    $resolved = (Resolve-Path -LiteralPath $Candidate).Path
    if (-not (Test-Path -LiteralPath (Join-Path $resolved "engine\CMakeLists.txt"))) {
        throw "Not a Codename Subspace repository root: $resolved"
    }
    if (-not (Test-Path -LiteralPath (Join-Path $resolved "SubspaceTools.ps1"))) {
        throw "SubspaceTools.ps1 was not found at repository root: $resolved"
    }
    return $resolved
}

function Find-GreenSnapshot([string]$RepoRoot, [string]$Explicit) {
    if ($Explicit) {
        $p = (Resolve-Path -LiteralPath $Explicit).Path
        if (-not (Test-Path -LiteralPath $p -PathType Leaf)) {
            throw "Requested snapshot does not exist: $p"
        }
        return $p
    }

    # Exact known-good authority from QG-20260911-175455-full-cdc7bef7.
    $exact = Join-Path $RepoRoot "artifacts\snapshots\source\Codename_Subspace_CERTIFIED_SourceSnapshot_20260911-175416.zip"
    if (Test-Path -LiteralPath $exact -PathType Leaf) {
        return $exact
    }

    # Fallback only to a certified snapshot, never to an arbitrary failed/debug bundle.
    $dir = Join-Path $RepoRoot "artifacts\snapshots\source"
    if (Test-Path -LiteralPath $dir -PathType Container) {
        $candidate = Get-ChildItem -LiteralPath $dir -File -Filter "Codename_Subspace_CERTIFIED_SourceSnapshot_*.zip" |
            Sort-Object LastWriteTime -Descending |
            Select-Object -First 1
        if ($candidate) {
            return $candidate.FullName
        }
    }

    throw "No certified Subspace source snapshot was found under artifacts\snapshots\source."
}

function Find-SnapshotFile([string]$ExtractRoot, [string]$RelativePath) {
    $needle = $RelativePath.Replace("\", "/")
    $matches = @(Get-ChildItem -LiteralPath $ExtractRoot -Recurse -File | Where-Object {
        $_.FullName.Replace("\", "/").EndsWith("/" + $needle, [System.StringComparison]::OrdinalIgnoreCase) -or
        $_.FullName.Replace("\", "/").EndsWith($needle, [System.StringComparison]::OrdinalIgnoreCase)
    })
    if ($matches.Count -eq 0) {
        throw "Snapshot is missing required file: $RelativePath"
    }
    if ($matches.Count -gt 1) {
        throw "Snapshot contains multiple candidates for $RelativePath; refusing ambiguous restore."
    }
    return $matches[0].FullName
}

function Backup-CurrentFile([string]$RepoRoot, [string]$BackupRoot, [string]$RelativePath) {
    $src = Join-Path $RepoRoot $RelativePath
    if (-not (Test-Path -LiteralPath $src -PathType Leaf)) {
        throw "Current repository is missing required file: $RelativePath"
    }
    $dst = Join-Path $BackupRoot $RelativePath
    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $dst) | Out-Null
    Copy-Item -LiteralPath $src -Destination $dst -Force
}

function Restore-FromSnapshot([string]$RepoRoot, [string]$ExtractRoot, [string]$RelativePath) {
    $src = Find-SnapshotFile -ExtractRoot $ExtractRoot -RelativePath $RelativePath
    $dst = Join-Path $RepoRoot $RelativePath
    Copy-Item -LiteralPath $src -Destination $dst -Force
    # Force incremental CMake/MSBuild dependency invalidation even if the ZIP preserved
    # an older source timestamp.
    (Get-Item -LiteralPath $dst).LastWriteTime = Get-Date
    $hash = (Get-FileHash -Algorithm SHA256 -LiteralPath $dst).Hash
    Write-Host "[RESTORE] $RelativePath"
    Write-Host "          SHA256=$hash"
}

function Rollback-CurrentFiles([string]$RepoRoot, [string]$BackupRoot, [string[]]$RelativePaths) {
    Write-Section "ROLLING BACK REPAIR ATTEMPT"
    foreach ($rel in $RelativePaths) {
        $src = Join-Path $BackupRoot $rel
        if (Test-Path -LiteralPath $src -PathType Leaf) {
            $dst = Join-Path $RepoRoot $rel
            Copy-Item -LiteralPath $src -Destination $dst -Force
            (Get-Item -LiteralPath $dst).LastWriteTime = Get-Date
            Write-Host "[ROLLBACK] $rel"
        }
    }
}

function Invoke-NativeChecked([string]$Label, [scriptblock]$Command) {
    Write-Host ""
    Write-Host "[RUN] $Label"
    & $Command
    $code = $LASTEXITCODE
    if ($null -eq $code) { $code = 0 }
    Write-Host "[EXIT] $Label -> $code"
    return [int]$code
}

function Invoke-TargetedValidation([string]$RepoRoot) {
    $build = Join-Path $RepoRoot "engine\build"
    if (-not (Test-Path -LiteralPath $build -PathType Container)) {
        Write-Host "[INFO] Existing engine\build tree is absent; configuring it."
        $configure = Invoke-NativeChecked "CMake configure" {
            cmake -S (Join-Path $RepoRoot "engine") -B $build `
                -DSUBSPACE_HEADLESS=OFF -DSUBSPACE_BUILD_OPENGL=ON -DSUBSPACE_BUILD_TESTS=ON
        }
        if ($configure -ne 0) { return $configure }
    }

    $buildCode = Invoke-NativeChecked "CMake incremental build" {
        cmake --build $build --config Debug --parallel 8
    }
    if ($buildCode -ne 0) { return $buildCode }

    $regex = "SubspacePass595To614KitbashConstructionUpgradeTests|SubspacePass615To654KitbashRuntimeClosureTests"
    return Invoke-NativeChecked "Targeted CTest Pass595-654" {
        ctest --test-dir $build -C Debug -R $regex --output-on-failure --timeout 120
    }
}

$Root = Resolve-RepoRoot $Root
$Snapshot = Find-GreenSnapshot -RepoRoot $Root -Explicit $Snapshot

$stamp = Get-Date -Format "yyyyMMdd-HHmmss"
$repairRoot = Join-Path $Root ("artifacts\recovery\pass890-kitbash-regression-" + $stamp)
$backupRoot = Join-Path $repairRoot "pre-repair"
$extractRoot = Join-Path $repairRoot "green-snapshot"
$reportPath = Join-Path $repairRoot "REPAIR_REPORT.txt"

New-Item -ItemType Directory -Force -Path $backupRoot, $extractRoot | Out-Null

$stageA = @(
    "engine\src\ships\ShipClassRoleSystem.cpp",
    "engine\src\ships\FactionShipDesignSystem.cpp"
)
$stageB = @(
    "engine\src\ships\ShipPcgRuntimeClosureSystem.cpp"
)
$all = @($stageA + $stageB)

Write-Section "CODENAME SUBSPACE PASS890 KITBASH REGRESSION REPAIR"
Write-Host "Repository : $Root"
Write-Host "Snapshot   : $Snapshot"
Write-Host "Recovery   : $repairRoot"
Write-Host ""
Write-Host "Known failure surface:"
Write-Host "  Pass613 - frigate/battleship structural envelope"
Write-Host "  Pass638 - faction/class/hull-family/role lineage"
Write-Host ""
Write-Host "This repair does NOT roll back Pass791-890 globally."
Write-Host "It restores only the smallest implicated ship class/lineage authorities."

foreach ($rel in $all) {
    Backup-CurrentFile -RepoRoot $Root -BackupRoot $backupRoot -RelativePath $rel
}

Write-Host ""
Write-Host "[INFO] Expanding certified GREEN snapshot..."
Expand-Archive -LiteralPath $Snapshot -DestinationPath $extractRoot -Force

# Validate every possible restore target before touching current source.
foreach ($rel in $all) {
    [void](Find-SnapshotFile -ExtractRoot $extractRoot -RelativePath $rel)
}

$restored = New-Object System.Collections.Generic.List[string]

try {
    Write-Section "STAGE A - RESTORE CLASS ENVELOPE + LINEAGE VALIDATOR"
    foreach ($rel in $stageA) {
        Restore-FromSnapshot -RepoRoot $Root -ExtractRoot $extractRoot -RelativePath $rel
        $restored.Add($rel)
    }

    $stageACode = Invoke-TargetedValidation -RepoRoot $Root
    if ($stageACode -eq 0) {
        Write-Host ""
        Write-Host "[PASS] Stage A repaired both failing CTest targets. Runtime-closure source was left untouched."
    }
    else {
        Write-Section "STAGE B - RESTORE RUNTIME LINEAGE WRITER"
        foreach ($rel in $stageB) {
            Restore-FromSnapshot -RepoRoot $Root -ExtractRoot $extractRoot -RelativePath $rel
            $restored.Add($rel)
        }

        $stageBCode = Invoke-TargetedValidation -RepoRoot $Root
        if ($stageBCode -ne 0) {
            Rollback-CurrentFiles -RepoRoot $Root -BackupRoot $backupRoot -RelativePaths $restored.ToArray()
            @"
CODENAME SUBSPACE PASS890 KITBASH REGRESSION REPAIR
Result: FAILED / AUTOMATIC ROLLBACK COMPLETE
Timestamp: $(Get-Date -Format o)
Repository: $Root
Snapshot: $Snapshot
Targeted validation exit code: $stageBCode
Current source was restored from the pre-repair backup.
"@ | Set-Content -LiteralPath $reportPath -Encoding UTF8
            Write-Host ""
            Write-Host "[FAIL] Targeted tests are still failing. Automatic source rollback completed."
            Write-Host "Report: $reportPath"
            exit $stageBCode
        }
        Write-Host ""
        Write-Host "[PASS] Stage B repaired both failing CTest targets."
    }
}
catch {
    if ($restored.Count -gt 0) {
        Rollback-CurrentFiles -RepoRoot $Root -BackupRoot $backupRoot -RelativePaths $restored.ToArray()
    }
    @"
CODENAME SUBSPACE PASS890 KITBASH REGRESSION REPAIR
Result: ERROR / AUTOMATIC ROLLBACK COMPLETE
Timestamp: $(Get-Date -Format o)
Repository: $Root
Snapshot: $Snapshot
Error: $($_.Exception.Message)
"@ | Set-Content -LiteralPath $reportPath -Encoding UTF8
    Write-Error $_
    Write-Host "Report: $reportPath"
    exit 1
}

Write-Section "TARGETED REPAIR GREEN - RUNNING FULL QUALITY GATE"
$pwsh = Get-Command pwsh -ErrorAction SilentlyContinue
if (-not $pwsh) { $pwsh = Get-Command powershell -ErrorAction Stop }

$fullGateCode = Invoke-NativeChecked "Subspace Full Quality Gate" {
    & $pwsh.Source -NoProfile -ExecutionPolicy Bypass -File (Join-Path $Root "SubspaceTools.ps1") -Action full-gate
}

$restoredText = ($restored | ForEach-Object { "  - $_" }) -join [Environment]::NewLine
@"
CODENAME SUBSPACE PASS890 KITBASH REGRESSION REPAIR
Timestamp: $(Get-Date -Format o)
Repository: $Root
Certified GREEN snapshot: $Snapshot

Targeted validation: PASS
Restored files:
$restoredText

Full quality gate exit code: $fullGateCode
Backup of pre-repair source: $backupRoot

If the full gate failed outside Pass595-654, the known Pass613/Pass638 regression
remains repaired and the full-gate log should be used for the next issue.
"@ | Set-Content -LiteralPath $reportPath -Encoding UTF8

Write-Host ""
if ($fullGateCode -eq 0) {
    Write-Host "[PASS] PASS890 kitbash regression repair is FULL-GATE GREEN."
} else {
    Write-Host "[WARN] Targeted repair is GREEN, but the full quality gate reported another failure."
}
Write-Host "Report: $reportPath"
Write-Host "Backup: $backupRoot"
exit $fullGateCode

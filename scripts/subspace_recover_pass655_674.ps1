param(
    [string]$Root = (Get-Location).Path,
    [string]$ArchivePath = "",
    [switch]$DeepSearch
)

$ErrorActionPreference = "Stop"
$Root = [System.IO.Path]::GetFullPath($Root)

$expectedName = "Codename_Subspace_Pass655_674_ConstructionSymmetryCameraThumbnail_20260907.zip"
$requiredRelative = "engine\tests\pass655_674_editor_symmetry_camera_thumbnail_tests.cpp"
$replayPattern = "*Pass675_744*ShipyardDevStudio*PCCv2*.zip"

function Resolve-PayloadRoot {
    param([string]$Extracted)
    if (Test-Path -LiteralPath (Join-Path $Extracted "engine\CMakeLists.txt")) { return $Extracted }
    $children = @(Get-ChildItem -LiteralPath $Extracted -Directory -Force -ErrorAction SilentlyContinue)
    if ($children.Count -eq 1 -and (Test-Path -LiteralPath (Join-Path $children[0].FullName "engine\CMakeLists.txt"))) {
        return $children[0].FullName
    }
    throw "Archive payload root could not be resolved: $Extracted"
}

function Find-Archive {
    param([string]$ExactName)
    $candidates = [System.Collections.Generic.List[string]]::new()
    $roots = [System.Collections.Generic.List[string]]::new()
    foreach ($r in @(
        $Root,
        (Join-Path $Root "updates\inbox"),
        (Join-Path $Root "updates\applied"),
        (Join-Path $Root "updates\failed"),
        (Join-Path $Root "updates\superseded"),
        (Split-Path -Parent $Root),
        (Join-Path $env:USERPROFILE "Downloads"),
        (Join-Path $env:USERPROFILE "Desktop"),
        (Join-Path $env:USERPROFILE "Documents")
    )) {
        if ($r -and (Test-Path -LiteralPath $r)) { $roots.Add($r) | Out-Null }
    }

    foreach ($searchRoot in $roots) {
        try {
            foreach ($f in @(Get-ChildItem -LiteralPath $searchRoot -Filter $ExactName -File -Recurse -Depth 5 -ErrorAction SilentlyContinue)) {
                if (-not $candidates.Contains($f.FullName)) { $candidates.Add($f.FullName) | Out-Null }
            }
        } catch {}
    }

    if ($DeepSearch -and (Test-Path -LiteralPath "D:\")) {
        Write-Host "[INFO] Deep-searching D:\ for $ExactName ..."
        try {
            foreach ($f in @(Get-ChildItem -LiteralPath "D:\" -Filter $ExactName -File -Recurse -Depth 7 -ErrorAction SilentlyContinue)) {
                if (-not $candidates.Contains($f.FullName)) { $candidates.Add($f.FullName) | Out-Null }
            }
        } catch {}
    }

    return $candidates.ToArray()
}

function Get-PayloadMap {
    param([string]$PayloadRoot)
    $map = @{}
    foreach ($file in @(Get-ChildItem -LiteralPath $PayloadRoot -Recurse -File -Force)) {
        $rel = $file.FullName.Substring($PayloadRoot.Length).TrimStart([char[]]'\/').Replace('/','\')
        if ($rel -ieq "PATCH_MANIFEST.json") { continue }
        $map[$rel.ToLowerInvariant()] = [pscustomobject]@{ Relative=$rel; FullName=$file.FullName }
    }
    return $map
}

Write-Host "========================================================================" -ForegroundColor Cyan
Write-Host " SUBSPACE PASS655-674 BASELINE GAP RECOVERY" -ForegroundColor Cyan
Write-Host "========================================================================" -ForegroundColor Cyan
Write-Host "Root: $Root"

if ([string]::IsNullOrWhiteSpace($ArchivePath)) {
    $found = @(Find-Archive -ExactName $expectedName)
    if ($found.Count -eq 0) {
        throw @"
The original Pass655-674 handoff was not found locally.

Required archive:
  $expectedName

The current repository history proves Pass675-744 was applied over Pass654.
Do not disable subspace_pass655_674_tests and do not fabricate a replacement
test. Put the original 39-file handoff in the repository root (or Downloads)
and run this recovery action again. Use -DeepSearch to include D:\.
"@
    }
    if ($found.Count -gt 1) {
        Write-Host "[INFO] Multiple candidates found; using newest:"
        $items = @($found | ForEach-Object { Get-Item -LiteralPath $_ } | Sort-Object LastWriteTimeUtc -Descending)
        foreach ($i in $items) { Write-Host ("  " + $i.FullName) }
        $ArchivePath = $items[0].FullName
    } else {
        $ArchivePath = $found[0]
    }
}

$ArchivePath = [System.IO.Path]::GetFullPath($ArchivePath)
if (-not (Test-Path -LiteralPath $ArchivePath -PathType Leaf)) {
    throw "Pass655-674 archive not found: $ArchivePath"
}
Write-Host "Pass655-674 archive: $ArchivePath"

$newerArchive = Get-ChildItem -LiteralPath (Join-Path $Root "updates\applied") -Filter $replayPattern -File -ErrorAction SilentlyContinue |
    Sort-Object LastWriteTimeUtc -Descending |
    Select-Object -First 1
if (-not $newerArchive) {
    throw "Cannot safely reconstruct the gap because the applied Pass675-744 archive was not found under updates\applied."
}
Write-Host "Newer cumulative archive: $($newerArchive.FullName)"

$stamp = Get-Date -Format "yyyyMMdd-HHmmss"
$stageRoot = Join-Path $Root ("updates\staging\pass655-674-recovery-" + $stamp)
$oldExtract = Join-Path $stageRoot "pass655"
$newExtract = Join-Path $stageRoot "pass675"
$backupRoot = Join-Path $Root (".subspace\recovery\pass655-674-gap-" + $stamp)
$transaction = Join-Path $backupRoot "transaction.json"
New-Item -ItemType Directory -Force -Path $oldExtract,$newExtract,$backupRoot | Out-Null

try {
    Expand-Archive -LiteralPath $ArchivePath -DestinationPath $oldExtract -Force
    Expand-Archive -LiteralPath $newerArchive.FullName -DestinationPath $newExtract -Force

    $oldPayload = Resolve-PayloadRoot $oldExtract
    $newPayload = Resolve-PayloadRoot $newExtract
    $oldMap = Get-PayloadMap $oldPayload
    $newMap = Get-PayloadMap $newPayload

    if ($oldMap.Count -lt 35) {
        throw "Pass655-674 archive is unexpectedly small ($($oldMap.Count) files); expected the original ~39-file handoff."
    }
    if (-not $oldMap.ContainsKey($requiredRelative.ToLowerInvariant())) {
        throw "Pass655-674 archive does not contain required source: $requiredRelative"
    }

    $operations = [System.Collections.Generic.List[object]]::new()

    foreach ($key in @($oldMap.Keys | Sort-Object)) {
        $oldEntry = $oldMap[$key]
        $dest = Join-Path $Root $oldEntry.Relative
        $source = $oldEntry.FullName
        $sourceAuthority = "Pass655-674"

        if ($newMap.ContainsKey($key)) {
            # A newer cumulative file already owns this path. Preserve the live
            # current file when it exists; if it is missing, recover the newer
            # Pass675-744 version instead of downgrading to Pass655-674.
            if (Test-Path -LiteralPath $dest -PathType Leaf) {
                continue
            }
            $source = $newMap[$key].FullName
            $sourceAuthority = "Pass675-744"
        }

        $existed = Test-Path -LiteralPath $dest -PathType Leaf
        $backup = if ($existed) { Join-Path $backupRoot $oldEntry.Relative } else { "" }
        if ($existed) {
            New-Item -ItemType Directory -Force -Path (Split-Path -Parent $backup) | Out-Null
            Copy-Item -LiteralPath $dest -Destination $backup -Force
        }

        $operations.Add([pscustomobject]@{
            relative=$oldEntry.Relative
            existed=$existed
            backup=$backup
            source=$source
            sourceAuthority=$sourceAuthority
        }) | Out-Null
    }

    [pscustomobject]@{
        schemaVersion=1
        timestamp=(Get-Date).ToString("o")
        state="PREPARED"
        pass655Archive=$ArchivePath
        pass675Archive=$newerArchive.FullName
        operations=$operations.ToArray()
    } | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $transaction -Encoding UTF8

    foreach ($op in $operations) {
        $dest = Join-Path $Root $op.relative
        New-Item -ItemType Directory -Force -Path (Split-Path -Parent $dest) | Out-Null
        Copy-Item -LiteralPath $op.source -Destination $dest -Force
        Write-Host ("[APPLY] {0} <- {1}" -f $op.relative,$op.sourceAuthority)
    }

    $audit = Join-Path $Root "scripts\subspace_pass_continuity_audit.ps1"
    & $audit -Root $Root
    if ($LASTEXITCODE -ne 0) { throw "Pass/source continuity audit still fails after reconstruction." }

    $record = Get-Content -LiteralPath $transaction -Raw | ConvertFrom-Json
    $record.state = "COMMITTED"
    $record | Add-Member -NotePropertyName committedAt -NotePropertyValue (Get-Date).ToString("o") -Force
    $record | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $transaction -Encoding UTF8

    Write-Host ""
    Write-Host "[PASS] Pass655-674 baseline gap reconstructed without downgrading newer Pass675-745 paths." -ForegroundColor Green
    Write-Host "Transaction: $transaction"
    Write-Host "Next: run Full quality gate."
}
catch {
    Write-Host "[FAIL] Recovery failed: $($_.Exception.Message)" -ForegroundColor Red
    if (Test-Path -LiteralPath $transaction) {
        try {
            $record = Get-Content -LiteralPath $transaction -Raw | ConvertFrom-Json
            foreach ($op in @($record.operations) | Sort-Object -Property relative -Descending) {
                $dest = Join-Path $Root ([string]$op.relative)
                if ([bool]$op.existed) {
                    if (Test-Path -LiteralPath ([string]$op.backup)) {
                        New-Item -ItemType Directory -Force -Path (Split-Path -Parent $dest) | Out-Null
                        Copy-Item -LiteralPath ([string]$op.backup) -Destination $dest -Force
                    }
                } elseif (Test-Path -LiteralPath $dest) {
                    Remove-Item -LiteralPath $dest -Force
                }
            }
            $record.state = "ROLLED_BACK"
            $record | Add-Member -NotePropertyName rolledBackAt -NotePropertyValue (Get-Date).ToString("o") -Force
            $record | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $transaction -Encoding UTF8
            Write-Host "[PASS] Partial recovery rolled back." -ForegroundColor Yellow
        } catch {
            Write-Host "[WARN] Automatic rollback encountered an error: $($_.Exception.Message)" -ForegroundColor Yellow
        }
    }
    throw
}
finally {
    if (Test-Path -LiteralPath $stageRoot) {
        Remove-Item -LiteralPath $stageRoot -Recurse -Force -ErrorAction SilentlyContinue
    }
}

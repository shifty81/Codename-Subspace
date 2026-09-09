param(
    [string]$Root = (Get-Location).Path,
    [switch]$DryRun,
    [switch]$BuildAfter,
    [switch]$Clean
)

$ErrorActionPreference = "Stop"
$Root = [System.IO.Path]::GetFullPath($Root)
$Timestamp = Get-Date -Format "yyyyMMdd-HHmmss"
$Inbox = Join-Path $Root "updates\inbox"
$Applied = Join-Path $Root "updates\applied"
$Failed = Join-Path $Root "updates\failed"
$Superseded = Join-Path $Root "updates\superseded"
$Staging = Join-Path $Root "updates\staging"
$Backups = Join-Path $Root "updates\backups"
$Transactions = Join-Path $Root "updates\transactions"
$Logs = Join-Path $Root "updates\logs"
$Latest = Join-Path $Root "updates\LATEST_UPDATE_APPLY.txt"

foreach ($dir in @($Inbox,$Applied,$Failed,$Superseded,$Staging,$Backups,$Transactions,$Logs)) {
    New-Item -ItemType Directory -Force -Path $dir | Out-Null
}

$LogFile = Join-Path $Logs "update-inbox-$Timestamp.log"
$StepResults = [System.Collections.Generic.List[object]]::new()

function LogLine([string]$Message, [string]$Level = "INFO") {
    $line = "[{0}] [{1}] {2}" -f (Get-Date -Format "yyyy-MM-dd HH:mm:ss"), $Level, $Message
    $color = switch ($Level) { "PASS" { "Green" } "FAIL" { "Red" } "WARN" { "Yellow" } "STEP" { "Cyan" } default { "Gray" } }
    Write-Host $line -ForegroundColor $color
    Add-Content -LiteralPath $LogFile -Value $line
}

function AddResult([string]$Name, [string]$Status, [string]$Message = "") {
    $StepResults.Add([pscustomobject]@{ Name = $Name; Status = $Status; Message = $Message }) | Out-Null
}

function WriteLatest([string]$Result) {
    @(
        "Codename Subspace update inbox result",
        "Result: $Result",
        "Timestamp: $Timestamp",
        "Root: $Root",
        "Inbox: $Inbox",
        "Applied: $Applied",
        "Failed: $Failed",
        "Superseded: $Superseded",
        "Log: $LogFile",
        "Mode: $(if ($DryRun) { 'DRY-RUN' } else { 'APPLY' })",
        "",
        "Patch results:",
        ($StepResults | ForEach-Object { "- [$($_.Status)] $($_.Name) $($_.Message)" })
    ) | Set-Content -LiteralPath $Latest
}


function Get-FileSha256([string]$Path) {
    if (-not (Test-Path -LiteralPath $Path)) { return $null }
    return (Get-FileHash -Algorithm SHA256 -LiteralPath $Path).Hash.ToLowerInvariant()
}

function Restore-OverlayTransaction([System.Collections.Generic.List[object]]$Entries, [string]$BackupRoot) {
    LogLine "Rolling back partially applied patch transaction." "WARN"
    for ($i = $Entries.Count - 1; $i -ge 0; $i--) {
        $entry = $Entries[$i]
        $dest = Join-Path $Root $entry.relative
        if ([bool]$entry.existed) {
            $backupFile = Join-Path $BackupRoot $entry.relative
            if (-not (Test-Path -LiteralPath $backupFile)) { throw "Rollback backup missing: $backupFile" }
            New-Item -ItemType Directory -Force -Path (Split-Path -Parent $dest) | Out-Null
            Copy-Item -LiteralPath $backupFile -Destination $dest -Force
        }
        elseif (Test-Path -LiteralPath $dest) {
            Remove-Item -LiteralPath $dest -Force
        }
    }
    LogLine "Patch transaction rollback completed." "PASS"
}

function Get-PayloadRoot([string]$Stage) {
    $items = Get-ChildItem -LiteralPath $Stage -Force
    $dirs = @($items | Where-Object { $_.PSIsContainer })
    $files = @($items | Where-Object { -not $_.PSIsContainer })
    if ($dirs.Count -eq 1 -and $files.Count -eq 0) {
        $candidate = $dirs[0].FullName
        $names = @(Get-ChildItem -LiteralPath $candidate -Force | ForEach-Object { $_.Name })
        if ($names -contains "engine" -or $names -contains "scripts" -or $names -contains "docs" -or $names -contains "content" -or $names -contains "SubspaceTools.ps1" -or $names -contains "PATCH_MANIFEST.md") {
            return $candidate
        }
    }
    return $Stage
}

function Get-MaxPassFromName([string]$Name) {
    if ([string]::IsNullOrWhiteSpace($Name)) { return 0 }
    $maxPass = 0
    # Pass numbers are short project generations (for example Pass463 or Pass451_455).
    # Historical filenames also contain date suffixes such as PASS49_20260822.  The old
    # parser treated that date as "Pass20260822" and then rejected every real patch as
    # superseded.  Ignore numeric candidates outside the supported pass-number range.
    foreach ($match in [regex]::Matches($Name, '(?i)Pass(?<First>\d+)(?:[_-](?<Last>\d+))?')) {
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
    return $maxPass
}

function Get-RepositoryPass() {
    $maxPass = 0
    $docsRoot = Join-Path $Root "docs"
    if (Test-Path -LiteralPath $docsRoot) {
        foreach ($file in @(Get-ChildItem -LiteralPath $docsRoot -Recurse -File -ErrorAction SilentlyContinue)) {
            $value = Get-MaxPassFromName $file.Name
            if ($value -gt $maxPass) { $maxPass = $value }
        }
    }
    return $maxPass
}

function Archive-SupersededPatch([System.IO.FileInfo]$Patch, [int]$PatchPass, [int]$CurrentPass) {
    $name = $Patch.Name
    $dest = Join-Path $Superseded ("{0}_current-pass-{1}_{2}" -f $Timestamp, $CurrentPass, $name)
    if ($DryRun) {
        LogLine "Would skip superseded patch $name (patch Pass$PatchPass < repository Pass$CurrentPass)." "WARN"
        AddResult $name "SKIP" "Superseded: patch Pass$PatchPass < repository Pass$CurrentPass."
        return
    }
    Move-Item -LiteralPath $Patch.FullName -Destination $dest -Force
    LogLine "Skipped superseded patch $name (patch Pass$PatchPass < repository Pass$CurrentPass); archived to $dest" "WARN"
    AddResult $name "SKIP" "Superseded by repository Pass$CurrentPass; archived without overlay."
}

function Assert-SafeRelativePath([string]$Relative) {
    if ([string]::IsNullOrWhiteSpace($Relative)) { throw "Unsafe blank relative path." }
    if ($Relative.StartsWith("..")) { throw "Unsafe relative path outside payload: $Relative" }
    if ([System.IO.Path]::IsPathRooted($Relative)) { throw "Unsafe rooted path in payload: $Relative" }
    if ($Relative -match '(^|[\\/])\.git([\\/]|$)') { throw "Patch payload may not write .git content: $Relative" }
    if ($Relative -match '(^|[\\/])updates[\\/]inbox([\\/]|$)') { throw "Patch payload may not write into updates\\inbox: $Relative" }

    $normalized = $Relative.Replace('/', '\').TrimStart('\')
    if ($normalized -ieq 'SubspaceTools.cmd') {
        throw "SubspaceTools.cmd is the active root launcher and is bootstrap-only. Normal root-drop patches may not overwrite or remove the running CMD. Use a one-time extract/overwrite bootstrap if the launcher itself must change."
    }
}


function Get-PatchManifestInfo([string]$Payload) {
    $manifestPath = Join-Path $Payload "PATCH_MANIFEST.json"
    if (-not (Test-Path -LiteralPath $manifestPath)) {
        return [pscustomobject]@{ Present=$false; Path=$null; FileEntries=@(); RemoveEntries=@() }
    }

    try { $manifest = Get-Content -LiteralPath $manifestPath -Raw | ConvertFrom-Json }
    catch { throw "PATCH_MANIFEST.json is not valid JSON: $($_.Exception.Message)" }

    if ([int]$manifest.schemaVersion -ne 1) { throw "Unsupported PATCH_MANIFEST.json schemaVersion: $($manifest.schemaVersion)" }
    if ($null -eq $manifest.files) { throw "PATCH_MANIFEST.json must contain a files array." }

    $manifestFiles = @($manifest.files)
    $manifestRemovals = @($manifest.remove)
    $expected = @{}
    $fileEntries = [System.Collections.Generic.List[object]]::new()
    foreach ($entry in $manifestFiles) {
        $relative = [string]$entry.path
        Assert-SafeRelativePath $relative
        if ($relative -ieq "PATCH_MANIFEST.json") { throw "PATCH_MANIFEST.json may not list itself as an overlay file." }
        $key = $relative.Replace('/', '\').ToLowerInvariant()
        if ($expected.ContainsKey($key)) { throw "PATCH_MANIFEST.json contains duplicate file path: $relative" }
        $sha = ([string]$entry.sha256).ToLowerInvariant()
        if ($sha -notmatch '^[0-9a-f]{64}$') { throw "PATCH_MANIFEST.json has invalid sha256 for $relative" }
        $bytes = [int64]$entry.bytes
        if ($bytes -lt 0) { throw "PATCH_MANIFEST.json has invalid byte count for $relative" }
        $expected[$key] = $true
        $fileEntries.Add([pscustomobject]@{ path=$relative; sha256=$sha; bytes=$bytes }) | Out-Null
    }

    $removeSeen = @{}
    $removeEntries = [System.Collections.Generic.List[string]]::new()
    foreach ($raw in $manifestRemovals) {
        $relative = [string]$raw
        Assert-SafeRelativePath $relative
        $key = $relative.Replace('/', '\').ToLowerInvariant()
        if ($expected.ContainsKey($key)) { throw "PATCH_MANIFEST.json cannot overlay and remove the same path: $relative" }
        if ($removeSeen.ContainsKey($key)) { throw "PATCH_MANIFEST.json contains duplicate removal path: $relative" }
        $removeSeen[$key] = $true
        $removeEntries.Add($relative) | Out-Null
    }

    $actualFiles = @(Get-ChildItem -LiteralPath $Payload -Recurse -File -Force | Where-Object {
        $_.FullName -notmatch "[\\/]\.git[\\/]" -and $_.FullName -ne $manifestPath
    })
    $actual = @{}
    foreach ($file in $actualFiles) {
        $relative = [System.IO.Path]::GetRelativePath($Payload, $file.FullName)
        Assert-SafeRelativePath $relative
        $key = $relative.Replace('/', '\').ToLowerInvariant()
        if ($actual.ContainsKey($key)) { throw "Patch contains duplicate destination path: $relative" }
        $actual[$key] = $file
    }

    foreach ($entry in $fileEntries) {
        $key = ([string]$entry.path).Replace('/', '\').ToLowerInvariant()
        if (-not $actual.ContainsKey($key)) { throw "PATCH_MANIFEST.json lists missing payload file: $($entry.path)" }
        $file = $actual[$key]
        if ([int64]$file.Length -ne [int64]$entry.bytes) { throw "PATCH_MANIFEST.json byte count mismatch: $($entry.path)" }
        $sha = Get-FileSha256 $file.FullName
        if ($sha -ne ([string]$entry.sha256).ToLowerInvariant()) { throw "PATCH_MANIFEST.json SHA-256 mismatch: $($entry.path)" }
    }
    foreach ($key in $actual.Keys) {
        if (-not $expected.ContainsKey($key)) { throw "Payload contains an unmanifested file: $($actual[$key].FullName)" }
    }

    return [pscustomobject]@{
        Present=$true
        Path=$manifestPath
        FileEntries=$fileEntries.ToArray()
        RemoveEntries=$removeEntries.ToArray()
    }
}

LogLine "Subspace root-drop update inbox processor" "STEP"
LogLine "Root: $Root"
LogLine "Inbox: $Inbox"
LogLine "Mode: $(if ($DryRun) { 'DRY-RUN' } else { 'APPLY' })"
LogLine "Patch format: ZIP containing repo-root-relative files, or one top-level folder containing repo-root-relative files."

# Project-standard convenience: patch handoff ZIPs are often dropped directly in
# the repository root. Promote matching root-level patch ZIPs into updates\inbox
# before processing so build/full-gate consumes them automatically.
# Source rollups/debug bundles are intentionally ignored.
$rootPatchPatterns = @(
    "Subspace_Pass*.zip",
    "Subspace_Patch*.zip",
    "Subspace_Hotfix*.zip",
    "Subspace_RootDrop*.zip",
    "Subspace_ROOT_DROP*.zip",
    "Subspace_Update*.zip",
    "Codename_Subspace_Pass*.zip",
    "Codename_Subspace_Patch*.zip",
    "Codename_Subspace_Hotfix*.zip",
    "Codename_Subspace_RootDrop*.zip",
    "Codename_Subspace_ROOT_DROP*.zip",
    "Codename_Subspace_Update*.zip"
)
$rootPatchCandidates = @()
foreach ($pattern in $rootPatchPatterns) {
    $rootPatchCandidates += @(Get-ChildItem -LiteralPath $Root -Filter $pattern -File -ErrorAction SilentlyContinue | Where-Object {
        $_.Name -notmatch "FullSource|BuildRollup|DebugBundle|SourceRollup|sha256"
    })
}
$rootPatchCandidates = @($rootPatchCandidates | Sort-Object FullName -Unique)
foreach ($rootPatch in $rootPatchCandidates) {
    $dest = Join-Path $Inbox $rootPatch.Name
    if ($DryRun) {
        LogLine "Root-drop patch would be queued: $($rootPatch.Name)" "WARN"
    }
    else {
        if (Test-Path -LiteralPath $dest) {
            $base = [System.IO.Path]::GetFileNameWithoutExtension($rootPatch.Name)
            $ext = [System.IO.Path]::GetExtension($rootPatch.Name)
            $dest = Join-Path $Inbox ("{0}_{1}{2}" -f $base, $Timestamp, $ext)
        }
        Move-Item -LiteralPath $rootPatch.FullName -Destination $dest -Force
        LogLine "Queued root-drop patch from repo root: $($rootPatch.Name)" "PASS"
    }
}

$patches = @(Get-ChildItem -LiteralPath $Inbox -Filter *.zip -File -ErrorAction SilentlyContinue | Sort-Object Name)
if ($patches.Count -eq 0) {
    LogLine "No patch ZIPs found in updates\inbox or repo root." "PASS"
    AddResult "Update inbox/root-drop" "PASS" "No queued patch ZIPs."
    WriteLatest "PASS"
    exit 0
}

$failures = 0
$CurrentPass = Get-RepositoryPass
LogLine "Repository pass authority before update apply: $(if ($CurrentPass -gt 0) { 'Pass' + $CurrentPass } else { 'UNKNOWN' })"
foreach ($patch in $patches) {
    $patchPass = Get-MaxPassFromName $patch.Name
    if ($patchPass -gt 0 -and $CurrentPass -gt 0 -and $patchPass -lt $CurrentPass) {
        Archive-SupersededPatch -Patch $patch -PatchPass $patchPass -CurrentPass $CurrentPass
        continue
    }
    $patchName = [System.IO.Path]::GetFileNameWithoutExtension($patch.Name)
    $stage = Join-Path $Staging ("{0}_{1}" -f $Timestamp, $patchName)
    $backup = Join-Path $Backups ("{0}_{1}" -f $Timestamp, $patchName)
    $transactionEntries = [System.Collections.Generic.List[object]]::new()
    $transactionCommitted = $false
    try {
        LogLine "Processing $($patch.Name)" "STEP"
        if (Test-Path -LiteralPath $stage) { Remove-Item -LiteralPath $stage -Recurse -Force }
        New-Item -ItemType Directory -Force -Path $stage | Out-Null
        Expand-Archive -LiteralPath $patch.FullName -DestinationPath $stage -Force
        $payload = Get-PayloadRoot $stage
        LogLine "Payload root: $payload"

        $manifestInfo = Get-PatchManifestInfo $payload
        if ($manifestInfo.Present) { LogLine "PATCH_MANIFEST.json verified (schema v1)." "PASS" }
        else { LogLine "Legacy unmanifested patch accepted; generated handoffs should use PATCH_MANIFEST.json." "WARN" }

        $files = @(Get-ChildItem -LiteralPath $payload -Recurse -File -Force | Where-Object {
            $_.FullName -notmatch "[\\/]\.git[\\/]" -and $_.Name -ne "PATCH_MANIFEST.json"
        })
        $removePaths = if ($manifestInfo.Present) { @($manifestInfo.RemoveEntries) } else { @() }
        if ($files.Count -eq 0 -and $removePaths.Count -eq 0) { throw "Patch has no overlay or removal operations." }
        LogLine "Files to overlay: $($files.Count); explicit removals: $($removePaths.Count)"

        $seen = @{}
        foreach ($file in $files) {
            $relative = [System.IO.Path]::GetRelativePath($payload, $file.FullName)
            Assert-SafeRelativePath $relative
            $key = $relative.Replace('/', '\').ToLowerInvariant()
            if ($seen.ContainsKey($key)) { throw "Patch contains duplicate destination path: $relative" }
            $seen[$key] = $true
            $dest = Join-Path $Root $relative
            $existed = Test-Path -LiteralPath $dest
            $beforeHash = if ($existed) { Get-FileSha256 $dest } else { $null }
            $payloadHash = Get-FileSha256 $file.FullName
            LogLine "  [OVERLAY] $relative [$payloadHash]"
            if (-not $DryRun) {
                if ($existed) {
                    if ((Get-Item -LiteralPath $dest).PSIsContainer) { throw "Patch overlay destination is a directory, expected file: $relative" }
                    $backupFile = Join-Path $backup $relative
                    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $backupFile) | Out-Null
                    Copy-Item -LiteralPath $dest -Destination $backupFile -Force
                }
                $transactionEntries.Add([pscustomobject]@{ operation='overlay'; relative=$relative; existed=$existed; beforeSha256=$beforeHash; payloadSha256=$payloadHash }) | Out-Null
                New-Item -ItemType Directory -Force -Path (Split-Path -Parent $dest) | Out-Null
                Copy-Item -LiteralPath $file.FullName -Destination $dest -Force
                $afterHash = Get-FileSha256 $dest
                if ($afterHash -ne $payloadHash) { throw "Post-overlay hash mismatch: $relative" }
            }
        }

        foreach ($relative in $removePaths) {
            Assert-SafeRelativePath $relative
            $key = $relative.Replace('/', '\').ToLowerInvariant()
            if ($seen.ContainsKey($key)) { throw "Patch cannot overlay and remove the same destination path: $relative" }
            $seen[$key] = $true
            $dest = Join-Path $Root $relative
            $existed = Test-Path -LiteralPath $dest
            $beforeHash = if ($existed -and -not (Get-Item -LiteralPath $dest).PSIsContainer) { Get-FileSha256 $dest } else { $null }
            LogLine "  [REMOVE] $relative $(if ($existed) { '[present]' } else { '[already absent]' })"
            if (-not $DryRun) {
                if ($existed) {
                    if ((Get-Item -LiteralPath $dest).PSIsContainer) { throw "Explicit patch removal only supports files, not directories: $relative" }
                    $backupFile = Join-Path $backup $relative
                    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $backupFile) | Out-Null
                    Copy-Item -LiteralPath $dest -Destination $backupFile -Force
                }
                $transactionEntries.Add([pscustomobject]@{ operation='remove'; relative=$relative; existed=$existed; beforeSha256=$beforeHash; payloadSha256=$null }) | Out-Null
                if ($existed) { Remove-Item -LiteralPath $dest -Force }
                if (Test-Path -LiteralPath $dest) { throw "Post-remove verification failed: $relative" }
            }
        }

        if (-not $DryRun) {
            $transactionPath = Join-Path $Transactions ("{0}_{1}.json" -f $Timestamp, $patchName)
            [ordered]@{
                schemaVersion = 1
                timestamp = $Timestamp
                patch = $patch.Name
                patchSha256 = (Get-FileSha256 $patch.FullName)
                repositoryPassBefore = $CurrentPass
                manifest = if ($manifestInfo.Present) { 'PATCH_MANIFEST.json' } else { $null }
                files = $transactionEntries.ToArray()
                backupRoot = $backup
                state = 'COMMITTED'
            } | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $transactionPath -Encoding UTF8
            $transactionCommitted = $true
            $appliedZip = Join-Path $Applied ("{0}_{1}" -f $Timestamp, $patch.Name)
            Move-Item -LiteralPath $patch.FullName -Destination $appliedZip -Force
            Remove-Item -LiteralPath $stage -Recurse -Force -ErrorAction SilentlyContinue
            LogLine "Applied and archived: $appliedZip" "PASS"
            AddResult $patch.Name "PASS" "Applied and archived."
            if ($patchPass -gt $CurrentPass) { $CurrentPass = $patchPass }
        }
        else {
            Remove-Item -LiteralPath $stage -Recurse -Force -ErrorAction SilentlyContinue
            LogLine "Dry-run complete for $($patch.Name)" "PASS"
            AddResult $patch.Name "PASS" "Dry-run preview passed."
        }
    }
    catch {
        $failures++
        $message = $_.Exception.Message
        $exceptionType = $_.Exception.GetType().FullName
        $fqid = [string]$_.FullyQualifiedErrorId
        $stack = [string]$_.ScriptStackTrace
        LogLine "Failed $($patch.Name): $message" "FAIL"
        if (-not [string]::IsNullOrWhiteSpace($exceptionType)) { LogLine "  Exception type: $exceptionType" "FAIL" }
        if (-not [string]::IsNullOrWhiteSpace($fqid)) { LogLine "  FullyQualifiedErrorId: $fqid" "FAIL" }
        if (-not [string]::IsNullOrWhiteSpace($stack)) { LogLine "  Script stack: $($stack -replace '[\r\n]+', ' <- ')" "FAIL" }
        AddResult $patch.Name "FAIL" $message
        if (-not $DryRun) {
            if (-not $transactionCommitted -and $transactionEntries.Count -gt 0) {
                try { Restore-OverlayTransaction -Entries $transactionEntries -BackupRoot $backup } catch { LogLine "Rollback failed: $($_.Exception.Message)" "FAIL" }
            }
            try { Move-Item -LiteralPath $patch.FullName -Destination (Join-Path $Failed ("{0}_{1}" -f $Timestamp, $patch.Name)) -Force } catch {}
        }
    }
}

if ($BuildAfter -and -not $DryRun) {
    $tool = Join-Path $Root "SubspaceTools.ps1"
    if (Test-Path -LiteralPath $tool) {
        $args = @("-NoProfile", "-ExecutionPolicy", "Bypass", "-File", $tool, "-Action", "build-render", "-NoAutoApplyUpdates")
        if ($Clean) { $args += "-Clean" }
        LogLine "Running build after inbox apply." "STEP"
        & powershell.exe @args
        if ($LASTEXITCODE -ne 0) { $failures++ }
    }
}

LogLine "------------------------------------------------------------------------"
if ($failures -eq 0) {
    LogLine "RESULT: PASS" "PASS"
    WriteLatest "PASS"
    exit 0
}
else {
    LogLine "RESULT: FAIL ($failures failed patch(es)/step(s))" "FAIL"
    WriteLatest "FAIL"
    exit 1
}

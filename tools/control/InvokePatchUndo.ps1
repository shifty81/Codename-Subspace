param([string]$Root=(Get-Location).Path)
$ErrorActionPreference='Stop'
$Root=[System.IO.Path]::GetFullPath($Root)

$transactions=Join-Path $Root 'updates\transactions'
$records=@(
    Get-ChildItem -LiteralPath $transactions -Filter '*.json' -File -ErrorAction SilentlyContinue |
        Sort-Object LastWriteTimeUtc -Descending
)

if ($records.Count -eq 0) {
    throw 'No committed patch transaction is available to undo.'
}

$recordPath=$records[0].FullName
$tx=Get-Content -LiteralPath $recordPath -Raw | ConvertFrom-Json
if ([string]$tx.state -ne 'COMMITTED') {
    throw "Latest transaction is not COMMITTED: $recordPath"
}

$backup=[string]$tx.backupRoot
$entries=@($tx.files)

# ---------------------------------------------------------------------------
# Drift guard
# ---------------------------------------------------------------------------
# Undo must never silently destroy work created after the patch transaction.
# Every path is checked against the post-patch state recorded by the updater.
foreach ($e in $entries) {
    $relative=[string]$e.relative
    $operation=[string]$e.operation
    $dest=Join-Path $Root $relative

    if ($operation -eq 'overlay') {
        # The patch wrote this path. If it no longer exists, or its contents no
        # longer match the payload hash, something changed after application.
        if (-not (Test-Path -LiteralPath $dest -PathType Leaf)) {
            throw "Drift guard refused undo; path written by the patch has since been removed: $relative"
        }

        if ($e.payloadSha256) {
            $current=(Get-FileHash -Algorithm SHA256 -LiteralPath $dest -ErrorAction Stop).Hash.ToLowerInvariant()
            $expected=([string]$e.payloadSha256).ToLowerInvariant()
            if ($current -ne $expected) {
                throw "Drift guard refused undo; path written by the patch has since been modified: $relative"
            }
        }
    }
    elseif ($operation -eq 'remove') {
        # Pass735-744 contract: if a path removed by the patch has since been
        # recreated, undo must fail closed instead of overwriting newer work.
        if (Test-Path -LiteralPath $dest) {
            throw "Drift guard refused undo; path removed by the patch has since been recreated: $relative"
        }
    }
    else {
        throw "Drift guard refused undo; unknown transaction operation '$operation' for: $relative"
    }
}

# ---------------------------------------------------------------------------
# Transactional restore
# ---------------------------------------------------------------------------
for ($i=$entries.Count-1; $i -ge 0; $i--) {
    $e=$entries[$i]
    $relative=[string]$e.relative
    $operation=[string]$e.operation
    $dest=Join-Path $Root $relative
    $backupFile=Join-Path $backup $relative

    if ($operation -eq 'overlay') {
        if ([bool]$e.existed) {
            if (-not (Test-Path -LiteralPath $backupFile -PathType Leaf)) {
                throw "Backup missing for patched path: $backupFile"
            }
            New-Item -ItemType Directory -Force -Path (Split-Path -Parent $dest) | Out-Null
            Copy-Item -LiteralPath $backupFile -Destination $dest -Force
        }
        else {
            # The patch introduced the file; remove the still-certified payload.
            if (Test-Path -LiteralPath $dest) {
                Remove-Item -LiteralPath $dest -Force
            }
        }
    }
    elseif ($operation -eq 'remove') {
        if ([bool]$e.existed) {
            if (-not (Test-Path -LiteralPath $backupFile -PathType Leaf)) {
                throw "Backup missing for removed path: $backupFile"
            }
            New-Item -ItemType Directory -Force -Path (Split-Path -Parent $dest) | Out-Null
            Copy-Item -LiteralPath $backupFile -Destination $dest -Force
        }
    }
}

$tx.state='UNDONE'
$tx.undoneAt=(Get-Date).ToString('o')
$tx | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $recordPath -Encoding UTF8

$latest=Join-Path $Root 'updates\LATEST_PATCH_UNDO.txt'
@(
    "Patch undo PASS",
    "Transaction: $recordPath",
    "Patch: $($tx.patch)",
    "Timestamp: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')"
) | Set-Content -LiteralPath $latest -Encoding UTF8

Write-Host '[PASS] Latest committed patch transaction was restored.' -ForegroundColor Green
Write-Host $latest

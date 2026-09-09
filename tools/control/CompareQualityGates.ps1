param([string]$Root=(Get-Location).Path)
$ErrorActionPreference='Stop'
$history=Join-Path ([System.IO.Path]::GetFullPath($Root)) 'artifacts\gates\quality'
$records=@(Get-ChildItem -LiteralPath $history -Filter 'QG-*.json' -File -ErrorAction SilentlyContinue |
    Sort-Object LastWriteTimeUtc -Descending | Select-Object -First 8)
if ($records.Count -eq 0) { Write-Host 'No quality-gate history found.'; return }
Write-Host 'Recent quality gates'
Write-Host '------------------------------------------------------------------------'
foreach ($file in $records) {
    try {
        $g=Get-Content -LiteralPath $file.FullName -Raw | ConvertFrom-Json
        Write-Host ("{0,-30} {1,-5} {2,-4} {3,-12} {4}" -f $g.gateId,$g.kind,$g.result,$g.configuration,$g.pass)
    } catch { Write-Host ("Unreadable: " + $file.FullName) -ForegroundColor Yellow }
}

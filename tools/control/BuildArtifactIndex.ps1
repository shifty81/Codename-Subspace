param([string]$Root=(Get-Location).Path)
$ErrorActionPreference='Stop'
$Root=[System.IO.Path]::GetFullPath($Root)
$outDir=Join-Path $Root 'artifacts'
New-Item -ItemType Directory -Force -Path $outDir | Out-Null
$rows=[System.Collections.Generic.List[object]]::new()
foreach ($file in @(Get-ChildItem -LiteralPath $outDir -Recurse -File -ErrorAction SilentlyContinue)) {
    if ($file.Name -eq 'ARTIFACT_INDEX.json' -or $file.Name -eq 'ARTIFACT_INDEX.txt') { continue }
    $rel=$file.FullName.Substring($Root.TrimEnd('\').Length).TrimStart('\')
    $rows.Add([pscustomobject]@{path=$rel;bytes=[int64]$file.Length;modified=$file.LastWriteTime.ToString('o')}) | Out-Null
}
$json=Join-Path $outDir 'ARTIFACT_INDEX.json'
$txt=Join-Path $outDir 'ARTIFACT_INDEX.txt'
[pscustomobject]@{schemaVersion=1;timestamp=(Get-Date).ToString('o');artifacts=$rows.ToArray()} |
    ConvertTo-Json -Depth 6 | Set-Content -LiteralPath $json -Encoding UTF8
@("CODENAME SUBSPACE ARTIFACT INDEX","Generated: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss')","") +
    @($rows | Sort-Object path | ForEach-Object { "{0,12}  {1}" -f $_.bytes,$_.path }) |
    Set-Content -LiteralPath $txt -Encoding UTF8
Write-Host "ARTIFACT INDEX: $txt"

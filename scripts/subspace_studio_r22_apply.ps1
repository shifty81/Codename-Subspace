$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$py = Get-Command python -ErrorAction SilentlyContinue
if (-not $py) { throw 'Python is required for the R13-R22 Studio source migration.' }
& python (Join-Path $root 'tools\studio\apply_studio_r13_r22_normalization.py') --root $root --apply
if ($LASTEXITCODE -ne 0) { throw 'R13-R22 Studio source migration FAILED/BLOCKED.' }
Write-Host 'R13-R22 Studio source migration complete. Run scripts\subspace_studio_r22_gate.ps1, then PCC Full Gate.'

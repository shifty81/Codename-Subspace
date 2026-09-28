param([string]$Root=(Resolve-Path (Join-Path $PSScriptRoot '..')).Path)
$ErrorActionPreference='Stop'
$py=(Get-Command python -ErrorAction SilentlyContinue)
if(-not $py){$py=(Get-Command py -ErrorAction SilentlyContinue)}
if(-not $py){throw 'Python required for R82R4 source policy'}
$argsPy=@();if($py.Name -eq 'py.exe' -or $py.Name -eq 'py'){$argsPy+=('-3')}
& $py.Source @argsPy (Join-Path $Root 'tools\control	ests	est_r82r4_four_failure_policy.py')
if($LASTEXITCODE -ne 0){throw 'R82R4 policy check failed'}
& $py.Source @argsPy (Join-Path $Root 'tools\studiopply_studio_r82r2_gate_repair.py') --root $Root --apply
if($LASTEXITCODE -ne 0){throw 'R82R2 snapped-commit normalization failed'}
& cmake -P (Join-Path $Root 'tools\control\static-gates\pass1567_1568_studio_r82r4_four_failure_recovery.cmake')
if($LASTEXITCODE -ne 0){throw 'R82R4 source gate failed'}
Write-Host '[PASS] R82R4 focused prebuild contract' -ForegroundColor Green

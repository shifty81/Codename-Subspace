param([string]$Root=(Resolve-Path (Join-Path $PSScriptRoot '..')).Path)
$ErrorActionPreference='Stop'
$python=(Get-Command python -ErrorAction SilentlyContinue)
if(-not $python){$python=(Get-Command py -ErrorAction SilentlyContinue)}
if(-not $python){throw 'Python is required for the guarded R82R2 migration.'}
$exe=$python.Source
$arguments=@()
if($python.Name -eq 'py.exe' -or $python.Name -eq 'py'){$arguments+=('-3')}
$migration=Join-Path $Root 'tools\studio\apply_studio_r82r2_gate_repair.py'
$arguments+=@($migration,'--root',$Root,'--apply')
& $exe @arguments
if($LASTEXITCODE -ne 0){throw "R82R2 migration failed with exit code $LASTEXITCODE"}
& cmake -P (Join-Path $Root 'tools\control\static-gates\pass1565_1566_studio_r82r2_gate_repair.cmake')
if($LASTEXITCODE -ne 0){throw "R82R2 static gate failed with exit code $LASTEXITCODE"}
Write-Host '[PASS] R82R2 snapped commit + nonmodal axis diagnostic gate' -ForegroundColor Green

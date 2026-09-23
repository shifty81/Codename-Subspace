$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$test = Join-Path $root 'tools\control\tests\test_native_link_retry_policy.py'
if (-not (Test-Path -LiteralPath $test -PathType Leaf)) { throw 'Missing R22R1 native-link retry policy test.' }
$py = Get-Command python -ErrorAction SilentlyContinue
if (-not $py) { throw 'Python is required for the focused R22R1 gate.' }
& python $test -v
if ($LASTEXITCODE -ne 0) { throw 'R22R1 native-link retry policy test FAILED.' }
Write-Host 'R22R1 focused PCC linker-race policy checks PASS. Run the full PCC quality gate next.'

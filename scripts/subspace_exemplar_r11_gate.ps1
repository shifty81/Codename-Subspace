# R11 additive focused gate. PowerShell 5.1 compatible; not the PCC Full Gate.
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$tests = Join-Path $root 'tools\shipyard\tests'
if (-not (Test-Path $tests)) { throw 'Missing R11 tests or R10 prerequisite' }
$py = Get-Command python -ErrorAction SilentlyContinue
if (-not $py) { throw 'Python is required for the optional exemplar authoring tools' }
& python -m unittest discover -s $tests -p 'test_*intake.py' -v
if ($LASTEXITCODE -ne 0) { throw 'Exemplar intake tests FAILED' }
& python -m unittest discover -s $tests -p 'test_exemplar_graph_compiler.py' -v
if ($LASTEXITCODE -ne 0) { throw 'Exemplar graph compiler tests FAILED' }
Write-Host 'R11 focused exemplar checks PASS. This does not certify Windows runtime or promote PCG.'

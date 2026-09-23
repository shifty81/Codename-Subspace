# PS 5.1-compatible isolated Exemplar Intake test gate. No source mutation.
$ErrorActionPreference = 'Stop'
$root = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$python = Get-Command python -ErrorAction SilentlyContinue
if ($null -eq $python) {
    Write-Host '[FAIL] Python 3 not found. Exemplar intake tests not run.'
    exit 2
}
$tests = Join-Path $root 'tools\shipyard\tests'
& $python.Source -m unittest discover -s $tests -p 'test_*.py' -v
$code = $LASTEXITCODE
if ($code -ne 0) {
    Write-Host ('[FAIL] Exemplar Intake tests exit code: ' + $code)
    exit $code
}
Write-Host '[PASS] Exemplar Intake and donor inventory tests passed. Windows PCC Full Gate is still required for project certification.'
exit 0

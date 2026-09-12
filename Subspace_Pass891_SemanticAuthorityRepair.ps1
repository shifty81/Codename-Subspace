[CmdletBinding()]
param(
    [string]$Root = (Get-Location).Path,
    [switch]$NoFullGate
)

$ErrorActionPreference = "Stop"
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$repair = Join-Path $scriptDir "subspace_pass891_semantic_authority_repair.py"
if (-not (Test-Path -LiteralPath $repair)) {
    throw "Repair engine not found beside wrapper: $repair"
}

$python = Get-Command py -ErrorAction SilentlyContinue
if ($python) {
    $args = @("-3", $repair, "--root", $Root)
    if ($NoFullGate) { $args += "--no-full-gate" }
    & $python.Source @args
    exit $LASTEXITCODE
}

$python = Get-Command python -ErrorAction SilentlyContinue
if (-not $python) {
    throw "Python 3 was not found in PATH."
}

$args = @($repair, "--root", $Root)
if ($NoFullGate) { $args += "--no-full-gate" }
& $python.Source @args
exit $LASTEXITCODE

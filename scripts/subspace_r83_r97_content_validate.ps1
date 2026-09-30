param([string]$Root = (Split-Path -Parent $PSScriptRoot))
$ErrorActionPreference = "Stop"
$validator = Join-Path $Root "tools\content\validate_embodied_exploration_content.py"
if (-not (Test-Path -LiteralPath $validator)) { throw "Missing validator: $validator" }
$python = Get-Command python -ErrorAction SilentlyContinue
if (-not $python) { throw "python is required for content validation" }
& $python.Source $validator
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
Write-Host "[PASS] R83-R112 content-first tranche validates. No native build was run."

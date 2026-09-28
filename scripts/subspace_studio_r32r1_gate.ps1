$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$python = Get-Command python -ErrorAction SilentlyContinue
if (-not $python) { throw 'Python is required for the R32R1 focused gate.' }
$cmake = Get-Command cmake -ErrorAction SilentlyContinue
if (-not $cmake) { throw 'CMake is required for the R32R1 focused gate.' }

& python (Join-Path $root 'tools\control\tests\test_r32_prebuild_migration_policy.py') -v
if ($LASTEXITCODE -ne 0) { throw 'R32R1 PCC migration policy tests FAILED.' }

# Apply the already-installed R32 source transaction now when the focused gate
# is run manually. Full Gate has the same guarded path, so this is optional.
$r32 = Join-Path $root 'scripts\subspace_studio_r32_gate.ps1'
if (-not (Test-Path -LiteralPath $r32 -PathType Leaf)) { throw 'R32 focused gate is missing.' }
& $r32
if ($LASTEXITCODE -ne 0) { throw 'R32 source cutover/focused certification FAILED.' }

& cmake -P (Join-Path $root 'tools\control\static-gates\pass1510_r32_prebuild_migration_authority.cmake')
if ($LASTEXITCODE -ne 0) { throw 'R32R1 static orchestration gate FAILED.' }
Write-Host 'R32R1 focused gate PASS. Full Quality Gate can now run without a separate migration step.'

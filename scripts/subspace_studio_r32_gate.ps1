$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$python = Get-Command python -ErrorAction SilentlyContinue
if (-not $python) { throw 'Python is required for the R32 focused gate.' }
$cmake = Get-Command cmake -ErrorAction SilentlyContinue
if (-not $cmake) { throw 'CMake is required for the R32 focused gate.' }

Write-Host '[R32] mandatory transactional source cutover'
& python (Join-Path $root 'tools\studio\apply_studio_r23_r32_normalization.py') --root $root --apply
if ($LASTEXITCODE -ne 0) { throw 'R23-R32 source migration FAILED.' }
& python (Join-Path $root 'tools\studio\studio_r32_source_assertions.py') --root $root
if ($LASTEXITCODE -ne 0) { throw 'R32 source assertions FAILED.' }

Write-Host '[R32] static ProjectOps certification contract'
& cmake -P (Join-Path $root 'tools\control\static-gates\pass1509_studio_construct_cutover.cmake')
if ($LASTEXITCODE -ne 0) { throw 'R32 ProjectOps static gate FAILED.' }

Write-Host '[R32] exemplar / learning tests'
& python -m unittest discover -s (Join-Path $root 'tools\shipyard\tests') -p 'test_*.py' -v
if ($LASTEXITCODE -ne 0) { throw 'Shipyard exemplar tests FAILED.' }

Write-Host '[R32] migration tests'
& python (Join-Path $root 'tools\studio\test_apply_studio_r23_r32_normalization.py')
if ($LASTEXITCODE -ne 0) { throw 'R32 migration tests FAILED.' }

Write-Host '[R32] isolated C++ UI/layout contract'
$build = Join-Path $root 'Builds\studio-r32-contract'
cmake -S (Join-Path $root 'tools\control\studio-r32-contract') -B $build
if ($LASTEXITCODE -ne 0) { throw 'R32 CMake configure FAILED.' }
cmake --build $build --config Release
if ($LASTEXITCODE -ne 0) { throw 'R32 C++ contract build FAILED.' }
$exe = Get-ChildItem -Path $build -Recurse -Filter 'subspace_studio_r32_contract.exe' -ErrorAction SilentlyContinue | Select-Object -First 1
if (-not $exe) { throw 'R32 contract executable was not produced.' }
& $exe.FullName
if ($LASTEXITCODE -ne 0) { throw 'R32 C++ contract assertions FAILED.' }

Write-Host 'R32 focused gate PASS. Run PCC FULL QUALITY GATE next for Windows application certification.'

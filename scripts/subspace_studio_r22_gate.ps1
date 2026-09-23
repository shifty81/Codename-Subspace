$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$python = Get-Command python -ErrorAction SilentlyContinue
if (-not $python) { throw 'Python is required for the R22 focused gate.' }
$cmake = Get-Command cmake -ErrorAction SilentlyContinue
if (-not $cmake) { throw 'CMake is required for the R22 focused C++ contract gate.' }

Write-Host '[R22] source migration state'
& python (Join-Path $root 'tools\studio\apply_studio_r13_r22_normalization.py') --root $root --check
if ($LASTEXITCODE -ne 0) { throw 'R22 migration preflight FAILED.' }
& python (Join-Path $root 'tools\studio\studio_r22_source_assertions.py') --root $root
if ($LASTEXITCODE -ne 0) { throw 'R22 source assertions FAILED. Run scripts\subspace_studio_r22_apply.ps1 first.' }

Write-Host '[R22] Python authoring/tool tests'
& python -m unittest discover -s (Join-Path $root 'tools\shipyard\tests') -p 'test_*.py' -v
if ($LASTEXITCODE -ne 0) { throw 'Exemplar/shipyard Python tests FAILED.' }
& python -m unittest discover -s (Join-Path $root 'tools\blender\tests') -p 'test_*.py' -v
if ($LASTEXITCODE -ne 0) { throw 'Kitbash Foundry tests FAILED.' }
& python (Join-Path $root 'tools\studio\test_apply_studio_r13_r22_normalization.py')
if ($LASTEXITCODE -ne 0) { throw 'R22 migration utility tests FAILED.' }

Write-Host '[R22] isolated C++ transform contract'
$build = Join-Path $root 'Builds\studio-r22-contract'
cmake -S (Join-Path $root 'tools\control\studio-r22-contract') -B $build
if ($LASTEXITCODE -ne 0) { throw 'R22 CMake configure FAILED.' }
cmake --build $build --config Release
if ($LASTEXITCODE -ne 0) { throw 'R22 C++ contract build FAILED.' }
$exe = Get-ChildItem -Path $build -Recurse -Filter 'subspace_studio_r22_contract.exe' -ErrorAction SilentlyContinue | Select-Object -First 1
if (-not $exe) { throw 'R22 contract executable was not produced.' }
& $exe.FullName
if ($LASTEXITCODE -ne 0) { throw 'R22 C++ contract assertions FAILED.' }

Write-Host 'R22 focused gate PASS. This is not the Windows full application build or visual acceptance.'

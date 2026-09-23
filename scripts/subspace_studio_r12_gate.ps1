# PowerShell 5.1 compatible. Focused checks only; PCC Full Gate is separate.
$ErrorActionPreference = 'Stop'
$root = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$py = Get-Command python -ErrorAction SilentlyContinue
if (-not $py) { throw 'Python 3 is required for the R10/R11 optional tools' }
$tests = Join-Path $root 'tools\shipyard\tests'
if (-not (Test-Path (Join-Path $root 'tools\shipyard\exemplar_intake.py'))) { throw 'R10 intake dependency absent' }
& $py.Source -m unittest discover -s $tests -p 'test_*.py' -v
if ($LASTEXITCODE -ne 0) { throw 'R10/R11 exemplar tests FAILED' }
$cmake = Get-Command cmake -ErrorAction SilentlyContinue
if (-not $cmake) { throw 'CMake is required for isolated symmetry check' }
$source = Join-Path $root 'tools\control\studio-axis-contract'
$build = Join-Path $root 'engine\build\studio-axis-r12'
& $cmake.Source -S $source -B $build
if ($LASTEXITCODE -ne 0) { throw 'R12 CMake configure FAILED' }
& $cmake.Source --build $build --config Debug
if ($LASTEXITCODE -ne 0) { throw 'R12 C++ build FAILED' }
& $cmake.Source -E chdir $build ctest --output-on-failure -C Debug
if ($LASTEXITCODE -ne 0) { throw 'R12 CTest FAILED' }
Write-Host '[PASS] R12 focused checks. Run project PCC Full Quality Gate and Windows GUI acceptance separately.'

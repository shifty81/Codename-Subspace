$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
$python=Get-Command python -ErrorAction SilentlyContinue
$cmake=Get-Command cmake -ErrorAction SilentlyContinue
if(-not $python){throw 'Python is required for R72 focused gate.'}
if(-not $cmake){throw 'CMake is required for R72 focused gate.'}
Write-Host '[R72] prerequisite migrations'
& python (Join-Path $root 'tools\studio\apply_studio_r33_r42_bulk_polish.py') --root $root --apply
if($LASTEXITCODE -ne 0){throw 'R33-R42 migration failed.'}
& python (Join-Path $root 'tools\studio\apply_studio_r43_r52_transform_authority.py') --root $root --apply
if($LASTEXITCODE -ne 0){throw 'R43-R52 migration failed.'}
& python (Join-Path $root 'tools\studio\apply_studio_r53_r62_transform_ui_hygiene.py') --root $root --apply
if($LASTEXITCODE -ne 0){throw 'R53-R62 migration failed.'}
& python (Join-Path $root 'tools\studio\apply_studio_r63_r72_selection_placement.py') --root $root --apply
if($LASTEXITCODE -ne 0){throw 'R63-R72 migration failed.'}
Write-Host '[R72] static source gate'
& cmake -P (Join-Path $root 'tools\control\static-gates\pass1541_1550_studio_selection_placement.cmake')
if($LASTEXITCODE -ne 0){throw 'R63-R72 static source gate failed.'}
Write-Host '[R72] PCC prebuild policy'
& python (Join-Path $root 'tools\control\tests\test_r63_r72_prebuild_policy.py')
if($LASTEXITCODE -ne 0){throw 'R63-R72 PCC policy test failed.'}
Write-Host '[R72] configure/build focused tests'
$build=Join-Path $root 'Builds\studio-r72-contract'
& cmake -S (Join-Path $root 'engine') -B $build -DSUBSPACE_BUILD_TESTS=ON
if($LASTEXITCODE -ne 0){throw 'R72 CMake configure failed.'}
& cmake --build $build --config Release --target subspace_studio_placement_workflow_r63_tests subspace_studio_duplicate_staged_r64_tests subspace_studio_safe_delete_r67_tests
if($LASTEXITCODE -ne 0){throw 'R72 focused build failed.'}
& ctest --test-dir $build -C Release -R 'SubspaceStudio(PlacementWorkflowR63|DuplicateStagedR64|SafeDeleteR67|SelectionPlacementR72)' --output-on-failure
if($LASTEXITCODE -ne 0){throw 'R72 focused CTest failed.'}
Write-Host 'R72 focused selection/placement ergonomics gate PASS. Run PCC FULL QUALITY GATE next.'

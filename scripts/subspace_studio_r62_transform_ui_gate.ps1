$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
$python=Get-Command python -ErrorAction SilentlyContinue
$cmake=Get-Command cmake -ErrorAction SilentlyContinue
if(-not $python){throw 'Python is required for R62 focused gate.'}
if(-not $cmake){throw 'CMake is required for R62 focused gate.'}
Write-Host '[R62] prerequisite migrations'
& python (Join-Path $root 'tools\studio\apply_studio_r33_r42_bulk_polish.py') --root $root --apply
if($LASTEXITCODE -ne 0){throw 'R33-R42 migration failed.'}
& python (Join-Path $root 'tools\studio\apply_studio_r43_r52_transform_authority.py') --root $root --apply
if($LASTEXITCODE -ne 0){throw 'R43-R52 migration failed.'}
& python (Join-Path $root 'tools\studio\apply_studio_r53_r62_transform_ui_hygiene.py') --root $root --apply
if($LASTEXITCODE -ne 0){throw 'R53-R62 migration failed.'}
Write-Host '[R62] static source gate'
& cmake -P (Join-Path $root 'tools\control\static-gates\pass1531_1540_studio_transform_ui_hygiene.cmake')
if($LASTEXITCODE -ne 0){throw 'R53-R62 static source gate failed.'}
Write-Host '[R62] PCC prebuild policy'
& python (Join-Path $root 'tools\control\tests\test_r53_r62_prebuild_policy.py')
if($LASTEXITCODE -ne 0){throw 'R53-R62 PCC policy test failed.'}
Write-Host '[R62] configure/build focused tests'
$build=Join-Path $root 'Builds\studio-r62-contract'
& cmake -S (Join-Path $root 'engine') -B $build -DSUBSPACE_BUILD_TESTS=ON
if($LASTEXITCODE -ne 0){throw 'R62 CMake configure failed.'}
& cmake --build $build --config Release --target subspace_studio_transform_status_r53_tests subspace_studio_transform_constraint_hygiene_r55_tests subspace_studio_overlay_placement_r33_tests
if($LASTEXITCODE -ne 0){throw 'R62 focused build failed.'}
& ctest --test-dir $build -C Release -R 'SubspaceStudio(TransformStatusR53|TransformConstraintHygieneR55|OverlayPlacementR33|TransformUiHygieneR62)' --output-on-failure
if($LASTEXITCODE -ne 0){throw 'R62 focused CTest failed.'}
Write-Host 'R62 focused transform UI/state hygiene gate PASS. Run PCC FULL QUALITY GATE next.'

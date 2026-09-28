$ErrorActionPreference='Stop'
$root=Split-Path -Parent $PSScriptRoot
$python=Get-Command python -ErrorAction SilentlyContinue
$cmake=Get-Command cmake -ErrorAction SilentlyContinue
if(-not $python){throw 'Python is required for R82 focused gate.'}
if(-not $cmake){throw 'CMake is required for R82 focused gate.'}
Write-Host '[R82] prerequisite migrations'
foreach($m in @(
 'apply_studio_r33_r42_bulk_polish.py',
 'apply_studio_r43_r52_transform_authority.py',
 'apply_studio_r53_r62_transform_ui_hygiene.py',
 'apply_studio_r63_r72_selection_placement.py',
 'apply_studio_r73_r82_convergence_audit.py')){
    & python (Join-Path $root ('tools\studio\'+$m)) --root $root --apply
    if($LASTEXITCODE -ne 0){throw "Studio migration failed: $m"}
}
Write-Host '[R82] static source gate'
& cmake -P (Join-Path $root 'tools\control\static-gates\pass1551_1560_studio_convergence_audit.cmake')
if($LASTEXITCODE -ne 0){throw 'R73-R82 static source gate failed.'}
Write-Host '[R82] PCC prebuild policy'
& python (Join-Path $root 'tools\control\tests\test_r73_r82_prebuild_policy.py')
if($LASTEXITCODE -ne 0){throw 'R73-R82 PCC policy test failed.'}
Write-Host '[R82] configure/build focused test'
$build=Join-Path $root 'Builds\studio-r82-contract'
& cmake -S (Join-Path $root 'engine') -B $build -DSUBSPACE_BUILD_TESTS=ON
if($LASTEXITCODE -ne 0){throw 'R82 CMake configure failed.'}
& cmake --build $build --config Release --target subspace_studio_duplicate_snap_r73_tests subspace_studio_context_selection_r78_tests
if($LASTEXITCODE -ne 0){throw 'R82 focused build failed.'}
& ctest --test-dir $build -C Release -R 'SubspaceStudio(DuplicateSnapR73|ContextSelectionR78|ConvergenceR82)' --output-on-failure
if($LASTEXITCODE -ne 0){throw 'R82 focused CTest failed.'}
Write-Host 'R82 focused convergence gate PASS. Run PCC FULL QUALITY GATE next.'

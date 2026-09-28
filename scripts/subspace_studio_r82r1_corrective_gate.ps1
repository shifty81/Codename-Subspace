param([string]$Root=(Resolve-Path (Join-Path $PSScriptRoot '..')).Path)
$ErrorActionPreference='Stop'
Write-Host 'Subspace Studio R82R1 corrective gate' -ForegroundColor Cyan
& python (Join-Path $Root 'tools\studio\apply_studio_r82r1_corrective_audit.py') --root $Root --apply
if($LASTEXITCODE -ne 0){exit $LASTEXITCODE}
& python (Join-Path $Root 'tools\control\tests\test_r82r1_prebuild_policy.py')
if($LASTEXITCODE -ne 0){exit $LASTEXITCODE}
$build=Join-Path $Root 'engine\build-r82r1-focused'
& cmake -S (Join-Path $Root 'engine') -B $build -DSUBSPACE_HEADLESS=ON -DSUBSPACE_BUILD_OPENGL=OFF -DSUBSPACE_BUILD_TESTS=ON
if($LASTEXITCODE -ne 0){exit $LASTEXITCODE}
& cmake --build $build --target subspace_studio_transform_space_policy_r43_tests subspace_studio_transform_status_r53_tests subspace_studio_local_move_semantics_r47_tests subspace_studio_mirrored_local_move_r82r1_tests subspace_studio_duplicate_snap_r73_tests
if($LASTEXITCODE -ne 0){exit $LASTEXITCODE}
& ctest --test-dir $build -R 'SubspaceStudio(TransformSpacePolicyR43|TransformStatusR53|LocalMoveSemanticsR47|MirroredLocalMoveR82R1|DuplicateSnapR73)|SubspaceStudioR82R1CorrectiveSourceGate' --output-on-failure
exit $LASTEXITCODE

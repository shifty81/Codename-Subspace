param(
    [string]$Root = (Split-Path -Parent $PSScriptRoot),
    [string]$Configuration = 'Debug'
)
$ErrorActionPreference='Stop'
$Root=(Resolve-Path -LiteralPath $Root).Path
Write-Host '========================================================================'
Write-Host ' CODENAME SUBSPACE STUDIO R43-R52 TRANSFORM AUTHORITY CERTIFICATION'
Write-Host '========================================================================'
Write-Host "Root: $Root"

$python=(Get-Command python -ErrorAction SilentlyContinue)
if(-not $python){throw 'Python is required for the R43-R52 focused gate.'}
$cmake=(Get-Command cmake -ErrorAction SilentlyContinue)
if(-not $cmake){throw 'CMake is required for the R43-R52 focused gate.'}

Write-Host '[R43-R52] ensure prior bulk polish source contract'
& $python.Source (Join-Path $Root 'tools\studio\apply_studio_r33_r42_bulk_polish.py') --root $Root --apply
if($LASTEXITCODE -ne 0){throw 'R33-R42 prerequisite normalization FAILED.'}

Write-Host '[R43-R52] apply guarded transform-space authority'
& $python.Source (Join-Path $Root 'tools\studio\apply_studio_r43_r52_transform_authority.py') --root $Root --apply
if($LASTEXITCODE -ne 0){throw 'R43-R52 guarded source normalization FAILED.'}

Write-Host '[R43-R52] PCC ordering certification'
& $python.Source (Join-Path $Root 'tools\control\tests\test_r43_r52_prebuild_policy.py')
if($LASTEXITCODE -ne 0){throw 'R43-R52 PCC prebuild ordering FAILED.'}

Write-Host '[R43-R52] static source authority'
& $cmake.Source -P (Join-Path $Root 'tools\control\static-gates\pass1521_1530_studio_transform_authority.cmake')
if($LASTEXITCODE -ne 0){throw 'R43-R52 static source authority FAILED.'}

$build=Join-Path $Root 'engine\build'
Write-Host '[R43-R52] configure current native test graph'
& $cmake.Source -S (Join-Path $Root 'engine') -B $build '-DSUBSPACE_HEADLESS=OFF' '-DSUBSPACE_BUILD_OPENGL=ON' '-DSUBSPACE_BUILD_TESTS=ON'
if($LASTEXITCODE -ne 0){throw 'R43-R52 CMake configure FAILED.'}

Write-Host '[R43-R52] compile focused contracts'
& $cmake.Source --build $build --config $Configuration --parallel 8 --target `
    subspace_studio_transform_space_policy_r43_tests `
    subspace_studio_parent_object_basis_r45_tests `
    subspace_studio_local_move_semantics_r47_tests `
    subspace_studio_view_basis_r48_tests `
    subspace_studio_view_move_delta_r48b_tests
if($LASTEXITCODE -ne 0){throw 'R43-R52 focused C++ build FAILED.'}

Write-Host '[R43-R52] run focused CTest contracts'
& ctest --test-dir $build -C $Configuration --output-on-failure -R 'SubspaceStudio(TransformSpacePolicyR43|ParentObjectBasisR45|LocalMoveSemanticsR47|ViewBasisR48|ViewMoveDeltaR48B|TransformAuthorityR52SourceGate)'
if($LASTEXITCODE -ne 0){throw 'R43-R52 focused CTest certification FAILED.'}

Write-Host 'R43-R52 focused certification PASS. Run PCC FULL QUALITY GATE for authoritative Windows/game/Studio smoke.' -ForegroundColor Green

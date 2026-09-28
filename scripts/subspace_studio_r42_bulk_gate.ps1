param(
    [string]$Root = (Split-Path -Parent $PSScriptRoot),
    [string]$Configuration = 'Debug'
)
$ErrorActionPreference='Stop'
$Root=(Resolve-Path -LiteralPath $Root).Path
Write-Host '========================================================================'
Write-Host ' CODENAME SUBSPACE STUDIO R33-R42 FOCUSED CERTIFICATION'
Write-Host '========================================================================'
Write-Host "Root: $Root"

$python=(Get-Command python -ErrorAction SilentlyContinue)
if(-not $python){throw 'Python is required for the R33-R42 focused gate.'}
$cmake=(Get-Command cmake -ErrorAction SilentlyContinue)
if(-not $cmake){throw 'CMake is required for the R33-R42 focused gate.'}

$workspace=Join-Path $Root 'engine\src\ship_editor\ShipyardWorkspaceSystem.cpp'
if(-not (Test-Path -LiteralPath $workspace -PathType Leaf)){throw "Missing workspace source: $workspace"}
$workspaceText=Get-Content -LiteralPath $workspace -Raw
if($workspaceText -notmatch 'case ShipyardWorkspaceMode::Build:return"CONSTRUCT";' -or
   $workspaceText -notmatch 'case ShipyardWorkspaceMode::Model:return"GEOMETRY";'){
    throw 'R32 Construct source cutover is not present. Run the project Full Gate so the approved R32 migration executes first.'
}

Write-Host '[R33-R42] guarded source normalization'
& $python.Source (Join-Path $Root 'tools\studio\apply_studio_r33_r42_bulk_polish.py') --root $Root --apply
if($LASTEXITCODE -ne 0){throw 'R33-R42 guarded source normalization FAILED.'}

Write-Host '[R33-R42] static source authority'
& $cmake.Source -P (Join-Path $Root 'tools\control\static-gates\pass1511_1520_studio_bulk_polish.cmake')
if($LASTEXITCODE -ne 0){throw 'R33-R42 static source authority FAILED.'}

$build=Join-Path $Root 'engine\build'
Write-Host '[R33-R42] configure current native test graph'
& $cmake.Source -S (Join-Path $Root 'engine') -B $build '-DSUBSPACE_HEADLESS=OFF' '-DSUBSPACE_BUILD_OPENGL=ON' '-DSUBSPACE_BUILD_TESTS=ON'
if($LASTEXITCODE -ne 0){throw 'R33-R42 CMake configure FAILED.'}

Write-Host '[R33-R42] compile focused contracts'
& $cmake.Source --build $build --config $Configuration --parallel 8 --target `
    subspace_studio_overlay_placement_r33_tests `
    subspace_studio_axis_contract_r34_tests `
    subspace_studio_layout_stress_r35_tests
if($LASTEXITCODE -ne 0){throw 'R33-R42 focused C++ build FAILED.'}

Write-Host '[R33-R42] run focused CTest contracts'
& ctest --test-dir $build -C $Configuration --output-on-failure -R 'SubspaceStudio(OverlayPlacementR33|AxisContractR34|LayoutStressR35|BulkPolishR42SourceGate)'
if($LASTEXITCODE -ne 0){throw 'R33-R42 focused CTest certification FAILED.'}

Write-Host 'R33-R42 focused certification PASS. Run PCC FULL QUALITY GATE for authoritative Windows/game/Studio smoke.' -ForegroundColor Green

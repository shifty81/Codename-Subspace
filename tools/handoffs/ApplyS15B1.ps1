# S15B1 governed two-stage handoff.
# Run this AFTER the project PCC has consumed the root-drop ZIP-container .patch.
# This script uses git apply, rather than overwriting any newer files from an old source rollup.
param([switch]$CheckOnly)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$root=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$diff=Join-Path $PSScriptRoot 'S15B1_Studio_Orbit.diff'
$baseline='0de248a10a7834ae8b00eb5b8d6c6b5f99e6688a'
if(-not(Test-Path -LiteralPath $diff -PathType Leaf)){throw 'S15B1 diff payload is missing.'}
Push-Location $root
try {
    $head=(& git rev-parse HEAD 2>$null | Select-Object -First 1)
    if($LASTEXITCODE -ne 0 -or ([string]$head).Trim() -ne $baseline){
        throw "S15B1 expects Git HEAD $baseline. Actual: $head. No files changed."
    }
    # A reverse preflight is expected to fail on a fresh install. Windows
    # PowerShell 5.1 must not treat the expected native stderr as a terminating
    # script error before we can inspect LASTEXITCODE.
    $savedPreference=$ErrorActionPreference
    $ErrorActionPreference='Continue'
    try { & git apply --reverse --check -- $diff 2>$null; $reverseExit=$LASTEXITCODE }
    finally { $ErrorActionPreference=$savedPreference }
    if($reverseExit -eq 0){Write-Host '[PASS] S15B1 camera diff already applied. No changes made.' -ForegroundColor Green;return}
    # Do not edit around a locally modified source file, even when context applies.
    $files=@(
        'engine/src/platform/NativeWindow.cpp',
        'engine/src/editor/ConstructionEditorCameraSystem.cpp',
        'engine/src/application/NativeGameApplication.cpp',
        'engine/include/rendering/StrategicCamera.h',
        'engine/src/rendering/StrategicViewProjection.cpp',
        'engine/include/studio/StudioApplication.h',
        'engine/src/studio/StudioApplication.cpp',
        'engine/tests/pass1508r5_gui_camera_normalization_tests.cpp',
        'engine/tests/s15b_camera_orbit_smoke.cpp'
    )
    $dirty=@(& git status --porcelain -- @files)
    if($LASTEXITCODE -ne 0){throw 'Git status failed. No files changed.'}
    if($dirty.Count -gt 0){throw ("S15B1 source precondition failed. Modified target files: `n"+($dirty -join "`n"))}
    & git apply --check -- $diff
    if($LASTEXITCODE -ne 0){throw 'S15B1 git apply --check failed. No files changed.'}
    if($CheckOnly){Write-Host '[PASS] S15B1 baseline, clean paths, and git apply preflight.' -ForegroundColor Green;return}
    & git apply -- $diff
    if($LASTEXITCODE -ne 0){throw 'S15B1 git apply failed. Inspect repository state before retry.'}
    & git diff --check -- @files
    if($LASTEXITCODE -ne 0){
        & git apply --reverse -- $diff
        throw 'S15B1 whitespace validation failed; attempted reverse application.'
    }
    Write-Host '[PASS] S15B1 camera changes applied; run PCC Full Quality Gate and Windows interactive Studio tests before commit.' -ForegroundColor Green
}
finally { Pop-Location }

param(
    [string]$Root = (Get-Location).Path,
    [switch]$Quiet
)

$ErrorActionPreference = "Stop"
$Root = [System.IO.Path]::GetFullPath($Root)
$EngineRoot = Join-Path $Root "engine"
$CMakePath = Join-Path $EngineRoot "CMakeLists.txt"
$ArtifactRoot = Join-Path $Root "artifacts\gates\certifications\continuity"
New-Item -ItemType Directory -Force -Path $ArtifactRoot | Out-Null

$stamp = Get-Date -Format "yyyyMMdd-HHmmss"
$report = Join-Path $ArtifactRoot ("PASS_CONTINUITY_CERTIFICATION_" + $stamp + ".txt")
$latest = Join-Path $ArtifactRoot "LATEST_PASS_CONTINUITY_CERTIFICATION.txt"

$lines = [System.Collections.Generic.List[string]]::new()
function Emit([string]$Text) {
    $lines.Add($Text) | Out-Null
    if (-not $Quiet) { Write-Host $Text }
}

Emit "========================================================================"
Emit " CODENAME SUBSPACE PASS / SOURCE CONTINUITY AUDIT"
Emit "========================================================================"
Emit "Root: $Root"
Emit "Timestamp: $(Get-Date -Format 'yyyy-MM-dd HH:mm:ss zzz')"
Emit ""

$failures = [System.Collections.Generic.List[string]]::new()

# R32R1: approved R32 carried a transactional Construct source cutover, but
# Full Gate previously reached CTest without invoking it. Pass/source
# continuity is the disk-loaded pre-build checkpoint used by both Full Gate
# and render builds, so close that one-time approved migration here as well as
# from the updated root utility. This keeps same-process PCC sessions safe.
$pass1509 = Join-Path $Root 'tools\control\static-gates\pass1509_studio_construct_cutover.cmake'
$builderSource = Join-Path $Root 'engine\src\ship_editor\ShipyardBuilderSystem.cpp'
if (Test-Path -LiteralPath $pass1509 -PathType Leaf) {
    if (-not (Test-Path -LiteralPath $builderSource -PathType Leaf)) {
        throw 'PASS1509 is installed but ShipyardBuilderSystem.cpp is missing.'
    }
    $builderText = Get-Content -LiteralPath $builderSource -Raw -ErrorAction Stop
    if ($builderText -notmatch 'ConstructionScalePivotSystem\.h' -or
        $builderText -notmatch 'Modeled shape scaled / opposite face anchored') {
        $migration = Join-Path $Root 'tools\studio\apply_studio_r23_r32_normalization.py'
        $assertions = Join-Path $Root 'tools\studio\studio_r32_source_assertions.py'
        if (-not (Test-Path -LiteralPath $migration -PathType Leaf)) {
            throw 'PASS1509 is installed but the R32 source migration helper is missing.'
        }
        if (-not (Test-Path -LiteralPath $assertions -PathType Leaf)) {
            throw 'PASS1509 is installed but the R32 source assertions helper is missing.'
        }
        $python = Get-Command python -ErrorAction SilentlyContinue
        if (-not $python) { throw 'Python is required to close the approved R32 Studio source migration.' }
        Emit '[R32R1] Pending approved Studio Construct source cutover detected; applying transaction before continuity/build.'
        & $python.Source $migration --root $Root --apply
        if ($LASTEXITCODE -ne 0) { throw 'R32 Studio Construct source migration failed.' }
        & $python.Source $assertions --root $Root
        if ($LASTEXITCODE -ne 0) { throw 'R32 Studio source assertions failed after migration.' }
        $builderText = Get-Content -LiteralPath $builderSource -Raw -ErrorAction Stop
        if ($builderText -notmatch 'ConstructionScalePivotSystem\.h' -or
            $builderText -notmatch 'Modeled shape scaled / opposite face anchored') {
            throw 'R32 migration reported success but required Construct source tokens remain absent.'
        }
        Emit '[PASS] R32 Studio Construct source migration closed before native build.'
    }
    else {
        Emit '[PASS] R32 Studio Construct source cutover already present.'
    }
}


if (-not (Test-Path -LiteralPath $CMakePath -PathType Leaf)) {
    $failures.Add("engine/CMakeLists.txt is missing.") | Out-Null
}
else {
    $cmake = Get-Content -LiteralPath $CMakePath -Raw
    $matches = [regex]::Matches($cmake, '(?i)tests[\\/][A-Za-z0-9_.+\-/]+')
    $explicitTests = @(
        $matches |
        ForEach-Object { $_.Value.Replace('/', '\') } |
        Sort-Object -Unique
    )

    Emit ("Explicit CMake test/source references found: {0}" -f $explicitTests.Count)
    foreach ($relative in $explicitTests) {
        $candidate = Join-Path $EngineRoot $relative
        if (-not (Test-Path -LiteralPath $candidate -PathType Leaf)) {
            $failures.Add(("Missing explicit CMake source: engine\{0}" -f $relative)) | Out-Null
        }
    }
}

$knownPass655Test = Join-Path $Root "engine\tests\pass655_674_editor_symmetry_camera_thumbnail_tests.cpp"
$knownPass655AppliedEvidence = $false
$updateLogs = Join-Path $Root "updates\logs"
if (Test-Path -LiteralPath $updateLogs) {
    foreach ($log in @(Get-ChildItem -LiteralPath $updateLogs -Filter "*.log" -File -ErrorAction SilentlyContinue)) {
        try {
            if ((Get-Content -LiteralPath $log.FullName -Raw -ErrorAction Stop) -match 'Pass655_674|Pass655-674|ConstructionSymmetryCameraThumbnail') {
                $knownPass655AppliedEvidence = $true
                break
            }
        } catch {}
    }
}

if (-not (Test-Path -LiteralPath $knownPass655Test -PathType Leaf)) {
    Emit ""
    Emit "[FAIL] Known pass discontinuity: Pass655-674 baseline is absent."
    Emit "       Required source: engine\tests\pass655_674_editor_symmetry_camera_thumbnail_tests.cpp"
    Emit "       Expected handoff: Codename_Subspace_Pass655_674_ConstructionSymmetryCameraThumbnail_20260907.zip"
    Emit "       Pass675-744 was recorded as applying over Pass654, so this is a skipped baseline, not a CMake typo."
    if (-not $knownPass655AppliedEvidence) {
        Emit "       No Pass655-674 apply record was found under updates\logs."
    }
    $failures.Add("Pass655-674 Construction/Symmetry/Camera/Thumbnail baseline is missing.") | Out-Null
}
else {
    Emit "[PASS] Pass655-674 editor symmetry/camera/thumbnail test source exists."
}

Emit ""
if ($failures.Count -eq 0) {
    Emit "[PASS] Pass/source continuity certified."
    $result = "PASS"
}
else {
    Emit ("[FAIL] Pass/source continuity rejected: {0} issue(s)." -f $failures.Count)
    foreach ($failure in $failures) { Emit ("  - " + $failure) }
    $result = "FAIL"
}

$lines.Add("") | Out-Null
$lines.Add(("Result: " + $result)) | Out-Null
[System.IO.File]::WriteAllLines($report, $lines.ToArray(), [System.Text.UTF8Encoding]::new($false))
Copy-Item -LiteralPath $report -Destination $latest -Force

if (-not $Quiet) {
    Write-Host ""
    Write-Host "Report: $report"
}

if ($failures.Count -gt 0) { exit 1 }
exit 0

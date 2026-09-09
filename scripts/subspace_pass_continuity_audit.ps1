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

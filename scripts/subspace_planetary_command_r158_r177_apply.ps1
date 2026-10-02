param(
    [string]$Root = "."
)

$ErrorActionPreference = "Stop"
$rootPath = (Resolve-Path -LiteralPath $Root).Path
Write-Host "========================================================================"
Write-Host " R158-R177 PLANETARY COMMAND -> R179 CANONICAL MATERIALIZATION VERIFIER"
Write-Host "========================================================================"
Write-Host "Root: $rootPath"
Write-Host ""
Write-Host "The historical 17-transform text migration is retired after R179."
Write-Host "This entrypoint now verifies canonical source semantics and never replays stale preimages."

$required = @(
    @{ Path = "engine/include/economy/PlanetaryIndustrySystem.h"; Tokens = @("enum class PiClaimState", "enum class PiOverlayMode", "enum class PiProjectionMode", "PlaceGoverned", "AdvanceSector", "PiSectorIdentity") },
    @{ Path = "engine/src/economy/PlanetaryIndustrySystem.cpp"; Tokens = @("CLAIM REQUIRES CONTIGUOUS OWNED BORDER", "PlaceGoverned", "ClaimFrontier", "planetId") },
    @{ Path = "engine/include/input/InputState.h"; Tokens = @("PlanetaryCommandCycleOverlay", "FleetCommandCancel") },
    @{ Path = "engine/src/platform/NativeWindow.cpp"; Tokens = @("VK_F5", "PlanetaryCommandCycleOverlay") },
    @{ Path = "engine/include/integration/PlayerFacingIntegrationSystem.h"; Tokens = @("PlanetaryCommandLayout", "PiProjectionMode projection", "AdvancePlanetarySector") },
    @{ Path = "engine/src/integration/PlayerFacingIntegrationSystem.cpp"; Tokens = @("HitTestPlanetaryHex", "CyclePlanetaryOverlay", "TogglePlanetaryProjection", "RecommendedIndustryKind") },
    @{ Path = "engine/src/application/NativeGameApplication.cpp"; Tokens = @("R179 Planetary Command owns pointer clicks", "_window.IsShiftDown()", "PlanetaryCommandCycleOverlay") },
    @{ Path = "engine/src/application/NativeBattlefieldRenderer.cpp"; Tokens = @("PLANETARY COMMAND - ", "SECTOR INSPECTOR", "CLAIM FRONTIER") },
    @{ Path = "engine/src/ui/SandboxWorkspaceSystem.cpp"; Tokens = @("PLANETARY COMMAND", "Claim contiguous sector") }
)

$failures = New-Object System.Collections.Generic.List[string]
foreach ($entry in $required) {
    $path = Join-Path $rootPath $entry.Path
    if (-not (Test-Path -LiteralPath $path)) {
        $failures.Add("Missing canonical source file: $($entry.Path)")
        continue
    }
    $text = [System.IO.File]::ReadAllText($path)
    foreach ($token in $entry.Tokens) {
        if ($text.IndexOf($token, [System.StringComparison]::Ordinal) -lt 0) {
            $failures.Add("Missing canonical token '$token' in $($entry.Path)")
        }
    }
}

if ($failures.Count -gt 0) {
    foreach ($failure in $failures) { Write-Host "[FAIL] $failure" }
    throw "R179 canonical Planetary Command source is incomplete. Historical migration will not be replayed over drifted source."
}

$python = Get-Command python -ErrorAction SilentlyContinue
if ($null -ne $python) {
    $test = Join-Path $rootPath "tools/control/tests/test_r158_r177_planetary_command_source.py"
    if (Test-Path -LiteralPath $test) {
        & $python.Source $test $rootPath
        if ($LASTEXITCODE -ne 0) { throw "Planetary Command source verifier failed with exit code $LASTEXITCODE" }
    }
}

$migrationDir = Join-Path $rootPath "artifacts/migrations"
New-Item -ItemType Directory -Force -Path $migrationDir | Out-Null
$record = [ordered]@{
    schema = "subspace.planetary-command-materialization.v2"
    historicalRange = "R158-R177"
    supersededBy = "R179"
    sourceMode = "canonical-semantic-verification"
    legacyTextTransforms = "retired"
    status = "materialized"
    timestamp = [DateTimeOffset]::Now.ToString("o")
}
$json = $record | ConvertTo-Json -Depth 4
$markerNames = @(
    "R158_R177_PLANETARY_COMMAND_MATERIALIZED.json",
    "R158-R177_PLANETARY_COMMAND_MATERIALIZED.json",
    "r158_r177_planetary_command_materialized.json"
)
foreach ($name in $markerNames) {
    [System.IO.File]::WriteAllText((Join-Path $migrationDir $name), $json, [System.Text.UTF8Encoding]::new($false))
}

Write-Host "[PASS] R179 canonical Planetary Command source is already materialized."
Write-Host "[PASS] Historical exact/indent-normalized transform replay is no longer required."
exit 0

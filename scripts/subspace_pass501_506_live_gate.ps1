param(
    [string]$Root = (Resolve-Path (Join-Path $PSScriptRoot ".."))
)

$ErrorActionPreference = 'Stop'

function Pass([string]$Message) { Write-Host "[PASS] $Message" }
function Info([string]$Message) { Write-Host "[INFO] $Message" }

Write-Host "=== SUBSPACE PASS501-506 LIVE SHIPYARD AUTHORING GATE ==="
Write-Host "Root: $Root"

$required = @(
    'engine\include\ship_editor\ShipyardAuthoringAuthority.h',
    'engine\src\ship_editor\ShipyardAuthoringAuthority.cpp',
    'engine\include\ship_editor\ShipyardAuthoringBridge.h',
    'engine\src\ship_editor\ShipyardAuthoringBridge.cpp',
    'content\derived\greyoxide_shipyard_v07\authoring_overrides\pass497_506_certified_overrides.json',
    'content\derived\greyoxide_shipyard_v07\authoring_overrides\pass498_module_authoring_catalog.json',
    'tools\smoke\pass497_506_shipyard_authoring_smoke.cpp'
)

foreach ($rel in $required) {
    $path = Join-Path $Root $rel
    if (-not (Test-Path -LiteralPath $path)) {
        throw "Missing Pass501-506 live-gate prerequisite: $rel"
    }
    Pass $rel
}

$catalogPath = Join-Path $Root 'content\derived\greyoxide_shipyard_v07\authoring_overrides\pass498_module_authoring_catalog.json'
$catalog = Get-Content -LiteralPath $catalogPath -Raw | ConvertFrom-Json
if ([int]$catalog.module_count -ne 156) {
    throw "Pass498 authoring catalog reports $($catalog.module_count) modules; expected preserved 156-module corpus."
}
if (@($catalog.modules).Count -ne 156) {
    throw "Pass498 authoring catalog physically contains $(@($catalog.modules).Count) module entries; expected 156."
}
Pass "Pass498 catalog preserves all 156 Greyoxide modules"

$overridePath = Join-Path $Root 'content\derived\greyoxide_shipyard_v07\authoring_overrides\pass497_506_certified_overrides.json'
$overrides = Get-Content -LiteralPath $overridePath -Raw | ConvertFrom-Json
$wing149 = @($overrides.modules | Where-Object {
    $_.definition_id -eq 'shipyard_a_wing_149_shipyard_wing_003_miscfinhanger'
}) | Select-Object -First 1
if (-not $wing149) { throw "Certified wing_149 override is missing." }
if ([string]$wing149.subtype -ne 'LateralWing') { throw "wing_149 is not certified as LateralWing." }
if ([string]$wing149.certification -ne 'Certified') { throw "wing_149 certification regressed." }
if (-not [bool]$wing149.attachment_frame.lateral_surface) { throw "wing_149 lateral-surface flag regressed." }
Pass "Pass497 wing_149 certified attachment/orientation override is present"

$authorityHeader = Get-Content -LiteralPath (Join-Path $Root 'engine\include\ship_editor\ShipyardAuthoringAuthority.h') -Raw
$bridgeHeader = Get-Content -LiteralPath (Join-Path $Root 'engine\include\ship_editor\ShipyardAuthoringBridge.h') -Raw
$smokeSource = Get-Content -LiteralPath (Join-Path $Root 'tools\smoke\pass497_506_shipyard_authoring_smoke.cpp') -Raw

$requiredAuthorityTokens = @(
    'ShipyardRepairAction',
    'ScorePlacement',
    'RecommendRepair',
    'PreviewReroll',
    'PreviewReplacement',
    'PreviewRerollOperation',
    'CommitReroll',
    'RestorePreviousReroll',
    'LockSlot',
    'PairSlots',
    'InferMaterialZone',
    'ValidateBlueprint'
)
foreach ($token in $requiredAuthorityTokens) {
    if ($authorityHeader -notmatch [regex]::Escape($token)) {
        throw "Shipyard authoring authority contract is missing '$token'."
    }
}
Pass "Pass501-505 authoring contracts are present"

$requiredBridgeTokens = @(
    'BuildPreservedBlueprint',
    'ApplyModuleChoicesPreservingPlacement'
)
foreach ($token in $requiredBridgeTokens) {
    if ($bridgeHeader -notmatch [regex]::Escape($token)) {
        throw "Shipyard authoring bridge contract is missing '$token'."
    }
}
Pass "Pass506 preservation bridge contract is present"

$requiredSmokeTokens = @(
    'ShipyardRepairAction::Keep',
    'ShipyardRerollMode::Similar',
    'ShipyardRerollMode::Pair',
    'RestorePreviousReroll',
    'LockSlot',
    'InferMaterialZone',
    'ValidateBlueprint',
    'ApplyModuleChoicesPreservingPlacement'
)
foreach ($token in $requiredSmokeTokens) {
    if ($smokeSource -notmatch [regex]::Escape($token)) {
        throw "Pass501-506 smoke coverage is missing '$token'."
    }
}
Pass "Pass501-506 smoke source covers preserve/reroll/pair/lock/material/validation/bridge behavior"

$cmake = Get-Command cmake -ErrorAction Stop
$gateRoot = Join-Path $Root 'engine\build\pass501_506_live_gate'
$sourceRoot = Join-Path $gateRoot 'src'
$buildRoot = Join-Path $gateRoot 'build'

if (Test-Path -LiteralPath $gateRoot) {
    Remove-Item -LiteralPath $gateRoot -Recurse -Force
}
New-Item -ItemType Directory -Force -Path $sourceRoot | Out-Null

$authorityCpp = (Join-Path $Root 'engine\src\ship_editor\ShipyardAuthoringAuthority.cpp').Replace('\','/')
$bridgeCpp = (Join-Path $Root 'engine\src\ship_editor\ShipyardAuthoringBridge.cpp').Replace('\','/')
$moduleSystemCpp = (Join-Path $Root 'engine\src\content\ShipyardModuleSystem.cpp').Replace('\','/')
$variantSystemCpp = (Join-Path $Root 'engine\src\rendering\ProceduralVisualVariantSystem.cpp').Replace('\','/')
$mathCpp = (Join-Path $Root 'engine\src\core\Math.cpp').Replace('\','/')
$taxonomyCpp = (Join-Path $Root 'engine\src\content\ShipyardPartTaxonomySystem.cpp').Replace('\','/')
$designLanguageCpp = (Join-Path $Root 'engine\src\ships\ShipyardDesignLanguageSystem.cpp').Replace('\','/')
$smokeCpp = (Join-Path $Root 'tools\smoke\pass497_506_shipyard_authoring_smoke.cpp').Replace('\','/')
$includeDir = (Join-Path $Root 'engine\include').Replace('\','/')

$liveLinkDeps = @(
    'engine\src\core\Math.cpp',
    'engine\src\content\ShipyardPartTaxonomySystem.cpp',
    'engine\src\ships\ShipyardDesignLanguageSystem.cpp'
)
foreach ($rel in $liveLinkDeps) {
    if (-not (Test-Path -LiteralPath (Join-Path $Root $rel))) {
        throw "Missing Pass501-506 live-link dependency: $rel"
    }
    Pass "Live-link dependency present: $rel"
}

$cmakeLists = @"
cmake_minimum_required(VERSION 3.16)
project(SubspacePass501506LiveGate LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
add_executable(subspace_pass501_506_live_gate
    "$smokeCpp"
    "$authorityCpp"
    "$bridgeCpp"
    "$moduleSystemCpp"
    "$variantSystemCpp"
    "$mathCpp"
    "$taxonomyCpp"
    "$designLanguageCpp"
)
target_include_directories(subspace_pass501_506_live_gate PRIVATE "$includeDir")
if(MSVC)
    target_compile_options(subspace_pass501_506_live_gate PRIVATE /W4 /permissive-)
else()
    target_compile_options(subspace_pass501_506_live_gate PRIVATE -Wall -Wextra -Wpedantic)
endif()
"@
Set-Content -LiteralPath (Join-Path $sourceRoot 'CMakeLists.txt') -Value $cmakeLists -Encoding UTF8

$generator = $null
$architecture = $null
$mainCache = Join-Path $Root 'engine\build\CMakeCache.txt'
if (Test-Path -LiteralPath $mainCache) {
    foreach ($line in Get-Content -LiteralPath $mainCache) {
        if ($line -match '^CMAKE_GENERATOR:INTERNAL=(.+)$') { $generator = $Matches[1].Trim() }
        if ($line -match '^CMAKE_GENERATOR_PLATFORM:INTERNAL=(.+)$') { $architecture = $Matches[1].Trim() }
    }
}

$configureArgs = @('-S', $sourceRoot, '-B', $buildRoot)
if ($generator) {
    $configureArgs += @('-G', $generator)
    if ($architecture) { $configureArgs += @('-A', $architecture) }
}
Info ("CMake generator: " + $(if ($generator) { $generator } else { 'default' }))
& $cmake.Source @configureArgs
if ($LASTEXITCODE -ne 0) { throw "Pass501-506 standalone smoke configure failed with exit code $LASTEXITCODE." }

& $cmake.Source --build $buildRoot --config Debug
if ($LASTEXITCODE -ne 0) { throw "Pass501-506 standalone smoke build failed with exit code $LASTEXITCODE." }

$exeCandidates = @(
    (Join-Path $buildRoot 'subspace_pass501_506_live_gate.exe'),
    (Join-Path $buildRoot 'Debug\subspace_pass501_506_live_gate.exe'),
    (Join-Path $buildRoot 'Release\subspace_pass501_506_live_gate.exe')
)
$gateExe = $exeCandidates | Where-Object { Test-Path -LiteralPath $_ } | Select-Object -First 1
if (-not $gateExe) {
    throw "Pass501-506 standalone smoke executable was not produced."
}

& $gateExe
if ($LASTEXITCODE -ne 0) { throw "Pass501-506 live smoke failed with exit code $LASTEXITCODE." }
Pass "Pass501-506 live C++ smoke executed successfully against current repository source"

Write-Host ""
Write-Host "[PASS] PASS501-506 LIVE SHIPYARD AUTHORING GATE"

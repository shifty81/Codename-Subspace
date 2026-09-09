param(
    [string]$RepositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
)

$ErrorActionPreference = "Stop"
$manifestPath = Join-Path $RepositoryRoot "tools\dependencies\subspace_asset_toolchain.json"
$includePath = Join-Path $RepositoryRoot "engine\include"
$smokePath = Join-Path $RepositoryRoot "tools\smoke\pass425r3a_asset_foundation_smoke.cpp"
$buildDir = Join-Path $RepositoryRoot "artifacts\pass425r3a"
$exePath = Join-Path $buildDir "pass425r3a_asset_foundation_smoke.exe"

Write-Host "=== SUBSPACE PASS425R3A ASSET FOUNDATION GATE ==="

if (!(Test-Path $manifestPath)) { throw "Missing asset toolchain manifest: $manifestPath" }
if (!(Test-Path $smokePath)) { throw "Missing R3A smoke source: $smokePath" }

$manifest = Get-Content -Raw $manifestPath | ConvertFrom-Json
if ($manifest.schema_version -ne 1) { throw "Unsupported asset toolchain manifest schema." }
if ($manifest.canonical_runtime_asset -ne "glTF 2.0 GLB") { throw "Canonical runtime asset must be glTF 2.0 GLB." }
if ($manifest.policy -ne "PINNED_OFFLINE_AFTER_BOOTSTRAP") { throw "Asset toolchain policy is not fail-closed/offline capable." }

$required = @("ufbx", "cgltf", "MikkTSpace", "glTF-Validator")
foreach ($name in $required) {
    if (!($manifest.dependencies | Where-Object { $_.name -eq $name })) {
        throw "Missing required toolchain dependency lock: $name"
    }
}

New-Item -ItemType Directory -Force -Path $buildDir | Out-Null

$cl = Get-Command cl.exe -ErrorAction SilentlyContinue
if (-not $cl) {
    Write-Warning "cl.exe is not on PATH. Manifest/schema checks PASS; compile smoke deferred to Visual Studio developer environment/Full Gate integration."
    exit 0
}

& $cl.Path /nologo /std:c++17 /EHsc /W4 /I$includePath $smokePath /Fe:$exePath
if ($LASTEXITCODE -ne 0) { throw "R3A asset foundation smoke failed to compile." }

& $exePath
if ($LASTEXITCODE -ne 0) { throw "R3A asset foundation smoke failed." }

Write-Host "[PASS] PASS425R3A Asset Foundation"

param(
    [string]$Root = (Get-Location).Path,
    [switch]$Apply
)

$ErrorActionPreference = "Stop"
$rootPath = Resolve-Path -LiteralPath $Root
$root = $rootPath.Path
$referenceRoot = Join-Path $root "reference\csharp-to-cpp-source"
$destinationAvorion = Join-Path $referenceRoot "AvorionLike"
$destinationSln = Join-Path $referenceRoot "AvorionLike.sln"
$legacyArchiveRoot = Join-Path $root "docs\archive\reference\csharp-prototype"

$planned = New-Object System.Collections.Generic.List[string]

function Plan-Move {
    param([string]$Source, [string]$Destination)
    if (Test-Path -LiteralPath $Source) {
        $planned.Add("MOVE `"$Source`" -> `"$Destination`"")
    }
}

Plan-Move -Source (Join-Path $root "AvorionLike") -Destination $destinationAvorion
Plan-Move -Source (Join-Path $root "AvorionLike.sln") -Destination $destinationSln
Plan-Move -Source (Join-Path $legacyArchiveRoot "AvorionLike") -Destination $destinationAvorion
Plan-Move -Source (Join-Path $legacyArchiveRoot "AvorionLike.sln") -Destination $destinationSln

Write-Host "Subspace AvorionLike -> C++ conversion intake"
Write-Host "Root: $root"
Write-Host "Destination: $referenceRoot"
Write-Host ""
Write-Host "This keeps the active project C++-only while preserving the old C# project as source-to-port, not as retired archive."
Write-Host ""

if ($planned.Count -eq 0) {
    if (Test-Path -LiteralPath $destinationAvorion) {
        Write-Host "C# migration source already exists at: $destinationAvorion"
    }
    else {
        Write-Warning "No AvorionLike source found in root or previous docs archive."
    }
}
else {
    Write-Host "Planned operations:"
    foreach ($item in $planned) { Write-Host " - $item" }
}

if (-not $Apply) {
    Write-Host ""
    Write-Host "Preview only. Re-run with -Apply to move the C# source into the conversion-source area."
    exit 0
}

New-Item -ItemType Directory -Force -Path $referenceRoot | Out-Null

function Invoke-MoveIfExists {
    param([string]$Source, [string]$Destination)
    if (-not (Test-Path -LiteralPath $Source)) { return }
    if (Test-Path -LiteralPath $Destination) {
        Write-Warning "Destination already exists, preserving both: $Destination"
        $suffix = Get-Date -Format "yyyyMMdd-HHmmss"
        $Destination = "$Destination.duplicate-$suffix"
    }
    New-Item -ItemType Directory -Force -Path (Split-Path -Parent $Destination) | Out-Null
    Move-Item -LiteralPath $Source -Destination $Destination -Force
    Write-Host "Moved: $Source -> $Destination"
}

Invoke-MoveIfExists -Source (Join-Path $root "AvorionLike") -Destination $destinationAvorion
Invoke-MoveIfExists -Source (Join-Path $root "AvorionLike.sln") -Destination $destinationSln
Invoke-MoveIfExists -Source (Join-Path $legacyArchiveRoot "AvorionLike") -Destination $destinationAvorion
Invoke-MoveIfExists -Source (Join-Path $legacyArchiveRoot "AvorionLike.sln") -Destination $destinationSln

$readme = Join-Path $referenceRoot "README.md"
@(
    "# C# Source To Port",
    "",
    "This folder stores the old AvorionLike C# prototype as a migration source for the active C++ client/runtime.",
    "",
    "It is not the active build path and should not be launched from Visual Studio as the game.",
    "",
    "Conversion rule: do not delete source files from here until the matching C++ implementation has been reviewed and marked ported in the migration ledger.",
    "",
    "Active path:",
    "",
    "```text",
    "engine/",
    "SubspaceTools.ps1 -Action build-render -Clean",
    "SubspaceTools.ps1 -Action run-client",
    "```"
) | Set-Content -LiteralPath $readme

$matrixScript = Join-Path $root "scripts\subspace_generate_cpp_port_matrix.ps1"
if (Test-Path -LiteralPath $matrixScript) {
    & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $matrixScript -Root $root
}

Write-Host "C++ conversion intake complete."

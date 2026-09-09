param(
    [string]$Root = (Get-Location).Path,
    [string]$OutDir = "docs\migration\generated"
)

$ErrorActionPreference = "Stop"
$rootPath = Resolve-Path -LiteralPath $Root
$root = $rootPath.Path
$outPath = Join-Path $root $OutDir
New-Item -ItemType Directory -Force -Path $outPath | Out-Null

function Get-RelativePath {
    param([string]$Base, [string]$Path)
    $baseUri = [Uri]((Resolve-Path -LiteralPath $Base).Path.TrimEnd('\') + '\')
    $pathUri = [Uri](Resolve-Path -LiteralPath $Path)
    return [Uri]::UnescapeDataString($baseUri.MakeRelativeUri($pathUri).ToString()).Replace('/', '\')
}

function Find-CSharpSourceRoot {
    param([string]$Root)

    $candidates = @(
        "AvorionLike",
        "reference\csharp-to-cpp-source\AvorionLike",
        "docs\archive\reference\csharp-prototype\AvorionLike"
    )

    foreach ($candidate in $candidates) {
        $path = Join-Path $Root $candidate
        if (Test-Path -LiteralPath $path) { return $path }
    }

    throw "No AvorionLike C# source root found. Expected AvorionLike/, reference/csharp-to-cpp-source/AvorionLike/, or docs/archive/reference/csharp-prototype/AvorionLike/."
}

function Normalize-Stem {
    param([string]$Name)
    $value = $Name.ToLowerInvariant()
    $value = $value -replace '^x4', ''
    $value = $value -replace '^eve', ''
    $value = $value -replace 'inspired', ''
    $value = $value -replace 'enhanced', ''
    $value = $value -replace 'modular', ''
    $value = $value -replace 'procedural', ''
    $value = $value -replace '[^a-z0-9]', ''
    return $value
}

function Map-CategoryToCppTarget {
    param([string]$Category, [string]$Stem)

    switch -Regex ($Category) {
        '^AI$' { return "engine/include/ai + engine/src/ai" }
        '^Building$' { return "engine/include/ship_editor or future engine/include/building" }
        '^Combat$' { return "engine/include/combat + engine/include/weapons" }
        '^Common$' { return "engine/include/core" }
        '^Config|Configuration$' { return "engine/include/core/config" }
        '^DevTools$' { return "engine/include/developer + engine/include/debug_tools" }
        '^ECS$' { return "engine/include/core/ecs" }
        '^Economy$' { return "engine/include/trading + engine/include/crafting + engine/include/inventory" }
        '^Events$' { return "engine/include/core/events" }
        '^Faction$' { return "engine/include/factions + engine/include/diplomacy + engine/include/reputation" }
        '^Fleet$' { return "engine/include/fleet + engine/include/crew + engine/include/formation" }
        '^Graphics$' { return "engine/include/rendering or playable client renderer" }
        '^Input$' { return "engine/include/runtime or engine/include/developer/input" }
        '^Logging$' { return "engine/include/core/logging" }
        '^Mining$' { return "engine/include/mining" }
        '^Modular$' { return "engine/include/ships + engine/include/ship_editor" }
        '^Navigation$' { return "engine/include/navigation + engine/include/scanning" }
        '^Networking$' { return "engine/include/networking" }
        '^Persistence$' { return "engine/include/core/persistence" }
        '^Physics$' { return "engine/include/core/physics" }
        '^Power$' { return "engine/include/power" }
        '^Procedural$' { return "engine/include/procedural" }
        '^Progression$' { return "engine/include/rpg + engine/include/research + engine/include/achievement" }
        '^Quest$' { return "engine/include/quest" }
        '^RPG$' { return "engine/include/rpg" }
        '^Resources$' { return "engine/include/core/resources + engine/include/inventory + engine/include/crafting" }
        '^Scripting$' { return "engine/include/scripting" }
        '^SolarSystem$' { return "engine/include/navigation or future engine/include/solar_system" }
        '^Spatial$' { return "engine/include/core/physics" }
        '^Station$' { return "engine/include/hangar + engine/include/crafting" }
        '^Tutorial$' { return "engine/include/tutorial" }
        '^UI$' { return "engine/include/ui + engine/src/client" }
        '^Voxel$' { return "engine/include/ships + engine/include/ship_editor" }
        default { return "review" }
    }
}

$csharpRoot = Find-CSharpSourceRoot -Root $root
$cppFiles = @()
foreach ($cppRoot in @("engine\include", "engine\src")) {
    $path = Join-Path $root $cppRoot
    if (Test-Path -LiteralPath $path) {
        $cppFiles += Get-ChildItem -LiteralPath $path -Recurse -File -Include *.h,*.hpp,*.cpp,*.cxx
    }
}

$cppByStem = @{}
foreach ($file in $cppFiles) {
    $stem = Normalize-Stem -Name $file.BaseName
    if (-not $cppByStem.ContainsKey($stem)) { $cppByStem[$stem] = @() }
    $cppByStem[$stem] += (Get-RelativePath -Base $root -Path $file.FullName)
}

$rows = @()
$csharpFiles = Get-ChildItem -LiteralPath $csharpRoot -Recurse -File -Filter *.cs | Sort-Object FullName
foreach ($file in $csharpFiles) {
    $relativeCs = Get-RelativePath -Base $csharpRoot -Path $file.FullName
    $parts = $relativeCs -split '\\'
    $category = if ($parts.Count -ge 3 -and $parts[0] -eq "Core") { $parts[1] } elseif ($parts.Count -ge 2) { $parts[0] } else { "root" }
    $stem = Normalize-Stem -Name $file.BaseName
    $matches = @()
    if ($cppByStem.ContainsKey($stem)) { $matches = $cppByStem[$stem] }

    $status = if ($matches.Count -gt 0) { "ReviewExistingCpp" } else { "NeedsCppPort" }
    if ($relativeCs -match '\\Examples\\|^Examples\\') { $status = "ReferenceExample" }
    if ($file.BaseName -match 'Test|Example|Showcase') { $status = "ReferenceExample" }

    $target = Map-CategoryToCppTarget -Category $category -Stem $stem
    $priority = switch -Regex ($category) {
        'Voxel|Modular|Mining|Combat|Navigation|Physics|Resources|Economy|Procedural|UI|DevTools' { "P1"; break }
        'Faction|Fleet|Quest|Tutorial|RPG|Station|Power|Persistence' { "P2"; break }
        'Graphics|Scripting|Networking|SolarSystem|AI' { "P3"; break }
        default { "P4" }
    }

    $rows += [PSCustomObject]@{
        Status = $status
        Priority = $priority
        Category = $category
        CSharpSource = (Get-RelativePath -Base $root -Path $file.FullName)
        ProposedCppTarget = $target
        CurrentCppCandidates = (($matches | Sort-Object -Unique) -join '; ')
        Notes = if ($matches.Count -gt 0) { "Existing C++ candidate requires semantic comparison, not blind deletion." } else { "Port behavior into C++ before retiring this source file." }
    }
}

$timestamp = Get-Date -Format "yyyyMMdd-HHmmss"
$csvPath = Join-Path $outPath "AvorionLike_CppPortMatrix_$timestamp.csv"
$mdPath = Join-Path $outPath "AvorionLike_CppPortMatrix_$timestamp.md"
$latestCsv = Join-Path $outPath "LATEST_AvorionLike_CppPortMatrix.csv"
$latestMd = Join-Path $outPath "LATEST_AvorionLike_CppPortMatrix.md"

$rows | Export-Csv -NoTypeInformation -LiteralPath $csvPath
$rows | Export-Csv -NoTypeInformation -LiteralPath $latestCsv

$grouped = $rows | Group-Object Category | Sort-Object Name
$lines = New-Object System.Collections.Generic.List[string]
$lines.Add("# AvorionLike → C++ Port Matrix")
$lines.Add("")
$lines.Add("Generated: $timestamp")
$lines.Add("")
$lines.Add("C# source root: ``$(Get-RelativePath -Base $root -Path $csharpRoot)``")
$lines.Add("")
$lines.Add("## Summary")
$lines.Add("")
$lines.Add("| Status | Count |")
$lines.Add("|---|---:|")
foreach ($statusGroup in ($rows | Group-Object Status | Sort-Object Name)) {
    $lines.Add("| $($statusGroup.Name) | $($statusGroup.Count) |")
}
$lines.Add("")
$lines.Add("## Categories")
$lines.Add("")
$lines.Add("| Category | Count | P1 | P2 | P3 | P4 |")
$lines.Add("|---|---:|---:|---:|---:|---:|")
foreach ($group in $grouped) {
    $p1 = ($group.Group | Where-Object Priority -eq "P1").Count
    $p2 = ($group.Group | Where-Object Priority -eq "P2").Count
    $p3 = ($group.Group | Where-Object Priority -eq "P3").Count
    $p4 = ($group.Group | Where-Object Priority -eq "P4").Count
    $lines.Add("| $($group.Name) | $($group.Count) | $p1 | $p2 | $p3 | $p4 |")
}
$lines.Add("")
$lines.Add("## P1 conversion queue")
$lines.Add("")
$lines.Add("| Status | Category | C# Source | Proposed C++ Target | Current C++ Candidates |")
$lines.Add("|---|---|---|---|---|")
foreach ($row in ($rows | Where-Object Priority -eq "P1" | Sort-Object Category, CSharpSource)) {
    $candidate = if ([string]::IsNullOrWhiteSpace($row.CurrentCppCandidates)) { "" } else { $row.CurrentCppCandidates.Replace('|','/') }
    $lines.Add("| $($row.Status) | $($row.Category) | ``$($row.CSharpSource)`` | $($row.ProposedCppTarget) | $candidate |")
}
$lines.Add("")
$lines.Add("## Rule")
$lines.Add("")
$lines.Add("Do not delete or archive a C# source file as complete until the C++ implementation has been reviewed against it and the row is marked ported in a follow-up ledger.")

$lines | Set-Content -LiteralPath $mdPath
$lines | Set-Content -LiteralPath $latestMd

Write-Host "AvorionLike C++ port matrix generated."
Write-Host "CSV: $csvPath"
Write-Host "MD : $mdPath"

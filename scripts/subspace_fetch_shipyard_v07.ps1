param(
    [string]$Root = (Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)),
    [switch]$Force,
    [switch]$Recertify,
    [switch]$SkipBlender
)

$ErrorActionPreference = "Stop"
$Root = [System.IO.Path]::GetFullPath($Root)
$ThirdPartyRoot = Join-Path $Root "content\third_party\greyoxide_shipyard_v07"
$DownloadRoot = Join-Path $ThirdPartyRoot "downloads"
$SourceRoot = Join-Path $ThirdPartyRoot "source"
$DerivedRoot = Join-Path $Root "content\derived\greyoxide_shipyard_v07"
$ModuleRoot = Join-Path $DerivedRoot "normalized_raw\modules"
$CertifiedRoot = Join-Path $DerivedRoot "certified"
$CertifiedModuleRoot = Join-Path $CertifiedRoot "modules"
$PrimaryUrl = "https://opengameart.org/sites/default/files/Shipyard_v0.7-Extracted_r1.7z"
$FallbackUrl = "https://opengameart.org/sites/default/files/Shipyard.blend.zip"
$PrimaryArchive = Join-Path $DownloadRoot "Shipyard_v0.7-Extracted_r1.7z"
$FallbackArchive = Join-Path $DownloadRoot "Shipyard.blend.zip"
$Portable7ZipVersion = "26.02"
$Portable7ZipUrl = "https://github.com/ip7z/7zip/releases/download/$Portable7ZipVersion/7zr.exe"
$Portable7ZipRoot = Join-Path $Root "tools\third_party\7zip"
$Portable7ZipExe = Join-Path $Portable7ZipRoot "7zr.exe"
$Utf8 = New-Object System.Text.UTF8Encoding -ArgumentList $false
$SourceLockRoot = Join-Path $Root "content\cache\locks"
$SourceLockPath = Join-Path $SourceLockRoot "greyoxide_shipyard_v07.lock.json"
New-Item -ItemType Directory -Force -Path $SourceLockRoot | Out-Null

foreach ($dir in @($ThirdPartyRoot, $DownloadRoot, $SourceRoot, $DerivedRoot, $ModuleRoot, $CertifiedRoot, $CertifiedModuleRoot)) {
    New-Item -ItemType Directory -Force -Path $dir | Out-Null
}

function Write-Step([string]$Message) { Write-Host "[SHIPYARD] $Message" -ForegroundColor Cyan }
function Write-Pass([string]$Message) { Write-Host "[PASS] $Message" -ForegroundColor Green }
function Write-Warn([string]$Message) { Write-Host "[WARN] $Message" -ForegroundColor Yellow }

function Download-File([string]$Url, [string]$OutFile) {
    if ((Test-Path -LiteralPath $OutFile) -and -not $Force) {
        Write-Pass "Using cached archive: $OutFile"
        return
    }
    Write-Step "Downloading $Url"
    $headers = @{ "User-Agent" = "Codename-Subspace-AssetIntake/1.0" }
    Invoke-WebRequest -Uri $Url -OutFile $OutFile -Headers $headers -UseBasicParsing
    if (-not (Test-Path -LiteralPath $OutFile)) { throw "Download did not produce $OutFile" }
    Write-Pass "Downloaded $([math]::Round((Get-Item $OutFile).Length / 1MB, 2)) MB"
}

function Find-Command([string[]]$Names) {
    foreach ($name in $Names) {
        $cmd = Get-Command $name -ErrorAction SilentlyContinue
        if ($cmd) { return $cmd.Source }
    }
    return $null
}

function Ensure-Portable7Zip {
    if (Test-Path -LiteralPath $Portable7ZipExe) {
        Write-Pass "Using cached project-local 7zr.exe: $Portable7ZipExe"
        return $Portable7ZipExe
    }

    if (-not ($env:OS -eq "Windows_NT")) {
        return $null
    }

    New-Item -ItemType Directory -Force -Path $Portable7ZipRoot | Out-Null
    Write-Step "No 7z-capable extractor found. Bootstrapping project-local 7-Zip $Portable7ZipVersion."
    $headers = @{ "User-Agent" = "Codename-Subspace-DependencyBootstrap/1.0" }
    Invoke-WebRequest -Uri $Portable7ZipUrl -OutFile $Portable7ZipExe -Headers $headers -UseBasicParsing
    if (-not (Test-Path -LiteralPath $Portable7ZipExe)) {
        throw "Portable 7-Zip bootstrap did not produce $Portable7ZipExe"
    }
    if ((Get-Item -LiteralPath $Portable7ZipExe).Length -lt 100KB) {
        Remove-Item -LiteralPath $Portable7ZipExe -Force -ErrorAction SilentlyContinue
        throw "Portable 7-Zip bootstrap produced an unexpectedly small executable."
    }

    $hash = (Get-FileHash -Algorithm SHA256 -LiteralPath $Portable7ZipExe).Hash.ToLowerInvariant()
    $provenance = [ordered]@{
        tool = '7zr.exe'
        version = $Portable7ZipVersion
        sourceUrl = $Portable7ZipUrl
        sourceAuthority = '7-Zip official release repository (ip7z/7zip)'
        sha256 = $hash
        purpose = 'Project-local extraction of governed third-party content archives'
        installed = (Get-Date).ToString('o')
    }
    $provenance | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath (Join-Path $Portable7ZipRoot 'PROVENANCE.generated.json') -Encoding UTF8
    Write-Pass "Project-local 7zr.exe ready (SHA256 $hash)."
    return $Portable7ZipExe
}

function Assert-OrCreateSourceLock([string]$ArchivePath, [string]$SourceUrl) {
    if (-not (Test-Path -LiteralPath $ArchivePath)) { throw "Cannot lock missing source archive: $ArchivePath" }
    $hash = (Get-FileHash -Algorithm SHA256 -LiteralPath $ArchivePath).Hash.ToLowerInvariant()
    if (Test-Path -LiteralPath $SourceLockPath) {
        $lock = Get-Content -LiteralPath $SourceLockPath -Raw | ConvertFrom-Json
        if ([string]$lock.sha256 -ne $hash) {
            $quarantine = Join-Path $Root ("content\quarantine\greyoxide_shipyard_v07_" + (Get-Date -Format 'yyyyMMdd-HHmmss') + [System.IO.Path]::GetExtension($ArchivePath))
            New-Item -ItemType Directory -Force -Path (Split-Path -Parent $quarantine) | Out-Null
            Copy-Item -LiteralPath $ArchivePath -Destination $quarantine -Force
            throw "Source archive hash changed from approved lock. Payload quarantined: $quarantine"
        }
        if ([string]$lock.sourceUrl -ne $SourceUrl) { throw "Source URL differs from approved source lock." }
        Write-Pass "Approved source lock verified: $hash"
        return $hash
    }

    # Greyoxide Shipyard is an already-approved CC0 project source. The first
    # governed intake captures its exact bytes; every later Full Gate fails
    # closed if the source host serves different content under the same URL.
    [ordered]@{
        schemaVersion = 1
        sourceId = 'greyoxide_shipyard_v07'
        sourceUrl = $SourceUrl
        sha256 = $hash
        trustModel = 'FIRST_APPROVED_FETCH_PINNED_AFTER_HASH'
        license = 'CC0-1.0'
        created = (Get-Date).ToString('o')
    } | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath $SourceLockPath -Encoding UTF8
    Write-Pass "Created governed source lock: $hash"
    return $hash
}

function Extract-7z([string]$Archive, [string]$Destination) {
    $seven = Find-Command @("7z.exe", "7zz.exe", "7za.exe", "7z", "7zz", "7za")
    if (-not $seven) {
        try { $seven = Ensure-Portable7Zip } catch { Write-Warn "Portable 7-Zip bootstrap failed: $($_.Exception.Message)" }
    }
    if ($seven) {
        Write-Step "Extracting with $seven"
        & $seven x "-o$Destination" -y $Archive | Out-Host
        if ($LASTEXITCODE -eq 0) { return $true }
        Write-Warn "7-Zip extraction returned exit code $LASTEXITCODE."
    }

    # Do not fall back to Windows tar.exe for .7z. The inbox debug bundle
    # proved that common Windows tar builds report 'LZMA codec is unsupported'.
    # Returning false here intentionally activates the governed Blender ZIP fallback.
    return $false
}

function Get-Class([string]$Name) {
    $v = $Name.ToLowerInvariant()
    if ($v -match 'cockpit|canopy|bridge|command') { return 'command' }
    if ($v -match 'engine|thruster|nozzle|exhaust|drive') { return 'propulsion' }
    if ($v -match 'hardpoint|turret|weapon|gun|mount') { return 'hardpoint' }
    if ($v -match 'greeble|detail|vent|panel|antenna|radar|sensor') { return 'detail' }
    if ($v -match 'wing|fin') { return 'wing' }
    if ($v -match 'adapter|connector|join|neck|fairing') { return 'adapter' }
    if ($v -match 'hull|body|fuselage|section|frame') { return 'hull' }
    return 'component'
}

function Get-SafeName([string]$Name) {
    $s = $Name.ToLowerInvariant() -replace '[^a-z0-9]+','_'
    $s = $s.Trim('_')
    if ([string]::IsNullOrWhiteSpace($s)) { return 'mesh' }
    return $s
}

function Convert-ObjCentered([string]$InputFile, [string]$OutputFile) {
    $lines = [System.IO.File]::ReadAllLines($InputFile)
    $points = New-Object System.Collections.Generic.List[object]
    foreach ($line in $lines) {
        if ($line -match '^v\s+([-+0-9eE\.]+)\s+([-+0-9eE\.]+)\s+([-+0-9eE\.]+)') {
            $ci = [System.Globalization.CultureInfo]::InvariantCulture
            $points.Add(@(
                [double]::Parse($matches[1], $ci),
                [double]::Parse($matches[2], $ci),
                [double]::Parse($matches[3], $ci)
            )) | Out-Null
        }
    }
    if ($points.Count -eq 0) { throw "OBJ has no vertices: $InputFile" }
    $minX = ($points | ForEach-Object { $_[0] } | Measure-Object -Minimum).Minimum
    $maxX = ($points | ForEach-Object { $_[0] } | Measure-Object -Maximum).Maximum
    $minY = ($points | ForEach-Object { $_[1] } | Measure-Object -Minimum).Minimum
    $maxY = ($points | ForEach-Object { $_[1] } | Measure-Object -Maximum).Maximum
    $minZ = ($points | ForEach-Object { $_[2] } | Measure-Object -Minimum).Minimum
    $maxZ = ($points | ForEach-Object { $_[2] } | Measure-Object -Maximum).Maximum
    $cx = ($minX + $maxX) * 0.5; $cy = ($minY + $maxY) * 0.5; $cz = ($minZ + $maxZ) * 0.5
    $ci = [System.Globalization.CultureInfo]::InvariantCulture
    $output = New-Object System.Collections.Generic.List[string]
    foreach ($line in $lines) {
        if ($line -match '^v\s+([-+0-9eE\.]+)\s+([-+0-9eE\.]+)\s+([-+0-9eE\.]+)(.*)$') {
            $x = [double]::Parse($matches[1], $ci) - $cx
            $y = [double]::Parse($matches[2], $ci) - $cy
            $z = [double]::Parse($matches[3], $ci) - $cz
            $output.Add(('v {0:R} {1:R} {2:R}{3}' -f $x,$y,$z,$matches[4])) | Out-Null
        } else {
            # Runtime OBJ loader ignores material directives, so preserving all
            # other lines is safe and keeps face/UV/normal indices intact.
            $output.Add($line) | Out-Null
        }
    }
    [System.IO.File]::WriteAllLines($OutputFile, $output, $Utf8)
}

function Find-Blender {
    $cmd = Get-Command blender.exe -ErrorAction SilentlyContinue
    if ($cmd) { return $cmd.Source }
    $candidates = @(
        "$env:ProgramFiles\Blender Foundation\Blender 4.5\blender.exe",
        "$env:ProgramFiles\Blender Foundation\Blender 4.4\blender.exe",
        "$env:ProgramFiles\Blender Foundation\Blender 4.3\blender.exe",
        "$env:ProgramFiles\Blender Foundation\Blender 4.2\blender.exe",
        "$env:ProgramFiles\Blender Foundation\Blender 4.1\blender.exe",
        "$env:ProgramFiles\Blender Foundation\Blender 4.0\blender.exe",
        "$env:ProgramFiles\Blender Foundation\Blender 3.6\blender.exe"
    )
    foreach ($candidate in $candidates) { if (Test-Path -LiteralPath $candidate) { return $candidate } }
    return $null
}

function Get-ShipyardGateMarkerContract {
    # Full Gate is authoritative for the certification generation it expects.
    # Read that contract from the current on-disk SubspaceTools.ps1 instead of
    # hard-coding R2 here. This keeps intake compatible with later Shipyard
    # taxonomy generations (R3/R4/...) without making the bootstrap script stale.
    $toolPath = Join-Path $Root "SubspaceTools.ps1"
    $bestVersion = -1
    $bestRelative = $null

    if (Test-Path -LiteralPath $toolPath) {
        $toolText = Get-Content -LiteralPath $toolPath -Raw -ErrorAction SilentlyContinue
        if ($toolText) {
            $rx = [regex]'content\\derived\\greyoxide_shipyard_v07\\([^"''\r\n]+\.txt)'
            foreach ($m in $rx.Matches($toolText)) {
                $relative = $m.Groups[1].Value
                $name = [System.IO.Path]::GetFileName($relative)
                $versionMatch = [regex]::Match($name, 'R(\d+)', [System.Text.RegularExpressions.RegexOptions]::IgnoreCase)
                if ($versionMatch.Success) {
                    $version = [int]$versionMatch.Groups[1].Value
                    if ($version -gt $bestVersion) {
                        $bestVersion = $version
                        $bestRelative = $relative
                    }
                }
            }
        }
    }

    # If the utility does not expose a versioned marker path, preserve the
    # newest marker already present in the certified payload.
    if (-not $bestRelative -and (Test-Path -LiteralPath $CertifiedRoot)) {
        foreach ($file in Get-ChildItem -LiteralPath $CertifiedRoot -File -Filter '*.txt' -ErrorAction SilentlyContinue) {
            $versionMatch = [regex]::Match($file.Name, 'R(\d+)', [System.Text.RegularExpressions.RegexOptions]::IgnoreCase)
            if ($versionMatch.Success) {
                $version = [int]$versionMatch.Groups[1].Value
                if ($version -gt $bestVersion) {
                    $bestVersion = $version
                    $bestRelative = 'certified\' + $file.Name
                }
            }
        }
    }

    if (-not $bestRelative) {
        $bestVersion = 5
        $bestRelative = 'certified\SHIPYARD_CERTIFIED_R5.txt'
    }

    [pscustomobject]@{
        Version = $bestVersion
        RelativePath = $bestRelative
        FullPath = Join-Path $DerivedRoot $bestRelative
        FileName = [System.IO.Path]::GetFileName($bestRelative)
    }
}

function Find-ShipyardCertifier {
    $names = @("subspace_shipyard_certifier.exe", "subspace_shipyard_certifier")
    $candidates = @()
    foreach ($name in $names) {
        $candidates += (Join-Path $Root ("engine\build\" + $name))
        $candidates += (Join-Path $Root ("engine\build\Debug\" + $name))
        $candidates += (Join-Path $Root ("engine\build\Release\" + $name))
    }
    foreach ($candidate in $candidates) { if (Test-Path -LiteralPath $candidate) { return $candidate } }
    return $null
}

function Ensure-ShipyardCertifier([switch]$Rebuild) {
    if (-not $Rebuild) {
        $certifier = Find-ShipyardCertifier
        if ($certifier) { return $certifier }
    }

    $cmake = Get-Command cmake.exe -ErrorAction SilentlyContinue
    if (-not $cmake) { $cmake = Get-Command cmake -ErrorAction SilentlyContinue }
    if (-not $cmake) { throw "CMake is required to bootstrap the native Shipyard certifier." }
    $buildRoot = Join-Path $Root "engine\build"
    Write-Step "Configuring native Shipyard certification tool against the current source contract."
    & $cmake.Source -S (Join-Path $Root "engine") -B $buildRoot -DSUBSPACE_BUILD_TESTS=ON | Out-Host
    if ($LASTEXITCODE -ne 0) { throw "CMake configure failed while bootstrapping Shipyard certifier." }
    Write-Step "Building native Shipyard certification tool from current source."
    & $cmake.Source --build $buildRoot --target subspace_shipyard_certifier --config Debug | Out-Host
    if ($LASTEXITCODE -ne 0) { throw "Shipyard certifier build failed. The current source taxonomy contract must compile before content can be recertified." }
    $certifier = Find-ShipyardCertifier
    if (-not $certifier) { throw "Shipyard certifier build completed but executable was not found." }
    return $certifier
}

function Test-CertifiedPayload {
    $contract = Get-ShipyardGateMarkerContract
    $catalog = Join-Path $CertifiedRoot "certified_module_catalog.csv"
    if (-not (Test-Path -LiteralPath $contract.FullPath) -or -not (Test-Path -LiteralPath $catalog)) { return $false }

    $moduleCount = @(Get-ChildItem -LiteralPath $CertifiedModuleRoot -File -Filter *.obj -ErrorAction SilentlyContinue).Count
    if ($moduleCount -le 0) { return $false }

    $rows = @(Get-Content -LiteralPath $catalog -ErrorAction SilentlyContinue | Where-Object { -not [string]::IsNullOrWhiteSpace($_) })
    if ($rows.Count -le 1) { return $false }

    # Require the catalog to remain a real preserved-authored-module catalog.
    # Later generations may add columns, but these baseline identity/geometry
    # signals prevent a marker-only false positive.
    $header = $rows[0].ToLowerInvariant()
    if ($header -notmatch 'module' -or $header -notmatch 'preserv|authored|source|semantic|class') { return $false }

    return $true
}

function Invoke-ShipyardCertification([string]$RawModuleRoot) {
    $contract = Get-ShipyardGateMarkerContract

    # A stale executable is exactly how an R2/R4 split can survive. Whenever
    # recertification is required, rebuild the certifier from the current tree.
    $certifier = Ensure-ShipyardCertifier -Rebuild
    if (Test-Path -LiteralPath $CertifiedRoot) { Remove-Item -LiteralPath $CertifiedRoot -Recurse -Force }
    New-Item -ItemType Directory -Force -Path $CertifiedRoot | Out-Null

    Write-Step "Classifying authored Shipyard modules for Full Gate certification generation R$($contract.Version)."
    & $certifier --input $RawModuleRoot --output $CertifiedRoot | Out-Host
    if ($LASTEXITCODE -ne 0) { throw "Shipyard certification failed with exit code $LASTEXITCODE." }

    $catalog = Join-Path $CertifiedRoot "certified_module_catalog.csv"
    if (-not (Test-Path -LiteralPath $contract.FullPath) -or -not (Test-Path -LiteralPath $catalog)) {
        $produced = @(Get-ChildItem -LiteralPath $CertifiedRoot -File -Filter '*.txt' -ErrorAction SilentlyContinue |
            ForEach-Object { $_.Name }) -join ', '
        if ([string]::IsNullOrWhiteSpace($produced)) { $produced = '<none>' }
        throw "Current Shipyard certifier did not produce Full Gate marker '$($contract.FileName)'. Produced marker files: $produced. The native Shipyard taxonomy/certifier and Full Gate are out of contract."
    }

    $rows = @(Get-Content -LiteralPath $catalog -ErrorAction SilentlyContinue | Where-Object { -not [string]::IsNullOrWhiteSpace($_) })
    if ($rows.Count -le 1) { throw "Shipyard classification produced no certified preserved authored runtime modules." }

    Write-Pass "Certification generation R$($contract.Version) produced $($rows.Count - 1) catalog modules."
    return $rows.Count - 1
}

if ($Force) {
    Write-Step "Force refresh requested; clearing source/derived payloads."
    if (Test-Path -LiteralPath $SourceRoot) { Remove-Item -LiteralPath $SourceRoot -Recurse -Force }
    if (Test-Path -LiteralPath $ModuleRoot) { Remove-Item -LiteralPath $ModuleRoot -Recurse -Force }
    if (Test-Path -LiteralPath $CertifiedRoot) { Remove-Item -LiteralPath $CertifiedRoot -Recurse -Force }
    New-Item -ItemType Directory -Force -Path $SourceRoot,$ModuleRoot,$CertifiedRoot | Out-Null
}

# Full-gate runs should not depend on an old download archive once a certified
# local payload exists. -Force performs a complete intake refresh; -Recertify
# intentionally rebuilds certification from normalized/source content.
if (-not $Force -and -not $Recertify -and (Test-CertifiedPayload)) {
    $catalog = Join-Path $CertifiedRoot "certified_module_catalog.csv"
    $rows = @(Get-Content -LiteralPath $catalog | Where-Object { -not [string]::IsNullOrWhiteSpace($_) })
    Write-Pass "Using existing certified Shipyard payload ($($rows.Count - 1) catalog modules)."
    Write-Pass "Dependency gate is local/offline-safe; use -Force to refresh upstream content or -Recertify to rebuild certification."
    exit 0
}

# The raw normalized corpus is a governed local cache. If Full Gate moved to a
# newer certification generation (for example R4) while the cache still has an
# older marker, recertify locally before attempting any network intake.
if (-not $Force -and -not $Recertify -and -not (Test-CertifiedPayload)) {
    $contract = Get-ShipyardGateMarkerContract
    $raw = @(Get-ChildItem -LiteralPath $ModuleRoot -File -Filter *.obj -ErrorAction SilentlyContinue)
    if ($raw.Count -gt 0) {
        Write-Step "Local Shipyard corpus exists but does not satisfy Full Gate marker '$($contract.FileName)'; recertifying $($raw.Count) cached modules."
        $count = Invoke-ShipyardCertification $ModuleRoot
        if (-not (Test-CertifiedPayload)) {
            throw "Shipyard recertification completed but the payload still does not satisfy the current Full Gate contract."
        }
        Write-Pass "Recertified $count Shipyard modules from the local cache without downloading content."
        exit 0
    }
}

# A recertification pass can reuse normalized modules without touching the
# network. This is useful after taxonomy/certifier code changes.
if ($Recertify -and -not $Force) {
    $raw = @(Get-ChildItem -LiteralPath $ModuleRoot -File -Filter *.obj -ErrorAction SilentlyContinue)
    if ($raw.Count -gt 0) {
        $count = Invoke-ShipyardCertification $ModuleRoot
        Write-Pass "Recertified $count Shipyard modules from cached normalized source."
        exit 0
    }
    Write-Warn "Recertify requested but no normalized raw module cache exists; falling back to governed source intake."
}

$archiveUsed = $PrimaryArchive
try {
    Download-File $PrimaryUrl $PrimaryArchive
    if (-not (Extract-7z $PrimaryArchive $SourceRoot)) { throw "No available extractor could unpack the 7z archive." }
}
catch {
    Write-Warn "Extracted archive path failed: $($_.Exception.Message)"
    Write-Step "Falling back to the original CC0 Blender ZIP."
    Download-File $FallbackUrl $FallbackArchive
    if (Test-Path -LiteralPath $SourceRoot) { Remove-Item -LiteralPath $SourceRoot -Recurse -Force }
    New-Item -ItemType Directory -Force -Path $SourceRoot | Out-Null
    Expand-Archive -LiteralPath $FallbackArchive -DestinationPath $SourceRoot -Force
    $archiveUsed = $FallbackArchive
}

if (Test-Path -LiteralPath $archiveUsed) {
    $archiveSourceUrl = if ($archiveUsed -eq $PrimaryArchive) { $PrimaryUrl } else { $FallbackUrl }
    $archiveHash = Assert-OrCreateSourceLock -ArchivePath $archiveUsed -SourceUrl $archiveSourceUrl
    [System.IO.File]::WriteAllText((Join-Path $ThirdPartyRoot 'SOURCE_SHA256.txt'), $archiveHash + [Environment]::NewLine, $Utf8)
} else {
    Write-Warn "Source archive is not present after intake; preserving any existing SOURCE_SHA256.txt instead of failing the full gate."
}

if (Test-Path -LiteralPath $ModuleRoot) { Remove-Item -LiteralPath $ModuleRoot -Recurse -Force }
New-Item -ItemType Directory -Force -Path $ModuleRoot | Out-Null

$objFiles = @(Get-ChildItem -LiteralPath $SourceRoot -Recurse -File -Filter *.obj -ErrorAction SilentlyContinue)
$normalized = 0
$counts = @{}
if ($objFiles.Count -gt 0) {
    Write-Step "Normalizing $($objFiles.Count) extracted OBJ models."
    foreach ($file in $objFiles | Sort-Object FullName) {
        $class = Get-Class $file.BaseName
        if (-not $counts.ContainsKey($class)) { $counts[$class] = 0 }
        $counts[$class]++
        $name = 'shipyard_{0}_{1:d3}_{2}.obj' -f $class,$counts[$class],(Get-SafeName $file.BaseName)
        Convert-ObjCentered $file.FullName (Join-Path $ModuleRoot $name)
        $normalized++
    }
}
elseif (-not $SkipBlender) {
    $blender = Find-Blender
    if ($blender) {
        $normalizer = Join-Path $Root 'tools\blender\shipyard_v07_normalize.py'
        $sourceFiles = @(Get-ChildItem -LiteralPath $SourceRoot -Recurse -File -ErrorAction SilentlyContinue |
            Where-Object { $_.Extension.ToLowerInvariant() -in @('.blend','.fbx','.dae') } |
            Sort-Object FullName)
        foreach ($sourceFile in $sourceFiles) {
            Write-Step "Blender normalize: $($sourceFile.Name)"
            if ($sourceFile.Extension.ToLowerInvariant() -eq '.blend') {
                & $blender --background $sourceFile.FullName --python $normalizer -- --output $ModuleRoot --source-label $sourceFile.BaseName
            } else {
                & $blender --background --python $normalizer -- --input $sourceFile.FullName --output $ModuleRoot --source-label $sourceFile.BaseName
            }
            if ($LASTEXITCODE -ne 0) { throw "Blender normalization failed for $($sourceFile.FullName)" }
        }
        $normalized = @(Get-ChildItem -LiteralPath $ModuleRoot -File -Filter *.obj -ErrorAction SilentlyContinue).Count
    } else {
        Write-Warn "No OBJ payload and Blender was not found. Source is fetched, but no runtime derivatives were generated."
    }
}

$certifiedCount = Invoke-ShipyardCertification $ModuleRoot
$certifiedCatalogPath = Join-Path $CertifiedRoot 'certified_module_catalog.csv'
$catalogPath = Join-Path $DerivedRoot 'module_catalog.csv'
# Preserve the legacy catalog location as a compatibility mirror, but it now
# contains only Grade-A certified derivatives. Runtime never reads raw modules.
[System.IO.File]::WriteAllLines($catalogPath, [System.IO.File]::ReadAllLines($certifiedCatalogPath), $Utf8)

$provenance = [ordered]@{
    assetId = 'greyoxide_shipyard_v07'
    title = 'Shipyard v0.7'
    author = 'Greyoxide'
    sourceUrl = 'https://opengameart.org/content/shipyard-v07-extracted'
    license = 'CC0-1.0'
    licenseUrl = 'https://creativecommons.org/publicdomain/zero/1.0/'
    archive = [System.IO.Path]::GetFileName($archiveUsed)
    archiveSha256 = $archiveHash
    sourceReadOnly = $true
    normalizedRawModuleCount = $normalized
    certifiedGradeAModuleCount = $certifiedCount
    derivedPath = 'content/derived/greyoxide_shipyard_v07/certified/modules'
    certification = ('Shipyard Classification + Assembly Certification R' + (Get-ShipyardGateMarkerContract).Version)
}
$provenance | ConvertTo-Json -Depth 4 | Set-Content -LiteralPath (Join-Path $ThirdPartyRoot 'PROVENANCE.generated.json') -Encoding UTF8

if ($normalized -gt 0 -and $certifiedCount -gt 0) {
    $contract = Get-ShipyardGateMarkerContract
    $readyText = "Shipyard v0.7 Classification + Assembly R$($contract.Version) ready`nraw_normalized=$normalized`ngrade_a=$certifiedCount`nmarker=$($contract.FileName)`n"
    [System.IO.File]::WriteAllText((Join-Path $DerivedRoot 'SHIPYARD_READY.txt'), $readyText, $Utf8)
    Write-Pass "Shipyard v0.7 ready: $normalized raw normalized modules -> $certifiedCount Grade-A preserved authored runtime modules."
    Write-Host "Raw staging : $ModuleRoot"
    Write-Host "Runtime path: $CertifiedModuleRoot"
    Write-Host "Catalog     : $catalogPath"
    exit 0
}

throw "Shipyard source was fetched but current-generation certification produced no preserved authored runtime modules."

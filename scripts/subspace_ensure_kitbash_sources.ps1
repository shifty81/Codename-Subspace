param(
    [string]$Root = (Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)),
    [ValidateSet('AUTO','CACHE_ONLY','VERIFY_ONLY','OFFLINE')]
    [string]$Mode = 'AUTO'
)

$ErrorActionPreference = 'Stop'
$Root = [System.IO.Path]::GetFullPath($Root)
$ManifestPath = Join-Path $Root 'content\kitbash\sources.subspace_sources.json'
$CertificationPath = Join-Path $Root 'content\derived\KITBASH_SOURCE_CERTIFICATION.txt'

if (-not (Test-Path -LiteralPath $ManifestPath)) { throw "Kitbash source manifest missing: $ManifestPath" }
$manifest = Get-Content -LiteralPath $ManifestPath -Raw | ConvertFrom-Json
if ($manifest.schemaVersion -ne 1) { throw "Unsupported kitbash source manifest schemaVersion: $($manifest.schemaVersion)" }
if (-not $manifest.sources -or @($manifest.sources).Count -eq 0) { throw 'Kitbash source manifest contains no sources.' }

$results = New-Object System.Collections.Generic.List[string]
$results.Add('CODENAME SUBSPACE KITBASH SOURCE CERTIFICATION') | Out-Null
$results.Add(('mode=' + $Mode)) | Out-Null
$results.Add(('timestamp=' + (Get-Date).ToString('o'))) | Out-Null

foreach ($source in @($manifest.sources)) {
    foreach ($field in @('id','displayName','author','license','licenseUrl','sourceUrl','revision','handler')) {
        if ([string]::IsNullOrWhiteSpace([string]$source.$field)) { throw "Kitbash source entry is missing required field '$field'." }
    }
    if ($source.license -eq 'UNKNOWN' -or $source.license -eq 'NOASSERTION') {
        throw "Kitbash source '$($source.id)' does not have an approved explicit license."
    }

    switch ([string]$source.handler) {
        'shipyard_v07' {
            $ready = Join-Path $Root 'content\derived\greyoxide_shipyard_v07\SHIPYARD_READY.txt'
            $certified = Join-Path $Root 'content\derived\greyoxide_shipyard_v07\certified\SHIPYARD_CERTIFIED_R5.txt'
            $catalog = Join-Path $Root 'content\derived\greyoxide_shipyard_v07\certified\certified_module_catalog.csv'
            $isReady = (Test-Path -LiteralPath $ready) -and (Test-Path -LiteralPath $certified) -and (Test-Path -LiteralPath $catalog)

            if (-not $isReady) {
                if ($Mode -in @('VERIFY_ONLY','CACHE_ONLY','OFFLINE')) {
                    if ([bool]$source.required) { throw "Required kitbash source '$($source.id)' is not installed/certified and mode '$Mode' forbids network intake." }
                    $results.Add("$($source.id)=OPTIONAL_MISSING") | Out-Null
                    continue
                }
                $fetch = Join-Path $Root 'scripts\subspace_fetch_shipyard_v07.ps1'
                if (-not (Test-Path -LiteralPath $fetch)) { throw "Kitbash handler script missing: $fetch" }
                & $fetch -Root $Root
                if ($LASTEXITCODE -ne 0) { throw "Kitbash source handler returned exit code $LASTEXITCODE for '$($source.id)'." }
                $isReady = (Test-Path -LiteralPath $ready) -and (Test-Path -LiteralPath $certified) -and (Test-Path -LiteralPath $catalog)
            }

            if (-not $isReady) { throw "Kitbash source '$($source.id)' did not produce its governed certification markers." }
            $markerText = Get-Content -LiteralPath $certified -Raw -ErrorAction SilentlyContinue
            if ($markerText -notmatch 'policy=PRESERVE_AUTHORED_OBJECTS') { throw "Kitbash source '$($source.id)' certification policy is not PRESERVE_AUTHORED_OBJECTS." }
            $header = Get-Content -LiteralPath $catalog -First 1 -ErrorAction SilentlyContinue
            if ($header -notmatch 'preserved_authored_object') { throw "Kitbash source '$($source.id)' catalog is not the normalized authored-object authority." }
            $results.Add("$($source.id)=CERTIFIED revision=$($source.revision) license=$($source.license)") | Out-Null
        }
        default {
            if ([bool]$source.required) { throw "No approved intake handler is registered for required kitbash source '$($source.id)' ($($source.handler))." }
            $results.Add("$($source.id)=OPTIONAL_UNSUPPORTED_HANDLER") | Out-Null
        }
    }
}

$parent = Split-Path -Parent $CertificationPath
New-Item -ItemType Directory -Force -Path $parent | Out-Null
$results | Set-Content -LiteralPath $CertificationPath -Encoding UTF8
Write-Host '[PASS] Approved kitbash source manifest verified.' -ForegroundColor Green
$results | ForEach-Object { Write-Host $_ }

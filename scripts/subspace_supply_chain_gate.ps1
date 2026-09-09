param(
    [string]$Root = (Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)),
    [ValidateSet('AUTO','VERIFY_ONLY','CACHE_ONLY','OFFLINE')]
    [string]$Mode = 'AUTO'
)

$ErrorActionPreference = 'Stop'
$Root = [System.IO.Path]::GetFullPath($Root)
$Timestamp = Get-Date -Format 'yyyyMMdd-HHmmss'
$RegistryPath = Join-Path $Root 'content\sources\subspace_source_registry.json'
$PolicyPath = Join-Path $Root 'tools\dependencies\subspace_supply_chain_policy.json'
$ToolchainPath = Join-Path $Root 'tools\dependencies\subspace_asset_toolchain.json'
$KitbashPath = Join-Path $Root 'content\kitbash\sources.subspace_sources.json'
$OutputRoot = Join-Path $Root 'artifacts\gates\certifications\supply-chain'
$Output = Join-Path $OutputRoot "SUPPLY_CHAIN_CERTIFICATION_$Timestamp.txt"
$Latest = Join-Path $OutputRoot 'LATEST_SUPPLY_CHAIN_CERTIFICATION.txt'
$CacheRoot = Join-Path $Root 'content\cache\sources'
$LockRoot = Join-Path $Root 'content\cache\locks'
$QuarantineRoot = Join-Path $Root 'content\quarantine'
foreach($d in @($OutputRoot,$CacheRoot,$LockRoot,$QuarantineRoot)){New-Item -ItemType Directory -Force -Path $d | Out-Null}

function Require-File([string]$Path,[string]$Name){if(-not(Test-Path -LiteralPath $Path)){throw "$Name missing: $Path"}}
function Require-Text([object]$Value,[string]$Name){if([string]::IsNullOrWhiteSpace([string]$Value)){throw "Required source field is blank: $Name"}}
function Test-ImmutableRevision([string]$Revision){if([string]::IsNullOrWhiteSpace($Revision)){return $false};return $Revision -notmatch '(?i)^(latest|main|master|head|trunk|stable|current)$'}
function Test-Https([string]$Url){return $Url -match '^https://'}
function Add-Line([System.Collections.Generic.List[string]]$Lines,[string]$Text){$Lines.Add($Text)|Out-Null}

Require-File $RegistryPath 'Source registry'
Require-File $PolicyPath 'Supply-chain policy'
Require-File $ToolchainPath 'Asset toolchain manifest'
Require-File $KitbashPath 'Kitbash source manifest'
$registry=Get-Content -LiteralPath $RegistryPath -Raw|ConvertFrom-Json
$policy=Get-Content -LiteralPath $PolicyPath -Raw|ConvertFrom-Json
$toolchain=Get-Content -LiteralPath $ToolchainPath -Raw|ConvertFrom-Json
$kitbash=Get-Content -LiteralPath $KitbashPath -Raw|ConvertFrom-Json
if($registry.schemaVersion -ne 1){throw "Unsupported source registry schema: $($registry.schemaVersion)"}
if($policy.schemaVersion -ne 1){throw "Unsupported supply-chain policy schema: $($policy.schemaVersion)"}
if($toolchain.schema_version -ne 1){throw "Unsupported asset toolchain schema: $($toolchain.schema_version)"}
if($kitbash.schemaVersion -ne 1){throw "Unsupported kitbash source schema: $($kitbash.schemaVersion)"}

$lines=New-Object 'System.Collections.Generic.List[string]'
Add-Line $lines 'CODENAME SUBSPACE SUPPLY-CHAIN / SOURCE GATE'
Add-Line $lines ('timestamp='+[DateTime]::Now.ToString('o'))
Add-Line $lines ('mode='+$Mode)
Add-Line $lines ('root='+$Root)
Add-Line $lines ('policy='+$registry.policy)
Add-Line $lines ''
Add-Line $lines 'REGISTRY'
foreach($r in @($registry.registries)){
    Require-Text $r.id 'registry.id';Require-Text $r.manifest "registry.$($r.id).manifest"
    $manifest=Join-Path $Root ([string]$r.manifest)
    if([bool]$r.required){Require-File $manifest "Required registry '$($r.id)'"}
    Add-Line $lines ("$($r.id)=CERTIFIED manifest=$($r.manifest)")
}

Add-Line $lines ''
Add-Line $lines 'TOOLCHAIN DEPENDENCIES'
foreach($d in @($toolchain.dependencies)){
    foreach($f in @('name','version','revision','source','purpose')){Require-Text $d.$f "toolchain.$($d.name).$f"}
    $license = if(-not [string]::IsNullOrWhiteSpace([string]$d.license)){[string]$d.license}else{[string]$d.license_choice}
    Require-Text $license "toolchain.$($d.name).license"
    if(-not(Test-Https ([string]$d.source))){throw "Dependency '$($d.name)' source is not HTTPS: $($d.source)"}
    if(-not(Test-ImmutableRevision ([string]$d.revision))){throw "Dependency '$($d.name)' does not declare an immutable revision: $($d.revision)"}
    Add-Line $lines ("$($d.name)=DECLARED_PINNED version=$($d.version) revision=$($d.revision) license=$license runtime=$($d.runtime)")
}

Add-Line $lines ''
Add-Line $lines 'KITBASH / CONTENT SOURCES'
foreach($s in @($kitbash.sources)){
    foreach($f in @('id','displayName','author','license','licenseUrl','sourceUrl','revision','handler')){Require-Text $s.$f "kitbash.$($s.id).$f"}
    if(-not(Test-Https ([string]$s.sourceUrl))){throw "Kitbash source '$($s.id)' sourceUrl is not HTTPS."}
    if(-not(Test-Https ([string]$s.licenseUrl))){throw "Kitbash source '$($s.id)' licenseUrl is not HTTPS."}
    if(-not(Test-ImmutableRevision ([string]$s.revision))){throw "Kitbash source '$($s.id)' does not declare an immutable revision."}
    if(([string]$s.license) -match '(?i)UNKNOWN|NOASSERTION'){throw "Kitbash source '$($s.id)' has no approved license."}
    Add-Line $lines ("$($s.id)=APPROVED_PINNED revision=$($s.revision) license=$($s.license) handler=$($s.handler)")
}

# Existing governed intake handlers remain responsible for actual fetch,
# normalization, and domain certification. The Root Control Center is now the
# only caller in Full Gate, making network intake a first-class gate step.
$ensure=Join-Path $Root 'scripts\subspace_ensure_kitbash_sources.ps1'
Require-File $ensure 'Kitbash source gate'
& $ensure -Root $Root -Mode $Mode

# Verify generated provenance for any project-local executable/tool payloads
# already present. Future managed dependencies use the same provenance contract.
$provenanceFiles=@(Get-ChildItem -LiteralPath (Join-Path $Root 'tools\third_party') -Filter 'PROVENANCE.generated.json' -File -Recurse -ErrorAction SilentlyContinue)
foreach($p in $provenanceFiles){
    $j=Get-Content -LiteralPath $p.FullName -Raw|ConvertFrom-Json
    Require-Text $j.sourceUrl "provenance.$($p.Name).sourceUrl";Require-Text $j.sha256 "provenance.$($p.Name).sha256"
    if(-not(Test-Https ([string]$j.sourceUrl))){throw "Generated provenance contains non-HTTPS source: $($p.FullName)"}
    if(([string]$j.sha256) -notmatch '^[a-fA-F0-9]{64}$'){throw "Generated provenance contains invalid SHA-256: $($p.FullName)"}
    Add-Line $lines ("TOOL_PROVENANCE=VERIFIED path=$([System.IO.Path]::GetRelativePath($Root,$p.FullName)) sha256=$($j.sha256)")
}

Add-Line $lines ''
Add-Line $lines 'RESULT=PASS'
$lines|Set-Content -LiteralPath $Output -Encoding UTF8
@("Latest supply-chain certification: $Output","Timestamp: $Timestamp","Mode: $Mode","Result: PASS")|Set-Content -LiteralPath $Latest -Encoding UTF8
Write-Host '[PASS] Subspace supply-chain/source gate certified.' -ForegroundColor Green
Write-Host $Output

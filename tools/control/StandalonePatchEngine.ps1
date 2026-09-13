param(
    [Parameter(Mandatory=$true)][string]$Root,
    [Parameter(Mandatory=$true)][string]$Package,
    [switch]$DryRun
)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'

function Normalize-StandalonePathArgument([string]$Value,[string]$Label) {
    if ([string]::IsNullOrWhiteSpace($Value)) { throw "$Label path is empty." }
    $clean=$Value.Trim().Trim([char[]]@([char]34,[char]39))
    if ([string]::IsNullOrWhiteSpace($clean)) { throw "$Label path is empty after quote normalization." }
    return [IO.Path]::GetFullPath($clean)
}

$Root=Normalize-StandalonePathArgument $Root 'Repository root'
$Package=Normalize-StandalonePathArgument $Package 'Patch package'
if(-not(Test-Path -LiteralPath $Package -PathType Leaf)){throw "Patch package not found: $Package"}
if([IO.Path]::GetExtension($Package) -ine '.patch'){throw 'Standalone PCC accepts .patch transports only on this lane.'}

$updates=Join-Path $Root 'updates'
$applied=Join-Path $updates 'applied'
$failed=Join-Path $updates 'failed'
$staging=Join-Path $updates 'staging'
$backups=Join-Path $updates 'backups'
$transactions=Join-Path $updates 'transactions'
$logs=Join-Path $updates 'logs'
foreach($d in @($updates,$applied,$failed,$staging,$backups,$transactions,$logs)){New-Item -ItemType Directory -Force -Path $d|Out-Null}
$stamp=Get-Date -Format 'yyyyMMdd-HHmmss'
$stem=[IO.Path]::GetFileNameWithoutExtension($Package)
$stage=Join-Path $staging ($stamp+'_'+$stem)
$backup=Join-Path $backups ($stamp+'_'+$stem)
$log=Join-Path $logs ('standalone-patch-'+$stamp+'.log')
$journal=[System.Collections.Generic.List[object]]::new()

function Get-SubspaceRelativePath {
    param(
        [Parameter(Mandatory=$true)][string]$BasePath,
        [Parameter(Mandatory=$true)][string]$TargetPath
    )
    # Windows PowerShell 5.1 runs on .NET Framework, which does not expose
    # System.IO.Path.GetRelativePath().  All call sites here operate on files
    # already enumerated beneath BasePath, so a normalized containment +
    # substring implementation is both deterministic and PS5.1-safe.
    $baseFull = [System.IO.Path]::GetFullPath($BasePath).TrimEnd([char[]]@(
        [System.IO.Path]::DirectorySeparatorChar,
        [System.IO.Path]::AltDirectorySeparatorChar
    ))
    $targetFull = [System.IO.Path]::GetFullPath($TargetPath)
    if ($targetFull.Equals($baseFull, [System.StringComparison]::OrdinalIgnoreCase)) { return '' }
    $prefix = $baseFull + [System.IO.Path]::DirectorySeparatorChar
    if (-not $targetFull.StartsWith($prefix, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Path is outside the expected base path. Base='$baseFull' Target='$targetFull'"
    }
    return $targetFull.Substring($prefix.Length)
}

function Log([string]$Message,[string]$Level='INFO'){
    $line='[{0}] [{1}] {2}' -f (Get-Date -Format 'yyyy-MM-dd HH:mm:ss'),$Level,$Message
    $color=switch($Level){'PASS'{'Green'}'FAIL'{'Red'}'WARN'{'Yellow'}'STEP'{'Cyan'}default{'Gray'}}
    Write-Host $line -ForegroundColor $color
    Add-Content -LiteralPath $log -Value $line
}
function Hash([string]$Path){(Get-FileHash -Algorithm SHA256 -LiteralPath $Path).Hash.ToLowerInvariant()}
function ManifestProp($Object,[string]$Name){
    if($null -eq $Object){return $null}
    $property=$Object.PSObject.Properties[$Name]
    if($null -eq $property){return $null}
    return $property.Value
}
function Safe([string]$Relative){
    if([string]::IsNullOrWhiteSpace($Relative)){throw 'Blank patch path.'}
    if([IO.Path]::IsPathRooted($Relative) -or $Relative.StartsWith('..')){throw "Unsafe patch path: $Relative"}
    $n=$Relative.Replace('/','\')
    if($n -match '(^|\\)\.git(\\|$)'){throw "Patch may not write .git: $Relative"}
    if($n -match '(^|\\)updates\\inbox(\\|$)'){throw "Patch may not write updates\\inbox: $Relative"}
}
function GitHead {
    if(-not(Test-Path -LiteralPath (Join-Path $Root '.git') -PathType Container)){return ''}
    Push-Location $Root
    try{$v=(& git rev-parse HEAD 2>$null|Select-Object -First 1);if($LASTEXITCODE -ne 0){return ''};return ([string]$v).Trim()}
    finally{Pop-Location}
}
function Restore {
    Log 'Rolling back patch transaction.' 'WARN'
    for($i=$journal.Count-1;$i-ge 0;$i--){
        $e=$journal[$i];$dest=Join-Path $Root $e.path
        if([bool]$e.existed){
            $src=Join-Path $backup $e.path
            if(-not(Test-Path -LiteralPath $src -PathType Leaf)){throw "Rollback backup missing: $src"}
            New-Item -ItemType Directory -Force -Path (Split-Path -Parent $dest)|Out-Null
            Copy-Item -LiteralPath $src -Destination $dest -Force
        } elseif(Test-Path -LiteralPath $dest){Remove-Item -LiteralPath $dest -Force}
    }
    Log 'Rollback completed.' 'PASS'
}

New-Item -ItemType Directory -Force -Path $stage,$backup|Out-Null
try{
    Log "Inspecting patch: $([IO.Path]::GetFileName($Package))" 'STEP'
    $sig=Get-Content -LiteralPath $Package -Encoding Byte -TotalCount 4
    if($sig.Count -lt 2 -or $sig[0] -ne 0x50 -or $sig[1] -ne 0x4B){
        throw 'This standalone lane expects a ZIP-container .patch with PATCH_MANIFEST.json. Plain unified-diff patches are intentionally not auto-applied.'
    }
    Add-Type -AssemblyName System.IO.Compression.FileSystem
    [IO.Compression.ZipFile]::ExtractToDirectory($Package,$stage)
    $manifestPath=Join-Path $stage 'PATCH_MANIFEST.json'
    if(-not(Test-Path -LiteralPath $manifestPath -PathType Leaf)){throw 'PATCH_MANIFEST.json missing from .patch package root.'}
    $m=Get-Content -LiteralPath $manifestPath -Raw|ConvertFrom-Json
    $schemaVersion=ManifestProp $m 'schemaVersion'
    if([int]$schemaVersion -ne 1){throw "Unsupported patch schemaVersion: $schemaVersion"}
    $patchSchema=[string](ManifestProp $m 'schema')
    if(-not [string]::IsNullOrWhiteSpace($patchSchema) -and $patchSchema -ne 'forge.patch.v1'){throw "Unsupported patch schema: $patchSchema"}

    # Resolve target identity before source applicability. A drifted tree is a
    # PRECONDITION_CONFLICT for a known project, never "no compatible project".
    $targetProject=''
    $target=ManifestProp $m 'target'
    $legacyProjectId=[string](ManifestProp $m 'projectId')
    if($null -ne $target){$targetProject=[string](ManifestProp $target 'projectId')}
    if([string]::IsNullOrWhiteSpace($targetProject)){$targetProject=$legacyProjectId}
    if(-not [string]::IsNullOrWhiteSpace($targetProject) -and $targetProject -ne 'codename-subspace'){
        throw "TARGET_MISMATCH: patch targets '$targetProject', current project is 'codename-subspace'."
    }

    $requires=ManifestProp $m 'requires'
    $requiredSchema=if($null -ne $requires){[string](ManifestProp $requires 'projectSchema')}else{''}
    if(-not [string]::IsNullOrWhiteSpace($requiredSchema)){
        $contractPath=Join-Path $Root 'project.control.json'
        if(-not(Test-Path -LiteralPath $contractPath -PathType Leaf)){throw 'TARGET=codename-subspace / CONTRACT_MISSING: project.control.json not found.'}
        $contract=Get-Content -LiteralPath $contractPath -Raw|ConvertFrom-Json
        if([string]$contract.schema -ne $requiredSchema){throw "TARGET=codename-subspace / CONTRACT_MISMATCH: expected $requiredSchema, current $($contract.schema)."}
    }

    $head=GitHead
    $baseline=ManifestProp $m 'baseline'
    $preconditions=ManifestProp $m 'preconditions'
    $expected=if($null -ne $baseline){[string](ManifestProp $baseline 'gitCommit')}else{''}
    if([string]::IsNullOrWhiteSpace($expected) -and $null -ne $preconditions){$expected=[string](ManifestProp $preconditions 'gitCommit')}
    $expectedPrefix=if($null -ne $preconditions){[string](ManifestProp $preconditions 'gitCommitPrefix')}else{''}
    $preconditionConflict=$false
    $preconditionLabel=''
    if(-not [string]::IsNullOrWhiteSpace($expected) -and $head -and $head -ne $expected){$preconditionConflict=$true;$preconditionLabel="expected Git $expected; current $head"}
    elseif(-not [string]::IsNullOrWhiteSpace($expectedPrefix) -and $head -and -not $head.StartsWith($expectedPrefix,[System.StringComparison]::OrdinalIgnoreCase)){$preconditionConflict=$true;$preconditionLabel="expected Git prefix $expectedPrefix; current $head"}
    if($preconditionConflict){
        # Permit idempotent re-check below; otherwise this is a hard lineage mismatch.
        $same=$true
        foreach($e in @($m.files)){$dest=Join-Path $Root ([string]$e.path);if(-not(Test-Path -LiteralPath $dest -PathType Leaf) -or (Hash $dest) -ne ([string]$e.sha256).ToLowerInvariant()){$same=$false;break}}
        if($same){foreach($r in @($m.remove)){if(Test-Path -LiteralPath (Join-Path $Root ([string]$r))){$same=$false;break}}}
        if(-not $same){throw "TARGET=codename-subspace / PRECONDITION_CONFLICT: $preconditionLabel. No files were changed."}
    }
    $expectedPaths=@{}
    foreach($e in @($m.files)){
        $rel=[string]$e.path;Safe $rel;$key=$rel.Replace('/','\').ToLowerInvariant()
        if($expectedPaths.ContainsKey($key)){throw "Duplicate manifest path: $rel"}
        $src=Join-Path $stage $rel
        if(-not(Test-Path -LiteralPath $src -PathType Leaf)){throw "Manifest payload missing: $rel"}
        if((Get-Item -LiteralPath $src).Length -ne [int64]$e.bytes){throw "Manifest byte mismatch: $rel"}
        if((Hash $src) -ne ([string]$e.sha256).ToLowerInvariant()){throw "Manifest SHA mismatch: $rel"}
        $expectedPaths[$key]=$true
    }
    foreach($f in @(Get-ChildItem -LiteralPath $stage -Recurse -File -Force)){
        if($f.FullName -eq $manifestPath){continue}
        $rel=(Get-SubspaceRelativePath -BasePath $stage -TargetPath $f.FullName);Safe $rel;$key=$rel.Replace('/','\').ToLowerInvariant()
        if(-not $expectedPaths.ContainsKey($key)){throw "Unmanifested payload file: $rel"}
    }
    foreach($r in @($m.remove)){Safe ([string]$r)}

    $already=$true
    foreach($e in @($m.files)){$dest=Join-Path $Root ([string]$e.path);if(-not(Test-Path -LiteralPath $dest -PathType Leaf) -or (Hash $dest) -ne ([string]$e.sha256).ToLowerInvariant()){$already=$false;break}}
    if($already){foreach($r in @($m.remove)){if(Test-Path -LiteralPath (Join-Path $Root ([string]$r))){$already=$false;break}}}
    if($already){
        Log 'Patch contents already match the project; treating transport as idempotently applied.' 'PASS'
    } elseif($DryRun){
        Log 'Dry-run validation passed; no files changed.' 'PASS'
    } else {
        foreach($e in @($m.files)){
            $rel=[string]$e.path;$src=Join-Path $stage $rel;$dest=Join-Path $Root $rel;$exists=Test-Path -LiteralPath $dest -PathType Leaf
            $journal.Add([pscustomobject]@{path=$rel;existed=$exists})|Out-Null
            if($exists){$bk=Join-Path $backup $rel;New-Item -ItemType Directory -Force -Path (Split-Path -Parent $bk)|Out-Null;Copy-Item -LiteralPath $dest -Destination $bk -Force}
            New-Item -ItemType Directory -Force -Path (Split-Path -Parent $dest)|Out-Null
            Copy-Item -LiteralPath $src -Destination $dest -Force
            if((Hash $dest) -ne ([string]$e.sha256).ToLowerInvariant()){throw "Post-write verification failed: $rel"}
        }
        foreach($raw in @($m.remove)){
            $rel=[string]$raw;$dest=Join-Path $Root $rel;$exists=Test-Path -LiteralPath $dest -PathType Leaf
            $journal.Add([pscustomobject]@{path=$rel;existed=$exists})|Out-Null
            if($exists){$bk=Join-Path $backup $rel;New-Item -ItemType Directory -Force -Path (Split-Path -Parent $bk)|Out-Null;Copy-Item -LiteralPath $dest -Destination $bk -Force;Remove-Item -LiteralPath $dest -Force}
        }
        Log 'Patch overlay and removals verified.' 'PASS'
    }
    $receipt=[ordered]@{schema='subspace.standalone-patch-receipt.v2';patchSchema=if(-not [string]::IsNullOrWhiteSpace($patchSchema)){$patchSchema}else{'forge.patch.v1-legacy-compatible'};timestamp=(Get-Date).ToString('o');patchId=[string]$m.patchId;targetProjectId=if($targetProject){$targetProject}else{'codename-subspace'};package=[IO.Path]::GetFileName($Package);packageSha256=(Hash $Package);baselineGit=$expected;result=if($DryRun){'DRY-RUN-PASS'}elseif($already){'ALREADY-APPLIED'}else{'PASS'};files=@($m.files).Count;removals=@($m.remove).Count;authority='SUBSPACE-INTERNAL-PCC';policy='discover-queue-explicit-apply-full-gate-never-consumes'}
    $receiptPath=Join-Path $transactions ($stamp+'_'+$stem+'.json');$receipt|ConvertTo-Json -Depth 8|Set-Content -LiteralPath $receiptPath -Encoding UTF8
    if(-not $DryRun){$archive=Join-Path $applied ($stamp+'_'+[IO.Path]::GetFileName($Package));Move-Item -LiteralPath $Package -Destination $archive -Force;Log "Patch archived: $archive" 'PASS'}
    Remove-Item -LiteralPath $stage -Recurse -Force -ErrorAction SilentlyContinue
    exit 0
}
catch{
    $message=$_.Exception.Message;Log $message 'FAIL'
    if(-not $DryRun -and $journal.Count -gt 0){try{Restore}catch{Log ('Rollback failed: '+$_.Exception.Message) 'FAIL'}}
    Remove-Item -LiteralPath $stage -Recurse -Force -ErrorAction SilentlyContinue
    if(-not $DryRun -and (Test-Path -LiteralPath $Package -PathType Leaf)){
        try{$dest=Join-Path $failed ($stamp+'_'+[IO.Path]::GetFileName($Package));Move-Item -LiteralPath $Package -Destination $dest -Force;Log "Failed transport archived: $dest" 'WARN'}catch{}
    }
    exit 1
}

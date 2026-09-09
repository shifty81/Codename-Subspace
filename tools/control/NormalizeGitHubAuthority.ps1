param(
    [string]$Root=(Get-Location).Path,
    [ValidateSet('Audit','Prepare','Publish')][string]$Mode='Audit',
    [string]$RemoteUrl='https://github.com/shifty81/Codename-Subspace.git',
    [string]$PublishConfirmation=''
)
$ErrorActionPreference='Stop'
Set-StrictMode -Version 2
$Root=[System.IO.Path]::GetFullPath($Root)
. (Join-Path $PSScriptRoot 'ControlCenterCommon.ps1')
$treeStageModule = Join-Path $PSScriptRoot 'UniversalTreeStage.psm1'
if (-not (Test-Path -LiteralPath $treeStageModule)) { throw "Universal ProjectOps TreeStage module missing: $treeStageModule" }
Import-Module $treeStageModule -Force -ErrorAction Stop

$gitCmd=Get-Command git -ErrorAction SilentlyContinue
if (-not $gitCmd) { throw 'Git is required for repository authority normalization.' }
$git=$gitCmd.Source

$state=Join-Path $Root '.subspace\repository-authority'
$artifacts=Join-Path $Root 'artifacts\repository\normalization'
$temp=Join-Path $Root 'artifacts\temp\repository-normalization'
foreach($p in @($state,$artifacts,$temp)){New-Item -ItemType Directory -Force -Path $p | Out-Null}

# Source/repository staging policy is owned by ProjectOps source authority.
# No parallel rootFiles/rootDirs/exclusion table is permitted here.

function Run-Git {
    param([string[]]$GitArgs,[string]$Working=$Root,[switch]$AllowFailure)
    Push-Location $Working
    $previousEap=$ErrorActionPreference
    $nativePreferenceExists=Test-Path Variable:\PSNativeCommandUseErrorActionPreference
    if($nativePreferenceExists){$previousNativePreference=$PSNativeCommandUseErrorActionPreference}
    try {
        # Git legitimately writes clone/fetch/progress text to stderr. Capture that
        # text without allowing PowerShell 7 to promote it to a terminating error.
        $ErrorActionPreference='Continue'
        if($nativePreferenceExists){$PSNativeCommandUseErrorActionPreference=$false}
        $output=@(& $git @GitArgs 2>&1 | ForEach-Object {[string]$_})
        $code=$LASTEXITCODE
    }
    finally {
        $ErrorActionPreference=$previousEap
        if($nativePreferenceExists){$PSNativeCommandUseErrorActionPreference=$previousNativePreference}
        Pop-Location
    }
    if ($code -ne 0 -and -not $AllowFailure) {
        throw ("git {0} failed ({1}): {2}" -f ($GitArgs -join ' '),$code,($output -join "`n"))
    }
    return [pscustomobject]@{Code=$code;Output=$output}
}

function Get-RemoteMainSha {
    $r=Run-Git -GitArgs @('ls-remote',$RemoteUrl,'refs/heads/main') -AllowFailure
    if ($r.Code -ne 0 -or @($r.Output).Count -eq 0) { return '' }
    $line=[string]$r.Output[0]
    if ($line -match '^([0-9a-fA-F]{40})\s+') { return $Matches[1].ToLowerInvariant() }
    return ''
}

function New-NormalizedStage {
    param([string]$Stamp)
    $stageRoot=Join-Path $temp $Stamp
    $repo=Join-Path $stageRoot 'Codename-Subspace'
    if(Test-Path -LiteralPath $stageRoot){Remove-Item -LiteralPath $stageRoot -Recurse -Force}
    New-Item -ItemType Directory -Force -Path $repo | Out-Null

    $result=Invoke-ProjectOpsGovernedTreeStage -Root $Root -Destination $repo -Quiet
    if(-not $result.Success){throw 'ProjectOps governed repository stage failed.'}

    $files=@(Get-ChildItem -LiteralPath $repo -Recurse -File -Force)
    $rows=[System.Collections.Generic.List[object]]::new()
    foreach($file in $files) {
        $rel=Get-ProjectOpsRelativePath -Root $repo -Path $file.FullName
        $rows.Add([pscustomobject]@{
            path=$rel.Replace('\','/')
            bytes=$file.Length
            sha256=(Get-FileHash -Algorithm SHA256 -LiteralPath $file.FullName).Hash.ToLowerInvariant()
        })
    }
    $manifest=[pscustomobject]@{
        schemaVersion=2
        timestamp=(Get-Date).ToString('o')
        remote=$RemoteUrl
        branch='main'
        sourceAuthorityId=$result.SourceAuthorityId
        sourceFingerprint=$result.SourceFingerprint
        sourcePathCount=$result.SourcePathCount
        fileCount=$rows.Count
        files=$rows.ToArray()
    }
    $manifestPath=Join-Path $artifacts ("REPOSITORY_MANIFEST_"+$Stamp+".json")
    $manifest | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $manifestPath -Encoding UTF8
    return $repo
}

function Write-Preview {
    param([string]$Repo,[string]$Stamp,[string]$RemoteSha)
    $files=@(Get-ChildItem -LiteralPath $Repo -Recurse -File -Force)
    $top=@(Get-ChildItem -LiteralPath $Repo -Force | Sort-Object Name | ForEach-Object {$_.Name})
    $report=Join-Path $artifacts ("REPOSITORY_NORMALIZATION_PREVIEW_"+$Stamp+".txt")
    $lines=@(
        'CODENAME SUBSPACE REPOSITORY NORMALIZATION PREVIEW',
        '========================================================================',
        "Timestamp: $Stamp",
        "Root: $Root",
        "Remote: $RemoteUrl",
        "Remote main before publish: $RemoteSha",
        "Normalized files: $($files.Count)",
        '',
        'Top-level normalized main:'
    )
    $lines += @($top | ForEach-Object {"  $_"})
    $lines += @(
        '',
        'Explicitly excluded from normalized main:',
        '  AvorionLike/ retired C# prototype',
        '  legacy root solutions/intake/hash debris (migrated to artifacts/legacy)',
        '  root patch/hash handoff debris',
        '  build/artifact/update/.subspace state',
        '  generated/imported/derived content',
        '  ungoverned binary/third-party payloads'
    )
    $lines | Set-Content -LiteralPath $report -Encoding UTF8

    $zip=Join-Path $artifacts ("Codename_Subspace_GitHubNormalized_"+$Stamp+".zip")
    if(Test-Path -LiteralPath $zip){Remove-Item -LiteralPath $zip -Force}
    Compress-Archive -Path (Join-Path $Repo '*') -DestinationPath $zip -CompressionLevel Optimal
    Write-Host "[PASS] Normalized repository preview: $zip" -ForegroundColor Green
    Write-Host "Report: $report"
    return [pscustomobject]@{Report=$report;Zip=$zip}
}

function Ensure-LocalAuthorityBootstrap {
    param([string]$Stamp)

    if (-not (Test-Path -LiteralPath (Join-Path $Root '.git'))) {
        $init=Run-Git -GitArgs @('init','-b','main') -AllowFailure
        if($init.Code -ne 0) {
            Run-Git -GitArgs @('init') | Out-Null
            Run-Git -GitArgs @('branch','-M','main') | Out-Null
        }
        Write-Host '[PASS] Local Git repository initialized on main.' -ForegroundColor Green
    }

    $origin=Run-Git -GitArgs @('remote','get-url','origin') -AllowFailure
    if($origin.Code -ne 0) {
        Run-Git -GitArgs @('remote','add','origin',$RemoteUrl) | Out-Null
        Write-Host "[PASS] origin configured: $RemoteUrl" -ForegroundColor Green
    } else {
        $existing=([string]$origin.Output[0]).Trim()
        if($existing -ne $RemoteUrl) {
            throw "origin is already configured to '$existing', expected '$RemoteUrl'. Correct it deliberately before normalization."
        }
    }

    # An unborn repository cannot produce a content-sensitive Git fingerprint
    # for untracked files. Create one local-only authority baseline commit so
    # the next Full Gate can certify real file content before remote publish.
    $head=Run-Git -GitArgs @('rev-parse','--verify','HEAD') -AllowFailure
    if($head.Code -ne 0) {
        $nameResult=Run-Git -GitArgs @('config','user.name') -AllowFailure
        $emailResult=Run-Git -GitArgs @('config','user.email') -AllowFailure
        $name=($nameResult.Output -join '').Trim()
        $email=($emailResult.Output -join '').Trim()
        if([string]::IsNullOrWhiteSpace($name)){throw 'Git user.name is not configured. Configure Git identity, then rerun Prepare.'}
        if([string]::IsNullOrWhiteSpace($email)){throw 'Git user.email is not configured. Configure Git identity, then rerun Prepare.'}

        $sourceSnapshot=Get-ProjectSourceAuthoritySnapshot -Root $Root -IncludePaths
        $batch=[System.Collections.Generic.List[string]]::new()
        foreach($entry in @($sourceSnapshot.files)) {
            $batch.Add(([string]$entry.path).Replace('/','\')) | Out-Null
            if($batch.Count -ge 64) {
                Run-Git -GitArgs (@('add','-f','--') + $batch.ToArray()) | Out-Null
                $batch.Clear()
            }
        }
        if($batch.Count -gt 0) {
            Run-Git -GitArgs (@('add','-f','--') + $batch.ToArray()) | Out-Null
        }
        Run-Git -GitArgs @('commit','-m',("Local repository authority prepare "+$Stamp)) | Out-Null
        Write-Host '[PASS] Local-only repository authority baseline committed. Nothing was pushed.' -ForegroundColor Green
    }
}

function Get-RemoteAudit {
    param([string]$Stamp)
    $sha=Get-RemoteMainSha
    $auditPath=Join-Path $artifacts ("REPOSITORY_REMOTE_AUDIT_"+$Stamp+".txt")
    $flags=[System.Collections.Generic.List[string]]::new()
    $probe=Join-Path $temp ("remote-audit-"+$Stamp)
    try {
        if(Test-Path -LiteralPath $probe){Remove-Item -LiteralPath $probe -Recurse -Force}
        $clone=Run-Git -Working $temp -GitArgs @('clone','--depth','1','--filter=blob:none','--no-checkout',$RemoteUrl,$probe) -AllowFailure
        if($clone.Code -eq 0) {
            $tree=Run-Git -Working $probe -GitArgs @('ls-tree','--name-only','HEAD')
            $top=@($tree.Output | ForEach-Object {[string]$_})
            foreach($legacy in @('AvorionLike','Legacy-CSharpPrototype.sln','121212.md.txt','Assets')) {
                if($top -contains $legacy){$flags.Add($legacy)}
            }
            $remoteReadme=Run-Git -Working $probe -GitArgs @('show','HEAD:README.md') -AllowFailure
            if($remoteReadme.Code -eq 0 -and (($remoteReadme.Output -join "`n") -match 'Your project description here|# Project Title')) {
                $flags.Add('placeholder README')
            }
        } else {
            $flags.Add('remote tree audit unavailable')
        }
    } finally {
        if(Test-Path -LiteralPath $probe){Remove-Item -LiteralPath $probe -Recurse -Force -ErrorAction SilentlyContinue}
    }
    $lines=@(
        'CODENAME SUBSPACE REMOTE AUTHORITY AUDIT',
        '========================================================================',
        "Timestamp: $Stamp",
        "Remote: $RemoteUrl",
        "Remote main SHA: $sha",
        "Legacy/stale authority flags: $($flags.Count)"
    )
    $lines += @($flags | ForEach-Object {"  - $_"})
    $lines += @('','This audit is read-only.')
    $lines | Set-Content -LiteralPath $auditPath -Encoding UTF8

    Write-Host "Remote main SHA: $sha"
    if($flags.Count -gt 0) {
        Write-Host 'Stale/legacy authority markers:' -ForegroundColor Yellow
        foreach($flag in $flags){Write-Host "  - $flag" -ForegroundColor Yellow}
    } else {
        Write-Host '[PASS] No known stale root markers detected.' -ForegroundColor Green
    }
    Write-Host "Audit: $auditPath"
    return [pscustomobject]@{Sha=$sha;Audit=$auditPath;Flags=$flags.ToArray()}
}

$stamp=Get-Date -Format 'yyyyMMdd-HHmmss'
$audit=Get-RemoteAudit -Stamp $stamp
if($Mode -eq 'Audit'){return}

Ensure-LocalAuthorityBootstrap -Stamp $stamp
$repo=New-NormalizedStage -Stamp $stamp
$preview=Write-Preview -Repo $repo -Stamp $stamp -RemoteSha $audit.Sha

if($Mode -eq 'Prepare') {
    @(
        'Repository authority PREPARED.',
        'No remote writes were performed.',
        'Run Full Quality Gate now so the GREEN record captures the initialized Git fingerprint and current authority content.',
        'After GREEN, use Publish last GREEN normalized main.'
    ) | Set-Content -LiteralPath (Join-Path $state 'LATEST_PREPARE.txt') -Encoding UTF8
    Write-Host ''
    Write-Host '[PASS] Repository authority prepared. No GitHub changes were made.' -ForegroundColor Green
    Write-Host 'NEXT: run Full Quality Gate, then Publish last GREEN normalized main.'
    return
}

$greenPath=Join-Path $Root '.subspace\last-green-quality-gate.json'
if(-not (Test-Path -LiteralPath $greenPath)){throw 'No GREEN Full Quality Gate exists. Run Full Quality Gate after Prepare.'}
$green=Get-Content -LiteralPath $greenPath -Raw | ConvertFrom-Json
if([string]$green.result -ne 'PASS' -or [string]$green.kind -ne 'FULL'){throw 'Latest promotion authority is not a GREEN Full Quality Gate.'}
if(-not $green.gitFingerprint){throw 'GREEN gate has no initialized-Git fingerprint. Run Prepare, then Full Quality Gate, then Publish.'}
if(-not $green.sourceFingerprint){throw 'GREEN gate predates ProjectOps source authority. Run Full Quality Gate again before publishing.'}
$currentFingerprint=Get-CertifiableGitFingerprint -Root $Root
if($currentFingerprint -ne [string]$green.gitFingerprint){
    throw 'Certifiable Git state changed since the GREEN gate. Run Full Quality Gate again before publishing.'
}
$currentSource=Get-ProjectSourceAuthoritySnapshot -Root $Root
if([string]$currentSource.fingerprint -ne [string]$green.sourceFingerprint -or [int]$currentSource.pathCount -ne [int]$green.sourcePathCount){
    throw 'Governed source bytes/path set changed since the GREEN gate. Run Full Quality Gate again before publishing.'
}

$publishAudit=Get-RemoteAudit -Stamp (Get-Date -Format 'yyyyMMdd-HHmmss')

# Interactivity belongs to the root Control Center. This lower-level authority
# must be deterministic and safe when invoked through logged child processes.
if($PublishConfirmation -cne 'PUBLISH'){
    throw 'Publish authorization missing. Use the interactive root Control Center and enter exact PUBLISH confirmation.'
}

Write-Host ("[PASS] Explicit publication authorization received for remote main {0}" -f $publishAudit.Sha) -ForegroundColor Green

Run-Git -Working $repo -GitArgs @('init','-b','main') | Out-Null
Run-Git -Working $repo -GitArgs @('remote','add','origin',$RemoteUrl) | Out-Null

if(-not [string]::IsNullOrWhiteSpace($publishAudit.Sha)) {
    Run-Git -Working $repo -GitArgs @('fetch','--depth','1','origin','main') | Out-Null
    $archiveBranch=('archive/pre-native-normalization-'+$stamp)
    Run-Git -Working $repo -GitArgs @('branch',$archiveBranch,$publishAudit.Sha) | Out-Null
    Run-Git -Working $repo -GitArgs @('push','origin',($archiveBranch+':refs/heads/'+$archiveBranch)) | Out-Null
    Write-Host "[PASS] Historical remote main preserved: $archiveBranch" -ForegroundColor Green
}

Run-Git -Working $repo -GitArgs @('add','-A') | Out-Null

$nameResult=Run-Git -Working $Root -GitArgs @('config','user.name') -AllowFailure
$emailResult=Run-Git -Working $Root -GitArgs @('config','user.email') -AllowFailure
$name=($nameResult.Output -join '').Trim()
$email=($emailResult.Output -join '').Trim()
if([string]::IsNullOrWhiteSpace($name)){throw 'Git user.name is not configured. Configure Git identity and rerun Publish.'}
if([string]::IsNullOrWhiteSpace($email)){throw 'Git user.email is not configured. Configure Git identity and rerun Publish.'}
Run-Git -Working $repo -GitArgs @('config','user.name',$name) | Out-Null
Run-Git -Working $repo -GitArgs @('config','user.email',$email) | Out-Null

Run-Git -Working $repo -GitArgs @('commit','-m',("Normalize Codename Subspace native authority - "+$green.gateId)) | Out-Null

if([string]::IsNullOrWhiteSpace($publishAudit.Sha)) {
    Run-Git -Working $repo -GitArgs @('push','-u','origin','main') | Out-Null
} else {
    if([string]$publishAudit.Sha -notmatch '^[0-9a-fA-F]{40}$'){
        throw ("Remote main SHA is not a valid 40-character Git object id: {0}" -f [string]$publishAudit.Sha)
    }

    # IMPORTANT: build the force-with-lease option as one scalar before placing
    # it in the Git argument array. PowerShell's array-expression parsing can
    # otherwise split inline string concatenation into separate native argv
    # entries, which previously produced:
    #   --force-with-lease=refs/heads/main: <sha> origin main:main
    # and caused Git to treat the SHA as the remote.
    $leaseArg=[string]::Concat('--force-with-lease=refs/heads/main:',[string]$publishAudit.Sha)
    $mainRefspec='refs/heads/main:refs/heads/main'

    if($leaseArg -ne ("--force-with-lease=refs/heads/main:{0}" -f [string]$publishAudit.Sha)){
        throw 'Internal repository publication lease argument construction failed.'
    }

    Write-Host ("[INFO] Publishing normalized main with guarded lease on {0}" -f $publishAudit.Sha)
    Run-Git -Working $repo -GitArgs @('push',$leaseArg,'origin',$mainRefspec) | Out-Null
}
$newHead=([string](Run-Git -Working $repo -GitArgs @('rev-parse','HEAD')).Output[0]).Trim()

Run-Git -GitArgs @('fetch','origin','main') | Out-Null
Run-Git -GitArgs @('checkout','-B','main','origin/main') | Out-Null
Run-Git -GitArgs @('branch','--set-upstream-to=origin/main','main') | Out-Null

$result=Join-Path $state 'LATEST_PUBLISH.txt'
@(
    'Codename Subspace normalized GitHub publication PASS',
    "Timestamp: $stamp",
    "Remote: $RemoteUrl",
    "Archived old main: $($publishAudit.Sha)",
    "New main: $newHead",
    "GREEN gate: $($green.gateId)",
    "Preview: $($preview.Zip)"
) | Set-Content -LiteralPath $result -Encoding UTF8

Write-Host ''
Write-Host '========================================================================' -ForegroundColor Green
Write-Host ' CODENAME SUBSPACE NORMALIZED GITHUB MAIN - PUBLISHED' -ForegroundColor Green
Write-Host '========================================================================' -ForegroundColor Green
Write-Host " Old main : $($publishAudit.Sha)"
Write-Host " New main : $newHead"
Write-Host " Gate     : $($green.gateId)"
Write-Host " Record   : $result"
Write-Host '========================================================================' -ForegroundColor Green

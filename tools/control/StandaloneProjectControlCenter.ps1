param([string]$Root='')
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'

function Resolve-StandaloneRepositoryRoot([string]$Candidate) {
    if ([string]::IsNullOrWhiteSpace($Candidate)) {
        return [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
    }
    # powershell.exe/native launchers can preserve an outer quote when a quoted
    # Windows path ends in a backslash. Strip only outer quote characters.
    $clean=$Candidate.Trim().Trim([char[]]@([char]34,[char]39))
    if ([string]::IsNullOrWhiteSpace($clean)) { throw 'Repository root resolved to an empty path.' }
    return [IO.Path]::GetFullPath($clean)
}

$Root=Resolve-StandaloneRepositoryRoot $Root
$RootTools=Join-Path $Root 'SubspaceTools.ps1'
$PatchEngine=Join-Path $Root 'tools\control\StandalonePatchEngine.ps1'
$BranchManager=Join-Path $Root 'tools\control\GitBranchManager.ps1'
$State=Join-Path $Root '.subspace'
$BranchSwitchState=Join-Path $State 'control-center\branch-switch-state.json'
$ContractPath=Join-Path $Root 'project.control.json'
if(-not(Test-Path -LiteralPath $RootTools -PathType Leaf)){throw "Existing project PCC authority missing: $RootTools"}
if(-not(Test-Path -LiteralPath $PatchEngine -PathType Leaf)){throw "Standalone patch engine missing: $PatchEngine"}
if(-not(Test-Path -LiteralPath $BranchManager -PathType Leaf)){throw "Git branch manager missing: $BranchManager"}

function P([string]$Text,[string]$Color='Gray'){Write-Host $Text -ForegroundColor $Color}
function Get-ControlContract {
    if(-not(Test-Path -LiteralPath $ContractPath -PathType Leaf)){throw "Project contract missing: $ContractPath"}
    try{return (Get-Content -LiteralPath $ContractPath -Raw|ConvertFrom-Json)}catch{throw "Project contract is invalid JSON: $($_.Exception.Message)"}
}
function Assert-StartupIntakePolicy {
    $contract=Get-ControlContract
    if([string]$contract.schema -ne 'forge.project.v1'){throw 'project.control.json must remain forge.project.v1.'}
    $updates=$contract.updates
    if($null -eq $updates){throw 'project.control.json is missing the standardized updates contract.'}
    if([string]$updates.patchSchema -ne 'forge.patch.v1'){throw 'updates.patchSchema must be forge.patch.v1.'}
    if([string]$updates.rootPattern -ne '*.patch'){throw 'updates.rootPattern must be *.patch.'}
    if($updates.startupScan -ne $true -or $updates.explicitApprovalRequired -ne $true){throw 'Startup patch scan and explicit approval must remain enabled.'}
    if($updates.autoApply -ne $false -or $updates.fullGateAutoApply -ne $false){throw 'Patch discovery/Full Gate may not silently apply updates.'}
    if($updates.transactional -ne $true -or $updates.rollbackOnFailure -ne $true){throw 'Transactional apply + rollback must remain enabled.'}
    return $contract
}
function Get-CurrentBranch {
    if(-not(Test-Path -LiteralPath (Join-Path $Root '.git'))){return ''}
    Push-Location $Root
    try{return ([string](& git branch --show-current 2>$null|Select-Object -First 1)).Trim()}
    finally{Pop-Location}
}
function Get-BranchSwitchState {
    if(-not(Test-Path -LiteralPath $BranchSwitchState -PathType Leaf)){return $null}
    try{return (Get-Content -LiteralPath $BranchSwitchState -Raw|ConvertFrom-Json)}catch{return $null}
}
function Header {
    try{Clear-Host}catch{}
    $branch='Not initialized';$dirty='';$head='';$upstream=''
    if(Test-Path -LiteralPath (Join-Path $Root '.git') -PathType Container){
        Push-Location $Root
        try{
            $branch=([string](& git branch --show-current 2>$null|Select-Object -First 1)).Trim()
            if([string]::IsNullOrWhiteSpace($branch)){$branch='DETACHED HEAD'}
            $head=([string](& git rev-parse --short HEAD 2>$null|Select-Object -First 1)).Trim()
            $s=@(& git status --short 2>$null)
            $dirty=if($s.Count -gt 0){' / Modified'}else{' / Clean'}
            $upstream=([string](& git rev-parse --abbrev-ref --symbolic-full-name '@{u}' 2>$null|Select-Object -First 1)).Trim()
        }finally{Pop-Location}
    }
    $gate='None'
    $gateColor='Gray'
    $g=Join-Path $State 'last-green-quality-gate.json'
    if(Test-Path -LiteralPath $g){
        try{$x=Get-Content -LiteralPath $g -Raw|ConvertFrom-Json;$gate="$($x.result) $($x.gateId)";$gateColor='Green'}catch{$gate='Unreadable';$gateColor='Yellow'}
    }
    else{
        $branchState=Get-BranchSwitchState
        if($null -ne $branchState -and $branchState.requiresFullGate -eq $true){
            $current=Get-CurrentBranch
            if([string]$branchState.toBranch -eq $current){
                $gate=("REQUIRED after branch switch to {0}" -f $current)
                $gateColor='Yellow'
            }
        }
    }
    $rootPatches=@(Get-ChildItem -LiteralPath $Root -Filter '*.patch' -File -ErrorAction SilentlyContinue|Sort-Object Name)
    $legacyRootDrops=@(PendingLegacyRootDrops)
    P '========================================================================' DarkGray
    P ' CODENAME SUBSPACE PROJECT CONTROL CENTER' Cyan
    P '========================================================================' DarkGray
    P (" Repository : {0}" -f $Root)
    P (" Git        : {0}{1}{2}" -f $branch,$dirty,$(if($head){" @ $head"}else{''}))
    if(-not [string]::IsNullOrWhiteSpace($upstream)){P (" Tracking   : {0}" -f $upstream) DarkGray}
    P (" Gate       : {0}" -f $gate) $gateColor
    P (" Patches    : {0} canonical .patch / {1} legacy ZIP handoff(s)" -f $rootPatches.Count,$legacyRootDrops.Count) $(if(($rootPatches.Count+$legacyRootDrops.Count) -gt 0){'Yellow'}else{'Green'})
    P (" PowerShell : {0}" -f $PSVersionTable.PSVersion.ToString()) DarkGray
    P ' Authority  : Project-owned PCC / forge.project.v1 (Forge-compatible provider)' DarkGray
    P ' Intake     : startup scan + explicit approval; Full Gate never auto-applies' DarkGray
    P ' Branches   : safe switch/create/toggle; no auto-stash/reset/force operations' DarkGray
    P '------------------------------------------------------------------------' DarkGray
}
function RunRoot([string[]]$Arguments){
    # IMPORTANT: callers assign the result of RunRoot to $rc. Native process
    # stdout is part of PowerShell's success-output stream, so without an
    # explicit sink every line from the child process is captured into $rc and
    # the PCC appears frozen until the child exits. Stream every child line to
    # the host and return only the integer process exit code.
    $displayArgs=($Arguments -join ' ')
    P ("[RUN] SubspaceTools.ps1 {0}" -f $displayArgs) Cyan
    P ("[INFO] Live child output follows. Project logs remain authoritative.") DarkGray
    & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $RootTools @Arguments 2>&1 | ForEach-Object {
        Write-Host ([string]$_)
    }
    $rc=$LASTEXITCODE
    if($null -eq $rc){$rc=0}
    P ("[INFO] Child process exited with code {0}." -f $rc) $(if($rc -eq 0){'Green'}else{'Red'})
    return [int]$rc
}
function PendingPatches {return @(Get-ChildItem -LiteralPath $Root -Filter '*.patch' -File -ErrorAction SilentlyContinue|Sort-Object Name)}
function Get-HandoffName($Item) {
    if($null -eq $Item){return ''}
    $prop=$Item.PSObject.Properties['Name']
    if($null -ne $prop -and $null -ne $prop.Value){return [string]$prop.Value}
    return [IO.Path]::GetFileName([string]$Item)
}
function Get-HandoffFullName($Item) {
    if($null -eq $Item){return ''}
    $prop=$Item.PSObject.Properties['FullName']
    if($null -ne $prop -and $null -ne $prop.Value){return [string]$prop.Value}
    $raw=[string]$Item
    if([IO.Path]::IsPathRooted($raw)){return $raw}
    return (Join-Path $Root $raw)
}
function Test-LegacyZipPatchManifest {
    param([System.IO.FileInfo]$File)
    if($null -eq $File -or -not (Test-Path -LiteralPath $File.FullName -PathType Leaf)){return $false}
    try{
        Add-Type -AssemblyName System.IO.Compression.FileSystem -ErrorAction SilentlyContinue
        $archive=[System.IO.Compression.ZipFile]::OpenRead($File.FullName)
        try{
            foreach($entry in $archive.Entries){
                if([string]::Equals($entry.FullName,'PATCH_MANIFEST.json',[System.StringComparison]::OrdinalIgnoreCase)){return $true}
            }
            return $false
        }
        finally{$archive.Dispose()}
    }
    catch{
        P ("[WARN] Legacy ZIP could not be inspected and will not be treated as an update handoff: {0}" -f $File.Name) Yellow
        return $false
    }
}
function PendingLegacyRootDrops {
    return @(Get-ChildItem -LiteralPath $Root -Filter '*.zip' -File -ErrorAction SilentlyContinue|Where-Object {
        # A legacy ZIP is an update only when BOTH its legacy naming pattern and
        # its archive contents identify it as a patch. This prevents cumulative
        # source rollups/snapshots/debug artifacts from blocking certification.
        if($_.Name -match '(?i)FullSource|BuildRollup|DebugBundle|SourceRollup|SourceSnapshot|CumulativeSource|CompleteSource|Full_Source|sha256'){return $false}
        if($_.Name -notmatch '(?i)^(Subspace|Codename_Subspace)_(Pass|Patch|Hotfix|Root[_-]?Drop(?:[_-]?Patch)?|Update|Rollup).*\.zip$'){return $false}
        return (Test-LegacyZipPatchManifest -File $_)
    }|Sort-Object Name)
}
function Get-RepositoryPassNumber {
    $maxPass=0
    foreach($dir in @((Join-Path $Root 'tools\control\static-gates'),(Join-Path $Root 'engine\tests'))){
        if(-not(Test-Path -LiteralPath $dir -PathType Container)){continue}
        foreach($file in @(Get-ChildItem -LiteralPath $dir -File -ErrorAction SilentlyContinue)){
            if($file.Name -match '(?i)^pass(?<pass>\d+)'){$n=[int]$Matches.pass;if($n -gt $maxPass){$maxPass=$n}}
        }
    }
    $statusPath=Join-Path $Root 'docs\CURRENT_STATUS.md'
    if(Test-Path -LiteralPath $statusPath -PathType Leaf){
        $statusText=Get-Content -LiteralPath $statusPath -Raw -ErrorAction SilentlyContinue
        foreach($match in [regex]::Matches([string]$statusText,'(?i)Pass(?<pass>\d+)')){$n=[int]$match.Groups['pass'].Value;if($n -gt $maxPass){$maxPass=$n}}
    }
    return $maxPass
}
function Archive-SupersededLegacyRootDrops {
    $legacy=@(PendingLegacyRootDrops)
    if($legacy.Count -eq 0){return}
    $currentPass=Get-RepositoryPassNumber
    if($currentPass -le 0){return}
    $superseded=Join-Path $Root 'updates\superseded'
    New-Item -ItemType Directory -Force -Path $superseded|Out-Null
    $stamp=Get-Date -Format 'yyyyMMdd-HHmmss'
    foreach($patch in $legacy){
        $patchName=Get-HandoffName $patch
        $patchFullName=Get-HandoffFullName $patch
        if([string]::IsNullOrWhiteSpace($patchName) -or [string]::IsNullOrWhiteSpace($patchFullName)){continue}
        $patchPass=0
        if($patchName -match '(?i)Pass(?<pass>\d+)'){$patchPass=[int]$Matches.pass}
        if($patchPass -gt 0 -and $patchPass -lt $currentPass){
            $dest=Join-Path $superseded ($stamp+'_'+$patchName)
            Move-Item -LiteralPath $patchFullName -Destination $dest -Force
            P ("[ARCHIVE] Superseded legacy root-drop moved out of certification path: {0} (Pass{1} < Pass{2})" -f $patchName,$patchPass,$currentPass) DarkGray
        }
    }
}
function StartupPatchScan {
    [void](Assert-StartupIntakePolicy)
    Archive-SupersededLegacyRootDrops
    $patches=@(PendingPatches)
    $legacy=@(PendingLegacyRootDrops)
    if($patches.Count -eq 0 -and $legacy.Count -eq 0){return}
    Header
    P ' PATCH FOUND AT PROJECT STARTUP' Yellow
    P ''
    if($legacy.Count -gt 0){
        P ' Legacy ZIP handoff(s) remain and cannot be silently consumed by the standalone PCC:' Yellow
        foreach($item in $legacy){P ("  - {0}" -f (Get-HandoffName $item)) Yellow}
        P ' Move/quarantine these legacy handoffs or use the Advanced PCC only after review. Full Gate remains blocked.' Yellow
        P ''
    }
    foreach($patch in $patches){
        $patchName=Get-HandoffName $patch;$patchFullName=Get-HandoffFullName $patch
        P (" Patch: {0}" -f $patchName) Yellow
        $answer=Read-Host ' Apply this patch now? [Y/N]'
        if($answer -notmatch '^(?i)y(?:es)?$'){P ' [SKIP] Patch left in project root for a later PCC launch.' Yellow;continue}
        $wrapper=$PSCommandPath;$before=(Get-FileHash -Algorithm SHA256 -LiteralPath $wrapper).Hash
        & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $PatchEngine -Root $Root -Package $patchFullName
        if($LASTEXITCODE -ne 0){
            P ("[FAIL] Patch apply failed and was archived: {0}" -f $patchName) Red
            $latestPatchLog=Get-ChildItem -LiteralPath (Join-Path $Root 'updates\logs') -Filter 'standalone-patch-*.log' -File -ErrorAction SilentlyContinue|Sort-Object LastWriteTime -Descending|Select-Object -First 1
            if($latestPatchLog){
                P ("[ERROR LOG] {0}" -f $latestPatchLog.FullName) Yellow
                P '------------------------------------------------------------------------' DarkGray
                Get-Content -LiteralPath $latestPatchLog.FullName -Tail 12 -ErrorAction SilentlyContinue|ForEach-Object{Write-Host ([string]$_)}
                P '------------------------------------------------------------------------' DarkGray
            }
            P '[INFO] No project payload is trusted as applied after a failed transaction. PCC remains open.' Yellow
            Read-Host 'Press Enter to acknowledge this patch failure and continue'|Out-Null
            continue
        }
        $after=(Get-FileHash -Algorithm SHA256 -LiteralPath $wrapper).Hash
        if($before -ne $after){P '[INFO] PCC changed during patch apply; restarting updated PCC.' Cyan;& powershell.exe -NoProfile -ExecutionPolicy Bypass -File $wrapper -Root $Root;exit $LASTEXITCODE}
    }
}
function RequireNoPendingPatch {
    Archive-SupersededLegacyRootDrops
    $pending=@(PendingPatches)
    $legacy=@(PendingLegacyRootDrops)
    if($pending.Count -gt 0 -or $legacy.Count -gt 0){
        $names=@()
        foreach($item in $pending){$name=Get-HandoffName $item;if(-not [string]::IsNullOrWhiteSpace($name)){$names+=$name}}
        foreach($item in $legacy){$name=Get-HandoffName $item;if(-not [string]::IsNullOrWhiteSpace($name)){$names+=$name}}
        if($names.Count -eq 0){$names=@('<unresolved update handoff>')}
        throw "Root update handoff(s) are still pending. Restart/apply canonical .patch files and review/quarantine legacy ZIPs before certifying GREEN. Pending: $($names -join ', ')"
    }
}
function RequireBranchGateFresh {
    $branchState=Get-BranchSwitchState
    if($null -eq $branchState){return}
    if($branchState.requiresFullGate -ne $true){return}
    $current=Get-CurrentBranch
    if([string]$branchState.toBranch -eq $current){
        throw "Branch '$current' points at a different source HEAD than the previously certified branch. Run option 1 Full Quality Gate before committing/pushing."
    }
}
function FullGate {
    RequireNoPendingPatch
    P ''
    P '[STEP] Starting authoritative Full Quality Gate...' Cyan
    P ('[INFO] Repository: '+$Root) DarkGray
    P '[INFO] Build/test output will stream below in this console.' DarkGray
    $rc=RunRoot @('-Action','full-gate','-NoPause')
    if($rc -ne 0){throw "Full Quality Gate failed with exit code $rc."}
    if(Test-Path -LiteralPath $BranchSwitchState -PathType Leaf){
        Remove-Item -LiteralPath $BranchSwitchState -Force
        P '[PASS] Branch-switch certification requirement cleared by this GREEN Full Gate.' Green
    }
    P '[PASS] Full Quality Gate returned GREEN. Test the game before option 2.' Green
}
function CommitPushGreen {
    RequireNoPendingPatch
    RequireBranchGateFresh
    P '[STEP] COMMIT: verify or create the exact certified GREEN commit.' Cyan
    $rc=RunRoot @('-Action','git-commit-green','-NoPause');if($rc -ne 0){throw "Certified GREEN commit verification/creation failed with exit code $rc."}
    P '[PASS] COMMIT: certified GREEN source is committed.' Green
    P '[STEP] PUSH + REMOTE VERIFY: publish current branch and verify origin matches local HEAD.' Cyan
    $rc=RunRoot @('-Action','git-push','-NoPause');if($rc -ne 0){throw "GitHub push/remote verification failed with exit code $rc. Nothing was force-pushed."}
    P '[PASS] REMOTE VERIFY: certified GREEN source is committed and present on origin.' Green
}
function PatchStatus {
    Archive-SupersededLegacyRootDrops
    Header;$p=@(PendingPatches);$legacy=@(PendingLegacyRootDrops);if($p.Count -eq 0 -and $legacy.Count -eq 0){P 'No root update handoffs pending.' Green}else{if($p.Count -gt 0){P 'Pending canonical root patches:' Yellow;foreach($x in $p){P ('  - '+(Get-HandoffName $x)) Yellow}};if($legacy.Count -gt 0){P 'Pending legacy ZIP handoffs requiring review:' Yellow;foreach($x in $legacy){P ('  - '+(Get-HandoffName $x)) Yellow}}}
    $tx=Join-Path $Root 'updates\transactions';if(Test-Path -LiteralPath $tx){$latest=Get-ChildItem -LiteralPath $tx -Filter '*.json' -File -ErrorAction SilentlyContinue|Sort-Object LastWriteTime -Descending|Select-Object -First 1;if($latest){P '';P ('Latest patch receipt: '+$latest.FullName) Cyan;Get-Content -LiteralPath $latest.FullName|ForEach-Object{Write-Host $_}}}
    $failed=Join-Path $Root 'updates\failed';if(Test-Path -LiteralPath $failed){$latestFailed=Get-ChildItem -LiteralPath $failed -Filter '*.patch' -File -ErrorAction SilentlyContinue|Sort-Object LastWriteTime -Descending|Select-Object -First 1;if($latestFailed){P '';P ('Latest failed patch evidence: '+$latestFailed.FullName) Yellow}}
}
function BranchControl {
    RequireNoPendingPatch
    & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $BranchManager -Root $Root
    $rc=$LASTEXITCODE
    if($null -eq $rc){$rc=0}
    if($rc -ne 0){throw "Git branch manager exited with code $rc."}
}

StartupPatchScan
while($true){
    Header
    P ' 1. FULL QUALITY GATE / CERTIFY GREEN' White
    P ' 2. COMMIT + PUSH CURRENT GREEN' White
    P ''
    P ' 3. Run & play'
    P ' 4. Patch status / receipts'
    P ' 5. Project status / health'
    P ' 6. Package debug bundle'
    P ' 7. Advanced / original PCC'
    P ' 8. Branches / switch / create / toggle' Cyan
    P ' 0. Exit'
    P ''
    $choice=Read-Host 'Select'
    try{
        switch($choice){
            '1'{FullGate}
            '2'{CommitPushGreen}
            '3'{$rc=RunRoot @('-Action','run-loop','-NoPause');if($rc -ne 0){throw "Game exited $rc"}}
            '4'{PatchStatus}
            '5'{$rc=RunRoot @('-Action','health','-NoPause');if($rc -ne 0){$rc=RunRoot @('-Action','status','-NoPause')}}
            '6'{$rc=RunRoot @('-Action','debug-bundle','-NoPause');if($rc -ne 0){throw "Debug bundle failed with exit code $rc"}}
            '7'{& powershell.exe -NoProfile -ExecutionPolicy Bypass -File $RootTools -Action menu; if($LASTEXITCODE -ne 0){throw "Original PCC exited $LASTEXITCODE"}}
            '8'{BranchControl}
            '0'{exit 0}
            default{P 'Unknown option.' Yellow}
        }
    }catch{P ('[FAIL] '+$_.Exception.Message) Red}
    if($choice -ne '0'){P '';Read-Host 'Press Enter to continue'|Out-Null}
}

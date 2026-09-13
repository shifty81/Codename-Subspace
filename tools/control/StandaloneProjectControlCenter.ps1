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
$State=Join-Path $Root '.subspace'
$ContractPath=Join-Path $Root 'project.control.json'
if(-not(Test-Path -LiteralPath $RootTools -PathType Leaf)){throw "Existing project PCC authority missing: $RootTools"}
if(-not(Test-Path -LiteralPath $PatchEngine -PathType Leaf)){throw "Standalone patch engine missing: $PatchEngine"}

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
function Header {
    try{Clear-Host}catch{}
    $branch='Not initialized';$dirty='';$head=''
    if(Test-Path -LiteralPath (Join-Path $Root '.git') -PathType Container){
        Push-Location $Root
        try{$branch=([string](& git branch --show-current 2>$null|Select-Object -First 1)).Trim();$head=([string](& git rev-parse --short HEAD 2>$null|Select-Object -First 1)).Trim();$s=@(& git status --short 2>$null);$dirty=if($s.Count -gt 0){' / Modified'}else{' / Clean'}}finally{Pop-Location}
    }
    $gate='None';$g=Join-Path $State 'last-green-quality-gate.json';if(Test-Path -LiteralPath $g){try{$x=Get-Content -LiteralPath $g -Raw|ConvertFrom-Json;$gate="$($x.result) $($x.gateId)"}catch{$gate='Unreadable'}}
    $rootPatches=@(Get-ChildItem -LiteralPath $Root -Filter '*.patch' -File -ErrorAction SilentlyContinue|Sort-Object Name)
    P '========================================================================' DarkGray
    P ' CODENAME SUBSPACE PROJECT CONTROL CENTER' Cyan
    P '========================================================================' DarkGray
    P (" Repository : {0}" -f $Root)
    P (" Git        : {0}{1}{2}" -f $branch,$dirty,$(if($head){" @ $head"}else{''}))
    P (" Gate       : {0}" -f $gate)
    P (" Patches    : {0} root .patch file(s)" -f $rootPatches.Count) $(if($rootPatches.Count -gt 0){'Yellow'}else{'Green'})
    P (" PowerShell : {0}" -f $PSVersionTable.PSVersion.ToString()) DarkGray
    P ' Authority  : Project-owned PCC / forge.project.v1 (Forge-compatible provider)' DarkGray
    P ' Intake     : startup scan + explicit approval; Full Gate never auto-applies' DarkGray
    P '------------------------------------------------------------------------' DarkGray
}
function RunRoot([string[]]$Arguments){
    # IMPORTANT: callers assign the result of RunRoot to $rc.  Native process
    # stdout is part of PowerShell's success-output stream, so without an
    # explicit sink every line from the child process is captured into $rc and
    # the PCC appears frozen until the child exits.  Stream every child line to
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
function StartupPatchScan {
    [void](Assert-StartupIntakePolicy)
    $patches=@(PendingPatches);if($patches.Count -eq 0){return}
    Header
    P ' PATCH FOUND AT PROJECT STARTUP' Yellow
    P ''
    foreach($patch in $patches){
        P (" Patch: {0}" -f $patch.Name) Yellow
        $answer=Read-Host ' Apply this patch now? [Y/N]'
        if($answer -notmatch '^(?i)y(?:es)?$'){P ' [SKIP] Patch left in project root for a later PCC launch.' Yellow;continue}
        $wrapper=$PSCommandPath;$before=(Get-FileHash -Algorithm SHA256 -LiteralPath $wrapper).Hash
        & powershell.exe -NoProfile -ExecutionPolicy Bypass -File $PatchEngine -Root $Root -Package $patch.FullName
        if($LASTEXITCODE -ne 0){
            P ("[FAIL] Patch apply failed and was archived: {0}" -f $patch.Name) Red
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
    $pending=@(PendingPatches)
    if($pending.Count -gt 0){throw "Root .patch file(s) are still pending. Restart the PCC and apply/clear them before certifying GREEN. Pending: $($pending.Name -join ', ')"}
}
function FullGate {
    RequireNoPendingPatch
    P ''
    P '[STEP] Starting authoritative Full Quality Gate...' Cyan
    P ('[INFO] Repository: '+$Root) DarkGray
    P '[INFO] Build/test output will stream below in this console.' DarkGray
    $rc=RunRoot @('-Action','full-gate','-NoPause')
    if($rc -ne 0){throw "Full Quality Gate failed with exit code $rc."}
    P '[PASS] Full Quality Gate returned GREEN. Test the game before option 2.' Green
}
function CommitPushGreen {
    RequireNoPendingPatch
    $rc=RunRoot @('-Action','git-commit-green','-NoPause');if($rc -ne 0){throw "Certified GREEN commit failed with exit code $rc."}
    $rc=RunRoot @('-Action','git-push','-NoPause');if($rc -ne 0){throw "GitHub push failed with exit code $rc. Nothing was force-pushed."}
    P '[PASS] Certified GREEN source committed and pushed through the project-owned PCC.' Green
}
function PatchStatus {
    Header;$p=@(PendingPatches);if($p.Count -eq 0){P 'No root .patch files pending.' Green}else{P 'Pending root patches:' Yellow;foreach($x in $p){P ('  - '+$x.Name) Yellow}}
    $tx=Join-Path $Root 'updates\transactions';if(Test-Path -LiteralPath $tx){$latest=Get-ChildItem -LiteralPath $tx -Filter '*.json' -File -ErrorAction SilentlyContinue|Sort-Object LastWriteTime -Descending|Select-Object -First 1;if($latest){P '';P ('Latest patch receipt: '+$latest.FullName) Cyan;Get-Content -LiteralPath $latest.FullName|ForEach-Object{Write-Host $_}}}
    $failed=Join-Path $Root 'updates\failed';if(Test-Path -LiteralPath $failed){$latestFailed=Get-ChildItem -LiteralPath $failed -Filter '*.patch' -File -ErrorAction SilentlyContinue|Sort-Object LastWriteTime -Descending|Select-Object -First 1;if($latestFailed){P '';P ('Latest failed patch evidence: '+$latestFailed.FullName) Yellow}}
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
            '0'{exit 0}
            default{P 'Unknown option.' Yellow}
        }
    }catch{P ('[FAIL] '+$_.Exception.Message) Red}
    if($choice -ne '0'){P '';Read-Host 'Press Enter to continue'|Out-Null}
}

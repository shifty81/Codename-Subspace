param([string]$Root='')
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'

function Resolve-RepositoryRoot([string]$Candidate) {
    if([string]::IsNullOrWhiteSpace($Candidate)){
        return [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
    }
    $clean=$Candidate.Trim().Trim([char[]]@([char]34,[char]39))
    if([string]::IsNullOrWhiteSpace($clean)){throw 'Repository root resolved to an empty path.'}
    return [IO.Path]::GetFullPath($clean)
}

$Root=Resolve-RepositoryRoot $Root
$StateRoot=Join-Path $Root '.subspace\control-center'
$BranchStatePath=Join-Path $StateRoot 'branch-switch-state.json'
$GatePath=Join-Path $Root '.subspace\last-green-quality-gate.json'
$GateArchiveRoot=Join-Path $StateRoot 'branch-gates'
New-Item -ItemType Directory -Force -Path $StateRoot,$GateArchiveRoot|Out-Null

function P([string]$Text,[string]$Color='Gray'){Write-Host $Text -ForegroundColor $Color}

function Invoke-Git {
    param(
        [Parameter(Mandatory=$true)][string[]]$Arguments,
        [switch]$AllowFailure
    )
    Push-Location $Root
    try{
        $lines=@(& git @Arguments 2>&1|ForEach-Object{[string]$_})
        $code=$LASTEXITCODE
        if($null -eq $code){$code=0}
    }finally{Pop-Location}
    if(-not $AllowFailure -and $code -ne 0){
        throw ("git {0} failed ({1}): {2}" -f ($Arguments -join ' '),$code,($lines -join [Environment]::NewLine))
    }
    return [pscustomobject]@{Code=[int]$code;Lines=$lines}
}

function Assert-GitRepository {
    if(-not(Get-Command git -ErrorAction SilentlyContinue)){throw 'Git was not found on PATH.'}
    $r=Invoke-Git -Arguments @('rev-parse','--is-inside-work-tree') -AllowFailure
    if($r.Code -ne 0 -or (($r.Lines -join '').Trim() -ne 'true')){throw "Not a Git working tree: $Root"}
}

function Current-Branch {
    $r=Invoke-Git -Arguments @('branch','--show-current')
    return (($r.Lines|Select-Object -First 1) -as [string]).Trim()
}

function Current-Head {
    $r=Invoke-Git -Arguments @('rev-parse','HEAD')
    return (($r.Lines|Select-Object -First 1) -as [string]).Trim()
}

function Assert-NoPendingPatch {
    $patches=@(Get-ChildItem -LiteralPath $Root -Filter '*.patch' -File -ErrorAction SilentlyContinue)
    if($patches.Count -gt 0){
        throw "Branch switching is blocked while root .patch file(s) are pending: $($patches.Name -join ', ')"
    }
}

function Assert-WorkingTreeClean {
    $r=Invoke-Git -Arguments @('status','--porcelain=v1','--untracked-files=all')
    if($r.Lines.Count -gt 0){
        P '[BLOCKED] Working tree is not clean. Branch switching never auto-stashes, resets, discards, or overwrites changes.' Yellow
        foreach($line in $r.Lines){P ("  $line") Yellow}
        throw 'Commit, intentionally stash, or otherwise resolve these changes before switching branches.'
    }
}

function Assert-ValidBranchName([string]$Name) {
    if([string]::IsNullOrWhiteSpace($Name)){throw 'Branch name may not be empty.'}
    $r=Invoke-Git -Arguments @('check-ref-format','--branch',$Name) -AllowFailure
    if($r.Code -ne 0){throw "Invalid Git branch name: $Name"}
}

function Test-LocalBranch([string]$Name) {
    $r=Invoke-Git -Arguments @('show-ref','--verify','--quiet',("refs/heads/{0}" -f $Name)) -AllowFailure
    return ($r.Code -eq 0)
}

function Target-HasModernPcc([string]$Ref) {
    $spec=("{0}:tools/control/GitBranchManager.ps1" -f $Ref)
    $r=Invoke-Git -Arguments @('cat-file','-e',$spec) -AllowFailure
    return ($r.Code -eq 0)
}

function Archive-CurrentGate([string]$Branch,[string]$Head) {
    if(-not(Test-Path -LiteralPath $GatePath -PathType Leaf)){return ''}
    $safe=[regex]::Replace($Branch,'[^A-Za-z0-9._-]','_')
    if([string]::IsNullOrWhiteSpace($safe)){$safe='detached'}
    $short=if($Head.Length -ge 8){$Head.Substring(0,8)}else{$Head}
    $stamp=Get-Date -Format 'yyyyMMdd-HHmmss'
    $dest=Join-Path $GateArchiveRoot ("{0}-{1}-{2}.json" -f $safe,$short,$stamp)
    Copy-Item -LiteralPath $GatePath -Destination $dest -Force
    Remove-Item -LiteralPath $GatePath -Force
    return $dest
}

function Write-BranchSwitchState(
    [string]$FromBranch,
    [string]$ToBranch,
    [string]$FromHead,
    [string]$ToHead,
    [bool]$RequiresFullGate,
    [string]$ArchivedGate
) {
    $payload=[ordered]@{
        schema='subspace.pcc.branch-switch.v1'
        switchedUtc=(Get-Date).ToUniversalTime().ToString('o')
        fromBranch=$FromBranch
        toBranch=$ToBranch
        fromHead=$FromHead
        toHead=$ToHead
        sourceHeadChanged=($FromHead -ne $ToHead)
        requiresFullGate=$RequiresFullGate
        archivedGate=$ArchivedGate
    }
    $payload|ConvertTo-Json -Depth 6|Set-Content -LiteralPath $BranchStatePath -Encoding UTF8
}

function Complete-Switch([string]$BeforeBranch,[string]$BeforeHead) {
    $afterBranch=Current-Branch
    $afterHead=Current-Head
    if([string]::IsNullOrWhiteSpace($afterBranch)){$afterBranch='DETACHED HEAD'}
    if($beforeHead -eq $afterHead){
        if(Test-Path -LiteralPath $BranchStatePath -PathType Leaf){Remove-Item -LiteralPath $BranchStatePath -Force}
        P ("[PASS] Active branch: {0} @ {1}" -f $afterBranch,$afterHead.Substring(0,8)) Green
        P '[INFO] HEAD did not change, so the exact certified source remains unchanged.' DarkGray
        return
    }

    $archived=Archive-CurrentGate -Branch $BeforeBranch -Head $BeforeHead
    Write-BranchSwitchState -FromBranch $BeforeBranch -ToBranch $afterBranch -FromHead $BeforeHead -ToHead $afterHead -RequiresFullGate $true -ArchivedGate $archived
    P ("[PASS] Active branch: {0} @ {1}" -f $afterBranch,$afterHead.Substring(0,8)) Green
    P '[WARN] Source HEAD changed. The previous GREEN gate was invalidated for safety.' Yellow
    if(-not [string]::IsNullOrWhiteSpace($archived)){P ("[INFO] Previous gate archived: {0}" -f $archived) DarkGray}
    P '[REQUIRED] Run PCC option 1 Full Quality Gate before Commit + Push on this branch.' Yellow
}

function Invoke-BranchSwitch([string[]]$Arguments,[string]$TargetRef) {
    Assert-NoPendingPatch
    Assert-WorkingTreeClean
    $beforeBranch=Current-Branch
    if([string]::IsNullOrWhiteSpace($beforeBranch)){$beforeBranch='DETACHED HEAD'}
    $beforeHead=Current-Head

    if(-not(Target-HasModernPcc $TargetRef)){
        P '[WARN] Target branch does not contain the current Pass1334 branch manager.' Yellow
        P '[WARN] The running PCC can still switch back during this session, but a fresh launch on that branch may expose its older PCC.' Yellow
        $answer=Read-Host ' Continue anyway? [Y/N]'
        if($answer -notmatch '^(?i)y(?:es)?$'){P '[CANCEL] Branch switch cancelled.' Yellow;return}
    }

    $r=Invoke-Git -Arguments $Arguments -AllowFailure
    foreach($line in $r.Lines){P ("  $line") DarkGray}
    if($r.Code -ne 0){
        throw ("Git refused the branch switch. No reset/force fallback was attempted. Command: git {0}" -f ($Arguments -join ' '))
    }
    Complete-Switch -BeforeBranch $beforeBranch -BeforeHead $beforeHead
}

function Get-LocalBranches {
    $r=Invoke-Git -Arguments @('for-each-ref','--sort=refname','--format=%(refname:short)|%(upstream:short)|%(upstream:trackshort)|%(objectname:short)|%(subject)','refs/heads')
    $items=@()
    foreach($line in $r.Lines){
        $parts=[string]$line -split '\|',5
        $items += [pscustomobject]@{
            Name=$parts[0]
            Upstream=if($parts.Count -gt 1){$parts[1]}else{''}
            Track=if($parts.Count -gt 2){$parts[2]}else{''}
            Head=if($parts.Count -gt 3){$parts[3]}else{''}
            Subject=if($parts.Count -gt 4){$parts[4]}else{''}
        }
    }
    return @($items)
}

function Get-OriginBranches {
    $r=Invoke-Git -Arguments @('for-each-ref','--sort=refname','--format=%(refname:short)|%(objectname:short)|%(subject)','refs/remotes/origin')
    $items=@()
    foreach($line in $r.Lines){
        $parts=[string]$line -split '\|',3
        if($parts[0] -eq 'origin/HEAD'){continue}
        $items += [pscustomobject]@{
            Name=$parts[0]
            Head=if($parts.Count -gt 1){$parts[1]}else{''}
            Subject=if($parts.Count -gt 2){$parts[2]}else{''}
        }
    }
    return @($items)
}

function Show-Branches {
    $current=Current-Branch
    $head=Current-Head
    P '------------------------------------------------------------------------' DarkGray
    P ' LOCAL BRANCHES' Cyan
    P '------------------------------------------------------------------------' DarkGray
    $locals=@(Get-LocalBranches)
    for($i=0;$i -lt $locals.Count;$i++){
        $b=$locals[$i]
        $marker=if($b.Name -eq $current){'*'}else{' '}
        $tracking=''
        if(-not [string]::IsNullOrWhiteSpace($b.Upstream)){$tracking=(" -> {0} {1}" -f $b.Upstream,$b.Track).TrimEnd()}
        P (" {0} [{1,2}] {2,-28} {3,-10}{4}" -f $marker,($i+1),$b.Name,$b.Head,$tracking) $(if($b.Name -eq $current){'Green'}else{'Gray'})
    }
    P ''
    P (" Current    : {0}" -f $(if($current){$current}else{'DETACHED HEAD'})) Green
    P (" HEAD       : {0}" -f $head) DarkGray
    $dirty=(Invoke-Git -Arguments @('status','--porcelain=v1','--untracked-files=all')).Lines.Count
    P (" Worktree   : {0}" -f $(if($dirty -eq 0){'Clean'}else{"Modified ($dirty item(s))"})) $(if($dirty -eq 0){'Green'}else{'Yellow'})
}

function Read-IndexedChoice([int]$Count,[string]$Prompt) {
    if($Count -le 0){return -1}
    $raw=Read-Host $Prompt
    $index=0
    if(-not [int]::TryParse($raw,[ref]$index)){return -1}
    if($index -lt 1 -or $index -gt $Count){return -1}
    return ($index-1)
}

function Switch-Local {
    $locals=@(Get-LocalBranches)
    Show-Branches
    $index=Read-IndexedChoice -Count $locals.Count -Prompt 'Local branch number (blank/cancel = no switch)'
    if($index -lt 0){P '[CANCEL] No branch selected.' Yellow;return}
    $target=$locals[$index].Name
    if($target -eq (Current-Branch)){P '[INFO] That branch is already active.' DarkGray;return}
    Invoke-BranchSwitch -Arguments @('switch',$target) -TargetRef $target
}

function Switch-RemoteTracking {
    $remotes=@(Get-OriginBranches)
    if($remotes.Count -eq 0){P '[WARN] No origin remote branches found. Use Fetch / prune origin first.' Yellow;return}
    P '------------------------------------------------------------------------' DarkGray
    P ' ORIGIN BRANCHES' Cyan
    P '------------------------------------------------------------------------' DarkGray
    for($i=0;$i -lt $remotes.Count;$i++){
        $b=$remotes[$i]
        P ("   [{0,2}] {1,-34} {2,-10} {3}" -f ($i+1),$b.Name,$b.Head,$b.Subject)
    }
    $index=Read-IndexedChoice -Count $remotes.Count -Prompt 'Remote branch number (blank/cancel = no switch)'
    if($index -lt 0){P '[CANCEL] No branch selected.' Yellow;return}
    $remote=$remotes[$index].Name
    $local=$remote.Substring('origin/'.Length)
    if(Test-LocalBranch $local){
        Invoke-BranchSwitch -Arguments @('switch',$local) -TargetRef $local
    }else{
        Invoke-BranchSwitch -Arguments @('switch','-c',$local,'--track',$remote) -TargetRef $remote
    }
}

function Create-NewBranch {
    Assert-NoPendingPatch
    Assert-WorkingTreeClean
    $name=(Read-Host 'New branch name').Trim()
    Assert-ValidBranchName $name
    if(Test-LocalBranch $name){throw "Local branch already exists: $name"}
    $beforeBranch=Current-Branch
    if([string]::IsNullOrWhiteSpace($beforeBranch)){$beforeBranch='DETACHED HEAD'}
    $beforeHead=Current-Head
    $r=Invoke-Git -Arguments @('switch','-c',$name) -AllowFailure
    foreach($line in $r.Lines){P ("  $line") DarkGray}
    if($r.Code -ne 0){throw 'Git refused to create/switch the branch. No fallback reset/force action was attempted.'}
    Complete-Switch -BeforeBranch $beforeBranch -BeforeHead $beforeHead
}

function Toggle-PreviousBranch {
    Assert-NoPendingPatch
    Assert-WorkingTreeClean
    $beforeBranch=Current-Branch
    if([string]::IsNullOrWhiteSpace($beforeBranch)){$beforeBranch='DETACHED HEAD'}
    $beforeHead=Current-Head
    $r=Invoke-Git -Arguments @('switch','-') -AllowFailure
    foreach($line in $r.Lines){P ("  $line") DarkGray}
    if($r.Code -ne 0){throw 'Git could not toggle to the previous branch. No reset/force fallback was attempted.'}
    Complete-Switch -BeforeBranch $beforeBranch -BeforeHead $beforeHead
}

function Fetch-Origin {
    Assert-NoPendingPatch
    $remote=Invoke-Git -Arguments @('remote','get-url','origin') -AllowFailure
    if($remote.Code -ne 0){throw "Remote 'origin' is not configured."}
    P ("[STEP] Fetching/pruning origin: {0}" -f ($remote.Lines|Select-Object -First 1)) Cyan
    $r=Invoke-Git -Arguments @('fetch','--prune','origin') -AllowFailure
    foreach($line in $r.Lines){P ("  $line") DarkGray}
    if($r.Code -ne 0){throw 'git fetch --prune origin failed.'}
    P '[PASS] origin refreshed.' Green
}

function Menu {
    while($true){
        try{
            Assert-GitRepository
            try{Clear-Host}catch{}
            P '========================================================================' DarkGray
            P ' CODENAME SUBSPACE - SAFE BRANCH MANAGER' Cyan
            P '========================================================================' DarkGray
            Show-Branches
            P ''
            P ' 1. Refresh / list branches'
            P ' 2. Switch local branch' White
            P ' 3. Switch / create tracking branch from origin' White
            P ' 4. Create new branch from current HEAD'
            P ' 5. Toggle previous branch (git switch -)' Cyan
            P ' 6. Fetch / prune origin'
            P ' 0. Return to PCC'
            P ''
            P ' Safety: branch changes require a clean worktree and no pending root patch.' DarkGray
            P ' No auto-stash, reset, checkout-force, branch deletion, merge, rebase, or force push.' DarkGray
            $choice=Read-Host 'Select'
            try{
                switch($choice){
                    '1'{Show-Branches}
                    '2'{Switch-Local}
                    '3'{Switch-RemoteTracking}
                    '4'{Create-NewBranch}
                    '5'{Toggle-PreviousBranch}
                    '6'{Fetch-Origin}
                    '0'{return}
                    default{P 'Unknown option.' Yellow}
                }
            }catch{P ("[FAIL] {0}" -f $_.Exception.Message) Red}
        }catch{
            P ("[FAIL] {0}" -f $_.Exception.Message) Red
            return
        }
        P ''
        Read-Host 'Press Enter to continue'|Out-Null
    }
}

Menu

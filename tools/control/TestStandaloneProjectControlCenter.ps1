param([string]$Root='')
Set-StrictMode -Version Latest;$ErrorActionPreference='Stop'
if([string]::IsNullOrWhiteSpace($Root)){$Root=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))}else{$Root=[IO.Path]::GetFullPath($Root.Trim().Trim([char[]]@([char]34,[char]39)))}
$fail=0
function T([string]$Name,[bool]$Pass){if($Pass){Write-Host "[PASS] $Name" -ForegroundColor Green}else{Write-Host "[FAIL] $Name" -ForegroundColor Red;$script:fail++}}
$paths=@('SubspaceTools.cmd','SubspaceTools.ps1','tools\control\StandaloneProjectControlCenter.ps1','tools\control\StandalonePatchEngine.ps1','tools\control\WriteQualityGateRecord.ps1','tools\control\SubspaceControlCenter.ps1','project.control.json')
foreach($rel in $paths){T $rel (Test-Path -LiteralPath (Join-Path $Root $rel) -PathType Leaf)}
$cmd=Get-Content -LiteralPath (Join-Path $Root 'SubspaceTools.cmd') -Raw;T 'direct actions bypass wrapper' ($cmd.Contains(':direct') -and $cmd.Contains('SubspaceTools.ps1'));T 'interactive launcher does not pass fragile trailing-backslash Root' (-not $cmd.Contains('-Root "%SUBSPACE_ROOT%"'))
$pcc=Get-Content -LiteralPath (Join-Path $Root 'tools\control\StandaloneProjectControlCenter.ps1') -Raw;T 'PCC self-resolves repository root' ($pcc.Contains('Resolve-StandaloneRepositoryRoot') -and $pcc.Contains("Join-Path $PSScriptRoot '..\\..'"))
foreach($s in @('FULL QUALITY GATE / CERTIFY GREEN','COMMIT + PUSH CURRENT GREEN','Apply this patch now? [Y/N]','git-commit-green','git-push','RequireNoPendingPatch','Assert-StartupIntakePolicy','Full Gate never auto-applies','[STEP] COMMIT: verify or create the exact certified GREEN commit.','[STEP] PUSH + REMOTE VERIFY: publish current branch and verify origin matches local HEAD.','[PASS] REMOTE VERIFY: certified GREEN source is committed and present on origin.')){T ("PCC contract: $s") $pcc.Contains($s)}
$pe=Get-Content -LiteralPath (Join-Path $Root 'tools\control\StandalonePatchEngine.ps1') -Raw
foreach($s in @('PATCH_MANIFEST.json','PRECONDITION_CONFLICT','TARGET_MISMATCH','forge.patch.v1','Rolling back patch transaction','ALREADY-APPLIED','targetProjectId')){T ("patch contract: $s") $pe.Contains($s)}

$contract=Get-Content -LiteralPath (Join-Path $Root 'project.control.json') -Raw|ConvertFrom-Json
T 'contract schema remains forge.project.v1' ([string]$contract.schema -eq 'forge.project.v1')
T 'contract declares forge.patch.v1' ([string]$contract.updates.patchSchema -eq 'forge.patch.v1')
T 'contract startup scan enabled' ($contract.updates.startupScan -eq $true)
T 'contract explicit apply approval required' ($contract.updates.explicitApprovalRequired -eq $true)
T 'contract auto-apply disabled' ($contract.updates.autoApply -eq $false -and $contract.updates.fullGateAutoApply -eq $false)
T 'contract transactional rollback enabled' ($contract.updates.transactional -eq $true -and $contract.updates.rollbackOnFailure -eq $true)
$projectOps=Get-Content -LiteralPath (Join-Path $Root 'tools\control\ProjectOpsCommon.psm1') -Raw
T 'shared certifiable staging authority present' ($projectOps.Contains('Invoke-ProjectOpsStageCertifiableGitChanges') -and $projectOps.Contains('Get-ProjectOpsCertifiableGitChangePaths'))
$subspaceControl=Get-Content -LiteralPath (Join-Path $Root 'tools\control\SubspaceControlCenter.ps1') -Raw
T 'certified commit uses shared source-only staging authority' ($subspaceControl.Contains('Invoke-ProjectOpsStageCertifiableGitChanges') -and -not $subspaceControl.Contains("git add -A -- . ':(exclude)"))
if($fail -gt 0){exit 1};Write-Host '[PASS] Standalone PCC static contract complete.' -ForegroundColor Green

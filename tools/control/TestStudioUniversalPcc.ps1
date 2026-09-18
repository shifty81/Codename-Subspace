param([string]$Root='')
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
if([string]::IsNullOrWhiteSpace($Root)){$Root=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))}
$contract=Get-Content -LiteralPath (Join-Path $Root 'project.control.json') -Raw|ConvertFrom-Json
if([string]$contract.schema -ne 'forge.project.v1'){throw 'Studio provider must retain forge.project.v1.'}
if([string]$contract.updates.patchSchema -ne 'forge.patch.v1'){throw 'Studio integration may not change the patch format.'}
if($contract.updates.autoApply -ne $false -or $contract.updates.fullGateAutoApply -ne $false -or $contract.updates.explicitApprovalRequired -ne $true){throw 'Studio integration weakened patch approval.'}
$commands=@($contract.commands)
$keys=@($commands|ForEach-Object{[string]$_.key})
if(@($keys|Sort-Object -Unique).Count -ne $keys.Count){throw 'Duplicate universal PCC command key.'}
$expected=@{'gate.full'='full-gate';'build.studio'='build-studio';'test.studio'='studio-smoke';'run.studio'='run-studio';'run.shipyard'='run-studio';'run.game'='run-game';'patch.apply'='apply-inbox';'recovery.undo-last'='undo-last-patch';'source.commit-green'='git-commit-green';'source.push-green'='git-push'}
foreach($key in $expected.Keys){
    $items=@($commands|Where-Object{[string]$_.key -eq $key})
    if($items.Count -ne 1){throw "Missing or duplicate PCC command: $key"}
    $command=$items[0]
    if([string]$command.executable -ne 'powershell.exe'){throw "PCC command has unsupported Windows interpreter: $key"}
    $args=@($command.arguments)
    if($args.Count -lt 7 -or [string]$args[4] -ne 'SubspaceTools.ps1' -or [string]$args[6] -ne $expected[$key]){throw "PCC command dispatch mismatch: $key"}
}
foreach($path in @('engine\src\studio_main.cpp','engine\src\studio\StudioApplication.cpp','tools\control\StandaloneProjectControlCenter.ps1')){
    if(-not(Test-Path -LiteralPath (Join-Path $Root $path) -PathType Leaf)){throw "Studio provider source missing: $path"}
}
Write-Host '[PASS] Studio universal PCC contract: interpreter, build, smoke, run, gate, rollback, and Git authorities.' -ForegroundColor Green
exit 0

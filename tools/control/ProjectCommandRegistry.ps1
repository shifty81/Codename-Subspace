[CmdletBinding()]
param(
    [string]$Root = (Get-Location).Path,
    [ValidateSet('List','Describe','Validate')][string]$Action = 'List',
    [string]$Key = '',
    [string]$Category = '',
    [switch]$Json
)

$ErrorActionPreference = 'Stop'
# Historical gate/error-contract marker; ProjectOpsCommon now performs this validation:
# project.control.json is missing commands
Set-StrictMode -Version 2

$Root = [System.IO.Path]::GetFullPath($Root)
$commonPath = Join-Path $PSScriptRoot 'ProjectOpsCommon.psm1'
if (-not (Test-Path -LiteralPath $commonPath -PathType Leaf)) {
    throw "ProjectOps common module missing: $commonPath"
}
Import-Module $commonPath -Force -ErrorAction Stop

$contract = Get-ProjectOpsContract -Root $Root
$commands = @(Get-ProjectOpsCommandRegistry -Root $Root)
if (-not [string]::IsNullOrWhiteSpace($Category)) {
    $commands = @($commands | Where-Object { [string]$_.category -ieq $Category })
}

if ($Action -eq 'Validate') {
    $errors = [System.Collections.Generic.List[string]]::new()
    if ($commands.Count -eq 0) { $errors.Add('Command registry is empty.') | Out-Null }

    foreach ($command in $commands) {
        if ($null -eq $command.arguments) { $errors.Add("Command '$($command.key)' has no arguments array.") | Out-Null }
        if ($null -eq $command.mutates) { $errors.Add("Command '$($command.key)' has no mutates flag.") | Out-Null }
        if ($null -eq $command.requiresConfirmation) { $errors.Add("Command '$($command.key)' has no requiresConfirmation flag.") | Out-Null }
    }

    $payload = [pscustomobject]@{
        project = [string]$contract.id
        commandCount = $commands.Count
        valid = ($errors.Count -eq 0)
        errors = $errors.ToArray()
    }

    if ($Json) {
        $payload | ConvertTo-Json -Depth 8
    }
    else {
        Write-Host 'PROJECTOPS COMMAND REGISTRY VALIDATION'
        Write-Host (" Project      : {0}" -f $contract.name)
        Write-Host (" Commands     : {0}" -f $commands.Count)
        Write-Host (" Result       : {0}" -f $(if($errors.Count -eq 0){'PASS'}else{'FAIL'})) -ForegroundColor $(if($errors.Count -eq 0){'Green'}else{'Red'})
        foreach($error in $errors){ Write-Host (" - " + $error) -ForegroundColor Red }
    }

    if ($errors.Count -gt 0) { exit 1 }
    exit 0
}

if ($Action -eq 'Describe') {
    if ([string]::IsNullOrWhiteSpace($Key)) { throw '-Key is required for Describe.' }
    $command = Get-ProjectOpsCommand -Root $Root -Key $Key
    if ($Json) { $command | ConvertTo-Json -Depth 12 }
    else { $command | Format-List * }
    exit 0
}

if ($Json) {
    @($commands) | ConvertTo-Json -Depth 12
}
else {
    Write-Host 'PROJECTOPS COMMAND REGISTRY'
    Write-Host (" Project      : {0}" -f $contract.name)
    Write-Host (" Commands     : {0}" -f $commands.Count)
    if (-not [string]::IsNullOrWhiteSpace($Category)) { Write-Host (" Category     : {0}" -f $Category) }
    Write-Host '------------------------------------------------------------------------'
    foreach ($command in $commands) {
        $flag = if ([bool]$command.mutates) { '*' } else { ' ' }
        Write-Host ("{0} {1,-32} {2}" -f $flag,$command.key,$command.label)
    }
    Write-Host ''
    Write-Host '* mutates project/state and may require confirmation.' -ForegroundColor DarkGray
}

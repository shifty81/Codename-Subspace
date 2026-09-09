param([string]$Root=(Get-Location).Path,[switch]$DryRun)
$ErrorActionPreference='Stop';$script=Join-Path ([IO.Path]::GetFullPath($Root)) 'scripts\subspace_apply_update_inbox.ps1';if(-not(Test-Path $script)){throw "Missing update authority: $script"}
if($DryRun){& $script -Root $Root -DryRun}else{& $script -Root $Root};if($LASTEXITCODE -ne 0){exit $LASTEXITCODE}

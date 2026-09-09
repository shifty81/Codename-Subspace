param([string]$Root=(Get-Location).Path,[switch]$FailOnWarnings)
$ErrorActionPreference='Stop';$script=Join-Path ([IO.Path]::GetFullPath($Root)) 'scripts\subspace_root_cleanliness_audit.ps1';if(-not(Test-Path $script)){throw "Missing root audit authority: $script"}
if($FailOnWarnings){& $script -Root $Root -FailOnWarnings}else{& $script -Root $Root};if($LASTEXITCODE -ne 0){exit $LASTEXITCODE}

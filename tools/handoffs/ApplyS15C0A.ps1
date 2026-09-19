# S15C0A: run AFTER Subspace PCC approves and consumes the root .patch.
# No stale source is copied into the working tree. Exact Git blob preimages and
# git apply --check protect user changes; a failed preflight writes nothing.
param([switch]$CheckOnly)
Set-StrictMode -Version Latest
$ErrorActionPreference='Stop'
$root=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..\..'))
$diff=Join-Path $PSScriptRoot 'S15C0A_SelectionLock.diff'
$base='cee9993c48f7b0d3c854096c763a77577760b63b'
$preimages=@{
  'engine/src/studio/StudioApplication.cpp'='26cccfaa83dcb550b1e914dbe6a3f0b8eb385228'
  'engine/src/application/NativeGameApplication.cpp'='1bedb3e60205173b718ca62374875ae7524b2d93'
}
if(-not (Test-Path -LiteralPath $diff -PathType Leaf)){throw 'S15C0A diff missing.'}
if(-not (Test-Path -LiteralPath (Join-Path $root 'engine/include/studio/StudioToolInteractionPolicy.h') -PathType Leaf)){throw 'First approve the root .patch using the project PCC.'}
Push-Location $root
try {
  & git rev-parse --is-inside-work-tree | Out-Null
  if($LASTEXITCODE -ne 0){throw 'Not a Git working tree.'}
  & git merge-base --is-ancestor $base HEAD 2>$null
  if($LASTEXITCODE -ne 0){throw "S15C0A expects baseline $base in history; rebase required."}
  $saved=$ErrorActionPreference;$ErrorActionPreference='Continue'
  try { & git apply --unidiff-zero --reverse --check -- $diff 2>$null; $already=$LASTEXITCODE }
  finally { $ErrorActionPreference=$saved }
  if($already -eq 0){Write-Host '[PASS] S15C0A already applied; no changes.' -ForegroundColor Green;return}
  foreach($path in $preimages.Keys){
    $actual=(& git hash-object -- $path).Trim()
    if($LASTEXITCODE -ne 0 -or $actual -ne $preimages[$path]){throw "Refusing stale or edited preimage: $path. Expected $($preimages[$path]); got $actual"}
  }
  & git apply --unidiff-zero --check -- $diff
  if($LASTEXITCODE -ne 0){throw 'S15C0A git apply preflight failed. No changes.'}
  if($CheckOnly){Write-Host '[PASS] S15C0A source and diff preflight.' -ForegroundColor Green;return}
  & git apply --unidiff-zero -- $diff
  if($LASTEXITCODE -ne 0){throw 'S15C0A git apply failed; inspect working tree.'}
  & git diff --check -- engine/src/studio/StudioApplication.cpp engine/src/application/NativeGameApplication.cpp
  if($LASTEXITCODE -ne 0){
    & git apply --unidiff-zero --reverse -- $diff
    throw 'Whitespace verification failed; reverse application attempted.'
  }
  Write-Host '[PASS] S15C0A selection-lock sources applied. Run PCC Full Quality Gate and interactive Studio test.' -ForegroundColor Green
} finally {Pop-Location}

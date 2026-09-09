param([string]$Root=(Get-Location).Path)
$ErrorActionPreference='Stop'
$Root=[System.IO.Path]::GetFullPath($Root)
. (Join-Path $PSScriptRoot 'ControlCenterCommon.ps1')
Write-Host 'SUBSPACE DEPENDENCY / TOOL STATUS'
Write-Host '------------------------------------------------------------------------'
foreach ($r in @(
    (Get-CommandVersionLine 'cmake' @('--version')),
    (Get-CommandVersionLine 'ctest' @('--version')),
    (Get-CommandVersionLine 'git' @('--version')),
    (Get-CommandVersionLine 'python' @('--version')),
    (Get-CommandVersionLine 'blender' @('--version'))
)) {
    Write-Host ("[{0}] {1,-10} {2}" -f $(if($r.present){'PASS'}else{'MISS'}),$r.tool,$r.version) -ForegroundColor $(if($r.present){'Green'}else{'Yellow'})
}
foreach ($pair in @(
    @('Shipyard source cache','content\external\shipyard'),
    @('Derived Shipyard content','content\derived'),
    @('Engine CMake','engine\CMakeLists.txt')
)) {
    $ok=Test-Path -LiteralPath (Join-Path $Root $pair[1])
    Write-Host ("[{0}] {1}" -f $(if($ok){'PASS'}else{'WARN'}),$pair[0]) -ForegroundColor $(if($ok){'Green'}else{'Yellow'})
}

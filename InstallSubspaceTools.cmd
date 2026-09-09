@echo off
setlocal
set "SUBSPACE_ROOT=%~dp0"
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%SUBSPACE_ROOT%scripts\install_subspace_root_utility.ps1" -Root "%SUBSPACE_ROOT%" %*
exit /b %ERRORLEVEL%

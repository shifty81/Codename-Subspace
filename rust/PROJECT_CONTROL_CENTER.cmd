@echo off
setlocal
set "ROOT=%~dp0"
where pwsh.exe >nul 2>&1
if %ERRORLEVEL%==0 (
  pwsh.exe -NoProfile -ExecutionPolicy Bypass -File "%ROOT%tools\project\SubspaceControl.ps1" -Action menu
) else (
  powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%ROOT%tools\project\SubspaceControl.ps1" -Action menu
)
exit /b %ERRORLEVEL%

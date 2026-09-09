@echo off
setlocal
set "SUBSPACE_ROOT=%~dp0"

powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%SUBSPACE_ROOT%SubspaceTools.ps1" %*
set "RC=%ERRORLEVEL%"

if not "%RC%"=="0" (
  echo.
  echo ========================================================================
  echo  CODENAME SUBSPACE CONTROL CENTER - PROCESS EXITED WITH FAILURE
  echo ========================================================================
  echo  Exit code : %RC%
  echo  Root      : %SUBSPACE_ROOT%
  echo.
  echo  The window is being kept open so the failure result can be read.
  echo  Review LATEST_DEBUG_BUNDLE.txt and logs\sessions for diagnostics.
  echo ========================================================================
  echo.
  pause
)

exit /b %RC%

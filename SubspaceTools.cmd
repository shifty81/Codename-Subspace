@echo off
setlocal
set "SUBSPACE_ROOT=%~dp0"
set "SUBSPACE_PCC=%SUBSPACE_ROOT%tools\control\StandaloneProjectControlCenter.ps1"

rem Direct/machine actions remain owned by the existing project utility.
rem Only the no-argument interactive launch uses the simplified standalone PCC front door.
if not "%~1"=="" goto :direct

if exist "%SUBSPACE_PCC%" (
  rem Do not pass %%~dp0 as a quoted -Root argument: its trailing backslash can
  rem be preserved with a quote by powershell.exe argument parsing.  The PCC
  rem derives the repository root from its own tools\control location instead.
  powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%SUBSPACE_PCC%"
) else (
  powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%SUBSPACE_ROOT%SubspaceTools.ps1"
)
goto :done

:direct
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%SUBSPACE_ROOT%SubspaceTools.ps1" %*

:done
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
  echo  Review LATEST_DEBUG_BUNDLE.txt and artifacts\logs\sessions for diagnostics.
  echo ========================================================================
  echo.
  pause
)
exit /b %RC%

@echo off
setlocal EnableExtensions EnableDelayedExpansion
set "SCRIPT_DIR=%~dp0"
for %%I in ("%SCRIPT_DIR%..") do set "ROOT=%%~fI"
set "LOGDIR=%ROOT%\artifacts\logs\r193"
if not exist "%LOGDIR%" mkdir "%LOGDIR%" >nul 2>&1
for /f "tokens=1-3 delims=/:. " %%a in ("%date% %time%") do set "STAMP=%date:~-4%%date:~4,2%%date:~7,2%-%time:~0,2%%time:~3,2%%time:~6,2%"
set "STAMP=%STAMP: =0%"
set "LOG=%LOGDIR%\%STAMP%-r193-player-smallship.log"

echo ============================================================================
echo  CODENAME SUBSPACE R193 - PLAYER SCALE / SMALL SHIP / CONTROL AUTHORITY
echo ============================================================================
echo Root : %ROOT%
echo Log  : %LOG%
echo.

set "PY="
where python >nul 2>&1 && set "PY=python"
if not defined PY (
  where py >nul 2>&1 && set "PY=py -3"
)
if not defined PY (
  echo [FAIL] Python was not found on PATH.
  echo [FAIL] Python was not found on PATH.>>"%LOG%"
  echo Log: %LOG%
  pause
  exit /b 2
)

set "MODE=%~1"
if /i "%MODE%"=="check" goto :check
if /i "%MODE%"=="apply" goto :apply
if not "%MODE%"=="" goto :usage

echo  [C] Check applicability only
echo  [A] Apply transactional R193 source migration
echo  [Q] Quit
echo.
set /p "CHOICE=Select: "
if /i "%CHOICE%"=="C" goto :check
if /i "%CHOICE%"=="A" goto :apply
if /i "%CHOICE%"=="Q" exit /b 0
goto :usage

:check
set "ACTION=--check"
goto :run
:apply
set "ACTION=--apply"
goto :run
:usage
echo Usage: %~nx0 [check^|apply]
echo Usage: %~nx0 [check^|apply]>>"%LOG%"
echo Log: %LOG%
pause
exit /b 2

:run
echo [RUN] %PY% "%SCRIPT_DIR%subspace_r193_control_player_smallship_apply.py" --root "%ROOT%" %ACTION%
echo [RUN] %PY% "%SCRIPT_DIR%subspace_r193_control_player_smallship_apply.py" --root "%ROOT%" %ACTION%>>"%LOG%"
%PY% "%SCRIPT_DIR%subspace_r193_control_player_smallship_apply.py" --root "%ROOT%" %ACTION% >>"%LOG%" 2>&1
set "RC=%ERRORLEVEL%"
type "%LOG%"
echo.
echo [EXIT CODE] %RC%
echo [LOG] %LOG%
if not "%RC%"=="0" (
  echo [FAIL] R193 did not complete. Keep this window open and send the log.
  pause
  exit /b %RC%
)
echo [PASS] R193 command completed.
if /i "%ACTION%"=="--apply" echo Next: run Project Control Center option 1 - FULL QUALITY GATE / CERTIFY GREEN.
if "%~1"=="" pause
exit /b 0

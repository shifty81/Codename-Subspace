@echo off
setlocal
set "ROOT=%~dp0..\.."
where py >nul 2>nul
if %ERRORLEVEL% EQU 0 (
  py -3 "%ROOT%\tools\studio\apply_g02.py" %*
) else (
  python "%ROOT%\tools\studio\apply_g02.py" %*
)
exit /b %ERRORLEVEL%

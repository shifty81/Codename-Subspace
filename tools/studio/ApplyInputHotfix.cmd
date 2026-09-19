@echo off
setlocal
set "ROOT=%~dp0..\.."
where py >nul 2>nul
if %ERRORLEVEL% EQU 0 (
  py -3 "%ROOT%\tools\studio\apply_g03_input.py" %*
) else (
  python "%ROOT%\tools\studio\apply_g03_input.py" %*
)
exit /b %ERRORLEVEL%

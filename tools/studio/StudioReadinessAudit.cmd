@echo off
setlocal
set "SCRIPT=%~dp0studio_readiness_audit.py"
where py >nul 2>nul
if not errorlevel 1 goto run_py
where python >nul 2>nul
if not errorlevel 1 goto run_python
echo [ERROR] Python 3 is required for this read-only Studio audit. 1>&2
exit /b 2
:run_py
py -3 "%SCRIPT%" %*
exit /b %ERRORLEVEL%
:run_python
python "%SCRIPT%" %*
exit /b %ERRORLEVEL%

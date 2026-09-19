@echo off
setlocal
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0ApplyS15B1.ps1"
set "rc=%ERRORLEVEL%"
echo.
if not "%rc%"=="0" echo S15B1 failed; review the error above.
pause
exit /b %rc%

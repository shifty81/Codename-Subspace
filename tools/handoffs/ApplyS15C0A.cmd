@echo off
setlocal
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0ApplyS15C0A.ps1" %*
exit /b %ERRORLEVEL%

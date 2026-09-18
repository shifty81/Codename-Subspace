@echo off
setlocal
set "STUDIO_ROOT=%~dp0..\.."
set "STUDIO_EXE=%STUDIO_ROOT%\engine\build\subspace_studio.exe"
if not exist "%STUDIO_EXE%" (
  echo [FAIL] Studio executable not built: "%STUDIO_EXE%"
  echo Run the project PCC Full Quality Gate first.
  exit /b 2
)
pushd "%STUDIO_ROOT%\engine\build" || exit /b 2
"%STUDIO_EXE%" %*
set "STUDIO_EXIT=%ERRORLEVEL%"
popd
exit /b %STUDIO_EXIT%

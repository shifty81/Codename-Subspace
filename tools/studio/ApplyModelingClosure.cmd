@echo off
setlocal
set "ROOT=%~dp0..\.."
where py >nul 2>nul && (py -3 "%~dp0apply_modeling_closure.py" %* --root "%ROOT%" & exit /b %errorlevel%)
where python >nul 2>nul && (python "%~dp0apply_modeling_closure.py" %* --root "%ROOT%" & exit /b %errorlevel%)
echo Python 3 is required to run the guarded Studio modeling closure. 1>&2
exit /b 2

@echo off
setlocal

REM Check if NSIS is installed
if not exist "%PROGRAMFILES%\NSIS\makensis.exe" (
    if not exist "%PROGRAMFILES(x86)%\NSIS\makensis.exe" (
        echo.
        echo ERROR: NSIS is not installed or not found in the default location.
        echo Please download and install NSIS from: https://nsis.sourceforge.io/
        echo.
        pause
        exit /b 1
    )
    set "NSIS_PATH=%PROGRAMFILES(x86)%\NSIS"
) else (
    set "NSIS_PATH=%PROGRAMFILES%\NSIS"
)

cd /d "%~dp0"

REM Verify bin folder and files exist
if not exist bin\jump.exe (
    echo ERROR: bin\jump.exe not found. Run build_and_run.bat first.
    pause
    exit /b 1
)

echo Compiling NSIS installer...
"%NSIS_PATH%\makensis.exe" flappy-bird-installer.nsi

if errorlevel 1 (
    echo.
    echo Installer compilation failed.
    pause
    exit /b 1
)

echo.
echo Installer created successfully: FlappyBird-Installer.exe
echo You can now distribute this file to users.
pause

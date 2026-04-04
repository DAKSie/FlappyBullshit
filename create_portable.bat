@echo off
setlocal

cd /d "%~dp0"

if not exist bin\jump.exe (
    echo ERROR: bin\jump.exe not found. Run build_and_run.bat first.
    pause
    exit /b 1
)

set "PACKAGE_DIR=FlappyBird-Portable"

if exist "%PACKAGE_DIR%" rmdir /s /q "%PACKAGE_DIR%"
mkdir "%PACKAGE_DIR%"

echo Copying game files...
copy /Y "bin\jump.exe" "%PACKAGE_DIR%\" >nul
copy /Y "bin\*.dll" "%PACKAGE_DIR%\" >nul

REM Create a blank scoreboard file if needed
if not exist "%PACKAGE_DIR%\scoreboard.txt" (
    echo.>"%PACKAGE_DIR%\scoreboard.txt"
)

REM Create a README for portable users
(
    echo Flappy Bird - Portable Version
    echo.
    echo To run the game:
    echo   Simply double-click jump.exe
    echo.
    echo Requirements:
    echo   - Windows 7 or later (64-bit^)
    echo   - Webcam or camera device
    echo.
    echo Controls:
    echo   - Move left/right to control the bird
    echo   - Press ESC to quit
    echo.
    echo Your scores are saved in scoreboard.txt
) > "%PACKAGE_DIR%\README.txt"

echo.
echo Portable package created: %PACKAGE_DIR%
echo Contents:
dir "%PACKAGE_DIR%"
echo.
echo To distribute:
echo   1. Rename folder to something like "FlappyBird"
echo   2. ZIP the entire folder
echo   3. Share the ZIP file with users
echo.
pause

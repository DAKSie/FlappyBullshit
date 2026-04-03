@echo off
setlocal

cd /d "%~dp0"
if not exist bin mkdir bin

set "MSYS2_UCRT64=C:\msys64\ucrt64"
set "PATH=%MSYS2_UCRT64%\bin;%PATH%"

echo Building src/*.cpp...
"%MSYS2_UCRT64%\bin\g++.exe" src/*.cpp -std=c++17 -IC:/msys64/ucrt64/include -IC:/msys64/ucrt64/include/opencv4 -LC:/msys64/ucrt64/lib -lSDL3 -lopencv_core -lopencv_objdetect -lopencv_imgproc -lopencv_photo -o bin\jump.exe
if errorlevel 1 goto :build_failed

copy /Y "%MSYS2_UCRT64%\bin\SDL3.dll" bin\ >nul 2>nul
copy /Y "%MSYS2_UCRT64%\bin\libopencv_core-*.dll" bin\ >nul 2>nul
copy /Y "%MSYS2_UCRT64%\bin\libopencv_objdetect-*.dll" bin\ >nul 2>nul
copy /Y "%MSYS2_UCRT64%\bin\libopencv_imgproc-*.dll" bin\ >nul 2>nul
copy /Y "%MSYS2_UCRT64%\bin\libopencv_photo-*.dll" bin\ >nul 2>nul
copy /Y "%MSYS2_UCRT64%\bin\libgcc_s_seh-1.dll" bin\ >nul 2>nul
copy /Y "%MSYS2_UCRT64%\bin\libstdc++-6.dll" bin\ >nul 2>nul
copy /Y "%MSYS2_UCRT64%\bin\libwinpthread-1.dll" bin\ >nul 2>nul

echo Running bin\jump.exe...
bin\jump.exe
if errorlevel 1 goto :run_failed

goto :eof

:build_failed
echo Build failed.
pause
exit /b 1

:run_failed
echo Program exited with an error.
pause
exit /b 1
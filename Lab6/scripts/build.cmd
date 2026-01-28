@echo off
cd /d %~dp0\..

set SFML_DIR=C:\SFML-3\lib\cmake\SFML

if exist build rmdir /S /Q build
mkdir build
cd build

cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -DSFML_DIR=%SFML_DIR% ..

mingw32-make

echo.
echo ==== COPYING SFML DLLs ====
copy /Y C:\SFML-3\bin\sfml-system-3.dll .
copy /Y C:\SFML-3\bin\sfml-window-3.dll .
copy /Y C:\SFML-3\bin\sfml-graphics-3.dll .
copy /Y C:\SFML-3\bin\sfml-network-3.dll .
copy /Y C:\SFML-3\bin\sfml-audio-3.dll .

echo.
echo BUILD DONE
pause

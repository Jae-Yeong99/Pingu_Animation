@echo off
if not exist build mkdir build
C:\MinGW\bin\g++.exe -std=c++17 -O2 -Wall -Wextra -mwindows src\main.cpp -o build\DesktopPet.exe -lgdi32 -luser32 -lshell32 -lgdiplus
if errorlevel 1 exit /b 1
if not exist build\assets mkdir build\assets
copy /Y assets\penguin_walk.png build\assets\penguin_walk.png >nul
echo Built build\DesktopPet.exe

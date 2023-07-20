@echo off

setlocal enabledelayedexpansion
set flags=-std=c++20 ^
    -Wall -Wextra -Wno-c99-designator -Wno-reorder-init-list -Wno-microsoft-enum-forward-reference ^
    -I include/ ^
    -I libs/include/ ^
    -D BAT_RUN ^
    -D HOT_RELOAD

if %errorlevel% neq 0 exit /b %errorlevel%
ccache.exe clang++ -c -o bin/obj/Game.o src/Game.cpp -g %flags% 
if %errorlevel% neq 0 exit /b %errorlevel%
ccache.exe clang++ -c -o bin/obj/Engine.o src/Engine.cpp -g %flags% 
if %errorlevel% neq 0 exit /b %errorlevel%
ccache.exe clang++ -c -o bin/obj/Graphics.o src/Graphics.cpp -g %flags% 
if %errorlevel% neq 0 exit /b %errorlevel%
ccache.exe clang++ -c -o bin/obj/Image.o src/Image.cpp -g %flags% 
if %errorlevel% neq 0 exit /b %errorlevel%
ccache.exe clang++ -c -o bin/obj/Input.o src/Input.cpp -g %flags% 
if %errorlevel% neq 0 exit /b %errorlevel%
ccache.exe clang++ -c -o bin/obj/Timer.o src/Timer.cpp -g %flags% 
if %errorlevel% neq 0 exit /b %errorlevel%
ccache.exe clang++ -c -o bin/obj/Audio.o src/Audio.cpp -g %flags% 
if %errorlevel% neq 0 exit /b %errorlevel%
ccache.exe clang++ -c -o bin/obj/Tile.o src/Tile.cpp   -g %flags% 
if %errorlevel% neq 0 exit /b %errorlevel%
ccache.exe clang++ -c -o bin/obj/Grid.o src/Grid.cpp   -g %flags% 
if %errorlevel% neq 0 exit /b %errorlevel%
ccache.exe clang++ -c -o bin/obj/Color.o src/Color.cpp -g %flags% 
if %errorlevel% neq 0 exit /b %errorlevel%
ccache.exe clang++ -c -o bin/obj/Random.o src/Random.cpp -g %flags% 
if %errorlevel% neq 0 exit /b %errorlevel%
ccache.exe clang++ -c -o bin/obj/UnitManager.o src/UnitManager.cpp -g %flags% 
if %errorlevel% neq 0 exit /b %errorlevel%

clang++ -o bin/Game.dll -shared bin/obj/Game.o bin/obj/Engine.o bin/obj/Graphics.o bin/obj/Image.o bin/obj/Input.o bin/obj/Timer.o bin/obj/Audio.o bin/obj/Tile.o bin/obj/Grid.o bin/obj/Color.o bin/obj/Random.o bin/obj/UnitManager.o 
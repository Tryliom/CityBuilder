@echo off

mkdir bin
mkdir bin\obj

call .\build_dll.bat

setlocal enabledelayedexpansion
set flags=-std=c++20 ^
    -Wall -Wextra -Wno-c99-designator -Wno-reorder-init-list -Wno-microsoft-enum-forward-reference ^
    -I include/ ^
    -I libs/include/ ^
    -D BAT_RUN ^
    -D HOT_RELOAD

ccache.exe clang++ -c -o bin/obj/Engine.o src/Engine.cpp -g %flags% 
if %errorlevel% neq 0 exit /b %errorlevel%
ccache.exe clang++ -c -o bin/obj/Image.o src/Image.cpp   -g %flags% 
if %errorlevel% neq 0 exit /b %errorlevel%
ccache.exe clang++ -c -o bin/obj/Input.o src/Input.cpp   -g %flags% 
if %errorlevel% neq 0 exit /b %errorlevel%
ccache.exe clang++ -c -o bin/obj/Timer.o src/Timer.cpp   -g %flags% 
if %errorlevel% neq 0 exit /b %errorlevel%

clang++ -o bin/game.exe -g bin/obj/Engine.o bin/obj/Image.o bin/obj/Input.o bin/obj/Timer.o bin/Game.lib
   
bin\game.exe
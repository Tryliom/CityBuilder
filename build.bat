@echo off

mkdir bin
mkdir bin\obj

set flags=-std=c++20 ^
    -Wall -Wextra -Wno-c99-designator -Wno-reorder-init-list -Wno-microsoft-enum-forward-reference -Wno-unused-parameter -Wno-unused-variable -Wno-missing-field-initializers -Wno-switch -Wno-logical-op-parentheses -Wno-deprecated-declarations -Wno-missing-braces ^
    -I include/ ^
    -I libs/include/ ^
    -D BAT_RUN 

ccache.exe clang++ -c -o bin/obj/Game.o src/Game.cpp -g %flags%
ccache.exe clang++ -c -o bin/obj/Engine.o src/Engine.cpp -g %flags%
ccache.exe clang++ -c -o bin/obj/Graphics.o src/Graphics.cpp -g %flags%
ccache.exe clang++ -c -o bin/obj/Image.o src/Image.cpp -g %flags%
ccache.exe clang++ -c -o bin/obj/Input.o src/Input.cpp -g %flags%
ccache.exe clang++ -c -o bin/obj/Timer.o src/Timer.cpp -g %flags%
ccache.exe clang++ -c -o bin/obj/Audio.o src/Audio.cpp -g %flags%
ccache.exe clang++ -c -o bin/obj/Tile.o src/Tile.cpp   -g %flags%
ccache.exe clang++ -c -o bin/obj/Grid.o src/Grid.cpp   -g %flags%
ccache.exe clang++ -c -o bin/obj/Color.o src/Color.cpp -g %flags%
ccache.exe clang++ -c -o bin/obj/Random.o src/Random.cpp -g %flags%
ccache.exe clang++ -c -o bin/obj/UnitManager.o src/UnitManager.cpp -g %flags%

ccache.exe clang++ -c -o bin/obj/imgui_demo.o src/imgui_demo.cpp -g %flags%
ccache.exe clang++ -c -o bin/obj/imgui_draw.o src/imgui_draw.cpp -g %flags%
ccache.exe clang++ -c -o bin/obj/imgui_tables.o src/imgui_tables.cpp -g %flags%
ccache.exe clang++ -c -o bin/obj/imgui_widgets.o src/imgui_widgets.cpp -g %flags%
ccache.exe clang++ -c -o bin/obj/imgui.o src/imgui.cpp -g %flags%


clang++ -o bin/game.exe -g bin/obj/Game.o bin/obj/Engine.o bin/obj/Graphics.o bin/obj/Image.o bin/obj/Input.o bin/obj/Timer.o bin/obj/Audio.o bin/obj/Tile.o bin/obj/Grid.o bin/obj/Color.o bin/obj/Random.o bin/obj/UnitManager.o bin/obj/imgui_demo.o bin/obj/imgui_draw.o bin/obj/imgui_tables.o bin/obj/imgui_widgets.o bin/obj/imgui.o

bin\game.exe
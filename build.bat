mkdir bin
clang++ src/Window.cpp src/Game.cpp src/Image.cpp src/Input.cpp src/Timer.cpp src/Audio.cpp src/Tile.cpp src/Grid.cpp src/Color.cpp src/Random.cpp src/UnitManager.cpp ^
    -o bin/game.exe ^
    -std=c++20 ^
    -Wall -Wextra -Wno-c99-designator -Wno-reorder-init-list -Wno-microsoft-enum-forward-reference -Wno-unused-parameter -Wno-logical-op-parentheses -Wno-deprecated-declarations -Wno-missing-braces ^
    -I include/ ^
    -I libs/include/ ^
    -D BAT_RUN 
bin\game.exe
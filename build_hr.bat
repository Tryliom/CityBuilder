mkdir bin
clang++ src/Engine.cpp src/Game.cpp src/Image.cpp src/Input.cpp src/Timer.cpp src/Audio.cpp src/Tile.cpp src/Grid.cpp src/Color.cpp src/Random.cpp src/UnitManager.cpp src/Graphics.cpp^
    -o bin/game.exe -g ^
    -std=c++20 ^
    -Wall -Wextra -Wno-c99-designator -Wno-reorder-init-list -Wno-microsoft-enum-forward-reference ^
    -I include/ ^
    -I libs/include/ ^
    -D BAT_RUN ^
    -D HOT_RELOAD

clang++ -o bin/Game.dll src/Game.cpp -shared -g src/Engine.cpp src/Graphics.cpp src/Image.cpp src/Input.cpp src/Timer.cpp src/Audio.cpp src/Tile.cpp src/Grid.cpp src/Color.cpp src/Random.cpp src/UnitManager.cpp ^
    -std=c++20 ^
    -Wall -Wextra -Wno-c99-designator -Wno-reorder-init-list -Wno-microsoft-enum-forward-reference ^
    -I include/ ^
    -I libs/include/ ^
    -D BAT_RUN ^
    -D HOT_RELOAD

bin\game.exe
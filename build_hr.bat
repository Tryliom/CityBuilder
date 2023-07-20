mkdir bin
mkdir bin\obj

call .\build_dll.bat

clang++ src/Engine.cpp src/Input.cpp src/Image.cpp src/Timer.cpp bin/Game.lib ^
    -o bin/game.exe ^
    -std=c++20 ^
    -Wall -Wextra -Wno-c99-designator -Wno-reorder-init-list -Wno-microsoft-enum-forward-reference -Wno-unused-parameter -Wno-unused-variable -Wno-missing-field-initializers -Wno-switch -Wno-logical-op-parentheses -Wno-deprecated-declarations -Wno-missing-braces ^
    -I include/ ^
    -I libs/include/ ^
    -D BAT_RUN ^
    -D HOT_RELOAD

bin\game.exe
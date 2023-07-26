mkdir -p bin
mkdir -p bin/obj

FLAGS="-std=c++20 \
    -Wall -Wextra -Wno-c99-designator -Wno-reorder-init-list -Wno-microsoft-enum-forward-reference -Wno-unused-parameter -Wno-unused-variable -Wno-missing-field-initializers -Wno-switch -Wno-logical-op-parentheses -Wno-deprecated-declarations -Wno-missing-braces \
    -I ./include/ \
    -I ./libs/include/ \
    -D BAT_RUN \
    -fobjc-arc -ObjC++ \
    -DSOKOL_GLCORE33 \
    "

    #-fsanitize=address \

./ccache clang++ -c -o bin/obj/Game.o src/Game.cpp -g $FLAGS
./ccache clang++ -c -o bin/obj/Engine.o src/Engine.cpp -g $FLAGS
./ccache clang++ -c -o bin/obj/Graphics.o src/Graphics.cpp -g $FLAGS
./ccache clang++ -c -o bin/obj/GUI.o src/GUI.cpp     -g $FLAGS
./ccache clang++ -c -o bin/obj/Image.o src/Image.cpp -g $FLAGS
./ccache clang++ -c -o bin/obj/Input.o src/Input.cpp -g $FLAGS
./ccache clang++ -c -o bin/obj/Timer.o src/Timer.cpp -g $FLAGS
./ccache clang++ -c -o bin/obj/Audio.o src/Audio.cpp -g $FLAGS
./ccache clang++ -c -o bin/obj/Tile.o src/Tile.cpp   -g $FLAGS
./ccache clang++ -c -o bin/obj/Grid.o src/Grid.cpp   -g $FLAGS
./ccache clang++ -c -o bin/obj/Color.o src/Color.cpp -g $FLAGS
./ccache clang++ -c -o bin/obj/Random.o src/Random.cpp -g $FLAGS
./ccache clang++ -c -o bin/obj/UnitManager.o src/UnitManager.cpp -g $FLAGS
./ccache clang++ -c -o bin/obj/Serialization.o src/Serialization.cpp -g $FLAGS
./ccache clang++ -c -o bin/obj/Platform.o src/Platform.cpp -g $FLAGS
./ccache clang++ -c -o bin/obj/imgui_demo.o src/imgui_demo.cpp -g $FLAGS
./ccache clang++ -c -o bin/obj/imgui_draw.o src/imgui_draw.cpp -g $FLAGS
./ccache clang++ -c -o bin/obj/imgui_tables.o src/imgui_tables.cpp -g $FLAGS
./ccache clang++ -c -o bin/obj/imgui_widgets.o src/imgui_widgets.cpp -g $FLAGS
./ccache clang++ -c -o bin/obj/imgui.o src/imgui.cpp -g $FLAGS

clang++ -o bin/game -g bin/obj/Game.o bin/obj/Engine.o bin/obj/Platform.o bin/obj/Graphics.o bin/obj/GUI.o bin/obj/Image.o bin/obj/Input.o bin/obj/Timer.o bin/obj/Audio.o bin/obj/Tile.o bin/obj/Grid.o bin/obj/Color.o bin/obj/Random.o bin/obj/UnitManager.o bin/obj/imgui_demo.o bin/obj/imgui_draw.o bin/obj/imgui_tables.o bin/obj/imgui_widgets.o bin/obj/imgui.o bin/obj/Serialization.o \
    -framework OpenGL -framework Cocoa -framework MetalKit -framework Quartz -framework AudioToolbox
    #-fsanitize=address

bin/game

# minimum CMake version required
cmake_minimum_required(VERSION 3.15)

# Project name, version and description
project("CityBuilder" VERSION 0.1 DESCRIPTION "Simple city builder for SAE sommative")

# Set C++ version
set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
# Include all cpp and h files
file(GLOB_RECURSE SRC src/*.cpp include/*.h libs/include/*.h)

# Add executable
add_executable(${PROJECT_NAME} ${SRC})

# Include libraries
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/libs/include/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/)

if(APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework Metal")
    target_link_libraries(${PROJECT_NAME} "-framework MetalKit")
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} "-framework AudioToolbox")
endif()

if(WIN32)
    target_link_libraries(${PROJECT_NAME} kernel32 user32 shell32 dxgi d3d11 ole32 gdi32 winmm opengl32)
endif()


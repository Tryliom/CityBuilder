# City builder

## Build

### CLion

- Go to `File` -> `Settings` -> `Build, Execution, Deployment` -> `Toolchains` -> `+` -> `System`

[Clion toolchains](documentation/images/clion_toolchains.png)

- Go to `File` -> `Settings` -> `Build, Execution, Deployment` -> `CMake` -> add a new profile with the toolchain you just created named `Debug`

[Clion CMake](documentation/images/clion_cmake.png)

- Go to `CMakeLists.txt` and reload it

### VsCode/Vs

Run `build.bat`

## How to compile the shader

Run `buildShader.bat`
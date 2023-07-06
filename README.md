# sokolTest

## Comment faire le shader basic

1. Cloner le repo https://github.com/floooh/sokol-tools-bin.git dans `C:\tools`
2. Cloner le repo https://github.com/floooh/sokol-samples.git n'importe où
3. Aller dans `sokol-tools-bin\bin\win32`
4. Copier le fichier `triangle-sapp.glsl` depuis sokol samples `sokol-samples\sapp`
5. Utiliser la commande `C:\tools\sokol-tools-bin\bin\win32\sokol-shdc.exe --input triangle-sapp.glsl --output basic-sapp.glsl.h --slang hlsl5:glsl330`
6. Déplacer le `basic-sapp.glsl.h` dans son projet
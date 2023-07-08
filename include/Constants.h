#pragma once

// Implement this when you need to access to any folder or variables depending on the platform.

#ifdef __EMSCRIPTEN__
#define ASSETS_PATH "assets/"
#elif BAT_RUN
#define ASSETS_PATH "assets/"
#else
#define ASSETS_PATH "../assets/"
#endif
#pragma once

// Dynamically linked library export/import defines
#ifdef WIN32
  #ifdef FZ_BUILD_LIB
    #define FZ_API __declspec(dllexport)
  #else
    #define FZ_API __declspec(dllimport)
  #endif
#else
  #define FZ_API
#endif

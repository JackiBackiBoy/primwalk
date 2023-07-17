#pragma once

// Dynamically linked library export/import defines
#ifdef WIN32
  #ifdef PW_BUILD_LIB
    #define PW_API __declspec(dllexport)
  #else
    #define PW_API __declspec(dllimport)
  #endif
#else
  #define PW_API
#endif

cmake_minimum_required(VERSION 3.20.0)

if (WIN32)
  set(CMAKE_CXX_COMPILER "C:/Users/TheGa/mingw64/bin/g++.exe")
  set(CMAKE_C_COMPILER "C:/Users/TheGa/mingw64/bin/gcc.exe")
endif()

project(fz)

# Initialize and update the submodule recursively
execute_process(
  COMMAND git submodule update --init --recursive
  WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY}/vendor/msdf-atlas-gen
  RESULT_VARIABLE submodule_update_result
)

# ------ MSDF-Atlas-Gen ------
set(MSDF_ATLAS_USE_SKIA OFF CACHE BOOL "Build with the Skia library" FORCE) # disable Skia
add_subdirectory(vendor/msdf-atlas-gen)

if(NOT submodule_update_result EQUAL "0")
  message(FATAL_ERROR "Failed to update sub-submodule")
endif()

add_subdirectory(fzui)
add_subdirectory(fzcoach)

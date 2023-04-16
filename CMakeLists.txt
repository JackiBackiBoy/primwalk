cmake_minimum_required(VERSION 3.20.0)
cmake_policy(SET CMP0091 NEW)

if (CMAKE_GENERATOR STREQUAL "MinGW Makefiles")
  set(CMAKE_CXX_COMPILER g++)
  set(CMAKE_C_COMPILER gcc)
endif()

project(fz)

# Initialize and update the submodule recursively
execute_process(
  COMMAND git submodule update --init --recursive
  WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY}/vendor/msdf-atlas-gen
  RESULT_VARIABLE submodule_update_result
)

if(NOT submodule_update_result EQUAL "0")
  message(FATAL_ERROR "Failed to update sub-submodule")
endif()

if(MSVC)
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
  add_compile_options(
    $<$<CONFIG:>:/MT> #---------|
    $<$<CONFIG:Debug>:/MTd> #---|-- Statically link the runtime libraries
    $<$<CONFIG:Release>:/MT> #--|
  )
endif()

# ------ MSDF-Atlas-Gen ------
set(MSDF_ATLAS_USE_SKIA OFF CACHE BOOL "Build with the Skia library" FORCE) # disable Skia

add_subdirectory(vendor/msdf-atlas-gen)
add_subdirectory(fzui)
add_subdirectory(fzcoach)

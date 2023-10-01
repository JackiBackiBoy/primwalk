cmake_minimum_required(VERSION 3.20.0)
cmake_policy(SET CMP0091 NEW)

if (CMAKE_GENERATOR STREQUAL "MinGW Makefiles")
  set(CMAKE_CXX_COMPILER g++)
  set(CMAKE_C_COMPILER gcc)
endif()

project(fz)

# 1. MSDFGEN
execute_process(
  COMMAND git submodule update --init --recursive
  WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY}/vendor/msdf-atlas-gen
  RESULT_VARIABLE submodule_update_result
)

if(NOT submodule_update_result EQUAL "0")
  message(FATAL_ERROR "Failed to update sub-submodule")
endif()

# 4. Asset Import Library (assimp)
find_path(ASSIMP_INCLUDE_DIR vendor/assimp ${CMAKE_HOME_DIRECTORY})

if((NOT ASSIMP_INCLUDE_DIR) OR (NOT EXISTS ${ASSIMP_INCLUDE_DIR}))
  message(STATUS "Unable to find Assimp. Installing it now.")
  execute_process(COMMAND git submodule update --init -- vendor/assimp
                    WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY})
  
  set(ASSIMP_INCLUDE_DIR ${CMAKE_HOME_DIRECTORY}/vendor/assimp/include)
else()
  set(ASSIMP_INCLUDE_DIR ${CMAKE_HOME_DIRECTORY}/vendor/assimp/include)
endif()

set(ASSIMP_LIBRARIES ${CMAKE_HOME_DIRECTORY}/build/vendor/assimp/lib)

if(MSVC)
  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
  add_compile_options(
    $<$<CONFIG:>:/MT> #---------|
    $<$<CONFIG:Debug>:/MDd> #---|-- Statically link the runtime libraries
    $<$<CONFIG:Release>:/MT> #--|
  )
endif()

# Assimp build flags
set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libraries" FORCE)
set(ASSIMP_BUILD_ZLIB OFF CACHE BOOL "enable zlib functionality" FORCE)
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "enable samples functionality" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "enable tests functionality" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "enable tools functionality" FORCE)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "enable build all imp functionality" FORCE)
set(ASSIMP_BUILD_GLTF_IMPORTER ON CACHE BOOL "enable GLTF functionality" FORCE)
set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF CACHE BOOL "enable build all exp functionality" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "[Assimp] Treat all warnings as errors." FORCE)
add_subdirectory(vendor/assimp)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries" FORCE)

# ------ MSDF-Atlas-Gen ------
set(MSDF_ATLAS_USE_SKIA OFF CACHE BOOL "Build with the Skia library" FORCE) # disable Skia
add_subdirectory(vendor/msdf-atlas-gen)

add_subdirectory(primwalk)
add_subdirectory(fzcoach)

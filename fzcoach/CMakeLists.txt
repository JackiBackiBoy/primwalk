cmake_minimum_required(VERSION 3.20.0)

# Platform specifics
if (WIN32)
  set(PLATFORM "windows")
  add_definitions(-DFZ_WIN32 -DWIN32 -D_WINDOWS -D_UNICODE -DUNICODE)
  set(GLAD_INCLUDE_DIR ${CMAKE_HOME_DIRECTORY}/vendor/glad/Windows/include)
elseif(APPLE)
  set(PLATFORM "macos")
  add_definitions(-DFZ_MACOS)
  set(GLAD_INCLUDE_DIR ${CMAKE_HOME_DIRECTORY}/vendor/glad/MacOS/include)
endif()

set(NAME fzcoach)
set(SOURCE_DIR ${CMAKE_HOME_DIRECTORY}/${NAME}/src) 

# Binary output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Compiler specification
if (WIN32)
  set(CMAKE_CXX_COMPILER "C:/Users/TheGa/mingw64/bin/g++.exe")
  #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mwindows")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lwinpthread")
endif()

project(${NAME})

include_directories(
  ${SOURCE_DIR}
  ${CMAKE_HOME_DIRECTORY}/fzui/src
)

# Settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (WIN32)
  add_definitions(-DWIN32 -D_WINDOWS -D_UNICODE -DUNICODE)
  set_source_files_properties(${CMAKE_HOME_DIRECTORY}/resources.rc LANGUAGE RC)

  if(MINGW)
    set(CMAKE_RC_COMPILER_INIT windres)
    enable_language(RC)
    set(CMAKE_RC_COMPILE_OBJECT
      "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")
  endif(MINGW)
endif(WIN32)

# Retrieve all source and header files
file(GLOB_RECURSE SOURCE_FILES ${SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE HEADER_FILES ${SOURCE_DIR}/*.hpp)
file(GLOB RESOURCE_FILE ${CMAKE_HOME_DIRECTORY}/resources.o)

find_package(OpenGL REQUIRED)

add_executable(${NAME} ${SOURCE_FILES} ${HEADER_FILES} ${RESOURCE_FILE})
target_compile_features(${NAME} PUBLIC cxx_std_17)

target_include_directories(${NAME} PRIVATE
  ${CMAKE_HOME_DIRECTORY}/fzui/include
  ${GLAD_INCLUDE_DIR}
)

target_link_libraries(${NAME} PUBLIC
  fzui
  OpenGL::GL
)

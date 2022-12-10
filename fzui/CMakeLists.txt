cmake_minimum_required(VERSION 3.20.0)

set(NAME fzui)
set(SOURCE_DIR ${CMAKE_HOME_DIRECTORY}/${NAME}/src) 
set(GLAD_INCLUDE_DIR ${CMAKE_HOME_DIRECTORY}/vendor/glad/include)

# Binary output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Compiler specification
if (WIN32)
  set(CMAKE_CXX_COMPILER "C:/Users/TheGa/mingw64/bin/g++.exe")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")
endif()

project(${NAME})

include_directories(
  ${SOURCE_DIR}
)

# Settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (WIN32)
  add_definitions(-DWIN32 -D_WINDOWS -D_UNICODE -DUNICODE)
endif()

# Retrieve all source and header files
file(GLOB_RECURSE SOURCE_FILES ${SOURCE_DIR}/*.cpp ${SOURCE_DIR}/*.c)
file(GLOB_RECURSE HEADER_FILES ${SOURCE_DIR}/*.hpp)

find_package(OpenGL REQUIRED)

add_library(${NAME} SHARED ${SOURCE_FILES} ${HEADER_FILES})

# Eliminate target prefixes
set_target_properties(${NAME} PROPERTIES PREFIX "")
set_target_properties(${NAME} PROPERTIES IMPORT_PREFIX "")

target_compile_features(${NAME} PUBLIC cxx_std_17)
target_compile_definitions(${NAME} PRIVATE
  FZ_BUILD_LIB
)

target_include_directories(${NAME} PRIVATE
  ${GLAD_INCLUDE_DIR}
)

target_link_libraries(${NAME} PUBLIC
  OpenGL::GL
  comctl32.lib
  msimg32.lib
  Dwmapi.lib
  UxTheme
)

cmake_minimum_required(VERSION 3.20.0)

set(NAME fzcoach)
set(SOURCE_DIR ${CMAKE_HOME_DIRECTORY}/src/${NAME}) 
set(GLAD_INCLUDE_DIR ${CMAKE_HOME_DIRECTORY}/vendor/glad/include)

# Compiler specification
if (WIN32)
  set(CMAKE_CXX_COMPILER "C:/Users/TheGa/mingw64/bin/g++.exe")
  set(CMAKE_C_COMPILER "C:/Users/TheGa/mingw64/bin/gcc.exe")
endif()

project(${NAME})

include_directories(
  ${SOURCE_DIR}
)

# Settings
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
file(GLOB_RECURSE SOURCE_FILES ${SOURCE_DIR}/*.c)
file(GLOB_RECURSE HEADER_FILES ${SOURCE_DIR}/*.h)
file(GLOB RESOURCE_FILE ${CMAKE_HOME_DIRECTORY}/resources.o)

find_package(OpenGL REQUIRED)

add_executable(${NAME} ${SOURCE_FILES} ${HEADER_FILES} ${RESOURCE_FILE})
set_property(TARGET ${NAME} PROPERTY C_STANDARD 11)

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

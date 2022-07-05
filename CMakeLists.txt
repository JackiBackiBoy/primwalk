cmake_minimum_required(VERSION 3.20.0)

set(NAME fzcoach)
set(SOURCE_DIR ${CMAKE_HOME_DIRECTORY}/src/${NAME}) 

project(${NAME})

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

add_executable(${NAME} ${SOURCE_FILES} ${HEADER_FILES} ${RESOURCE_FILE})
set_property(TARGET ${NAME} PROPERTY C_STANDARD 11)
target_link_libraries(${NAME} PUBLIC
  comctl32.lib)

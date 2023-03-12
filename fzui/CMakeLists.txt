cmake_minimum_required(VERSION 3.20.0)

# Platform specifics
if (WIN32)
  set(PLATFORM "windows")
  add_definitions(-DFZ_WIN32 -DWIN32 -D_WINDOWS -D_UNICODE -DUNICODE)
elseif(APPLE)
  set(PLATFORM "macos")
endif()

set(NAME fzui)
set(SOURCE_DIR ${CMAKE_HOME_DIRECTORY}/${NAME}/src) 
set(INCLUDE_DIR ${CMAKE_HOME_DIRECTORY}/${NAME}/include) 
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
  ${INCLUDE_DIR}
)

# 2. GLM
find_path(GLM_INCLUDE_DIR vendor/glm ${CMAKE_HOME_DIRECTORY})

if((NOT GLM_INCLUDE_DIR) OR (NOT EXISTS ${GLM_INCLUDE_DIR}))
  message(STATUS "Unable to find GLM. Installing it now.")
  execute_process(COMMAND git submodule update --init -- vendor/glm
                    WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY})
  
  set(GLM_INCLUDE_DIR ${CMAKE_HOME_DIRECTORY}/vendor/glm)
else()
  set(GLM_INCLUDE_DIR ${CMAKE_HOME_DIRECTORY}/vendor/glm)
endif()

# 3. MSDFGEN
find_path(MSDFGEN_INCLUDE_DIR vendor/msdf-atlas-gen ${CMAKE_HOME_DIRECTORY})

if((NOT MSDFGEN_INCLUDE_DIR) OR (NOT EXISTS ${MSDFGEN_INCLUDE_DIR}))
  message(STATUS "Unable to find MSDFGEN. Installing it now.")
  execute_process(COMMAND git submodule update --init -- vendor/msdf-atlas-gen
                    WORKING_DIRECTORY ${CMAKE_HOME_DIRECTORY})
  
  set(MSDFGEN_INCLUDE_DIR ${CMAKE_HOME_DIRECTORY}/vendor/msdf-atlas-gen)
else()
  set(MSDFGEN_INCLUDE_DIR ${CMAKE_HOME_DIRECTORY}/vendor/msdf-atlas-gen)
endif()

set(MSDFGEN_LIBRARIES ${CMAKE_HOME_DIRECTORY}/build/vendor/msdf-atlas-gen)


# Settings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Retrieve all source and header files
file(GLOB_RECURSE SOURCE_FILES ${SOURCE_DIR}/*.cpp ${SOURCE_DIR}/*.c)
file(GLOB_RECURSE HEADER_FILES ${INCLUDE_DIR}/*.hpp)

find_package(OpenGL REQUIRED)

add_library(${NAME} SHARED ${SOURCE_FILES} ${HEADER_FILES})

# Eliminate target prefixes
set_target_properties(${NAME} PROPERTIES PREFIX "")
set_target_properties(${NAME} PROPERTIES IMPORT_PREFIX "")

target_compile_features(${NAME} PUBLIC cxx_std_17)
target_compile_definitions(${NAME} PRIVATE
  FZ_BUILD_LIB
  BASE_DIR="${CMAKE_HOME_DIRECTORY}/"
  GLM_FORCE_RADIANS
)

target_include_directories(${NAME} PUBLIC
  ${GLAD_INCLUDE_DIR}
  ${GLM_INCLUDE_DIR}
  ${MSDFGEN_INCLUDE_DIR}
)

target_link_directories(${NAME} PUBLIC
  ${MSDFGEN_LIBRARIES}
)

target_link_libraries(${NAME} PUBLIC
  OpenGL::GL
  comctl32.lib
  msimg32.lib
  Dwmapi.lib
  UxTheme
  msdf-atlas-gen
)

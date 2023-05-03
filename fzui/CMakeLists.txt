cmake_minimum_required(VERSION 3.20.0)

# Platform specifics
if (WIN32)
  set(PLATFORM "windows")
  add_definitions(-DFZ_WIN32 -DWIN32 -D_WINDOWS -D_UNICODE -DUNICODE)
  set(GLAD_INCLUDE_DIR ${CMAKE_HOME_DIRECTORY}/vendor/glad/Windows/include)
elseif(APPLE)
  set(PLATFORM "macos")
  enable_language(OBJCXX CXX)
  add_definitions(-DFZ_MACOS -DGL_SILENCE_DEPRECATION)
  set(GLAD_INCLUDE_DIR ${CMAKE_HOME_DIRECTORY}/vendor/glad/MacOS/include)
endif()

set(NAME fzui)
set(SOURCE_DIR ${CMAKE_HOME_DIRECTORY}/${NAME}/src) 
set(INCLUDE_DIR ${CMAKE_HOME_DIRECTORY}/${NAME}/include) 

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

# 1. Find Vulkan SDK
if (DEFINED VULKAN_SDK_PATH)
  set(Vulkan_INCLUDE_DIRS "${VULKAN_SDK_PATH}/Include") # 1.1 Make sure this include path is correct
  set(Vulkan_LIBRARIES "${VULKAN_SDK_PATH}/Lib") # 1.2 Make sure lib path is correct
  set(Vulkan_FOUND "True")
else()
  find_package(Vulkan REQUIRED) # throws error if could not find Vulkan
  message(STATUS "Found Vulkan: $ENV{VULKAN_SDK}")
endif()
if (NOT Vulkan_FOUND)
  message(FATAL_ERROR "Could not find Vulkan library!")
else()
  message(STATUS "Using vulkan lib at: ${Vulkan_LIBRARIES}")
endif()

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
if(WIN32)
  file(GLOB_RECURSE SOURCE_FILES
  ${SOURCE_DIR}/common/*.cpp
  ${SOURCE_DIR}/common/*.c
  ${SOURCE_DIR}/windows/*.cpp
  ${SOURCE_DIR}/windows/*.c
  )
  file(GLOB_RECURSE HEADER_FILES
  ${INCLUDE_DIR}/data/*.hpp
  ${INCLUDE_DIR}/data/*.h
  ${INCLUDE_DIR}/windows/*.hpp
  ${INCLUDE_DIR}/windows/*.h
  ${INCLUDE_DIR}/math/*.hpp
  ${INCLUDE_DIR}/math/*.h
  ${INCLUDE_DIR}/rendering/*.hpp
  ${INCLUDE_DIR}/rendering/*.h
  ${INCLUDE_DIR}/vendor/*.hpp
  ${INCLUDE_DIR}/vendor/*.h
  ${INCLUDE_DIR}/application.hpp
  ${INCLUDE_DIR}/color.hpp
  ${INCLUDE_DIR}/core.hpp
  ${INCLUDE_DIR}/fzui.hpp
  ${INCLUDE_DIR}/mouse.hpp
  ${INCLUDE_DIR}/uiButton.hpp
  ${INCLUDE_DIR}/uiContainer.hpp
  ${INCLUDE_DIR}/uiElement.hpp
  ${INCLUDE_DIR}/uiImage.hpp
  ${INCLUDE_DIR}/uiImageButton.hpp
  ${INCLUDE_DIR}/uiLabel.hpp
  ${INCLUDE_DIR}/vertex.hpp
  ${INCLUDE_DIR}/window.hpp
  )
elseif(APPLE)
  file(GLOB_RECURSE SOURCE_FILES
    ${SOURCE_DIR}/common/*.cpp
    ${SOURCE_DIR}/common/*.c
    ${SOURCE_DIR}/common/*.mm
    ${SOURCE_DIR}/macos/*.cpp
    ${SOURCE_DIR}/macos/*.c
    ${SOURCE_DIR}/macos/*.mm
  )
  file(GLOB_RECURSE HEADER_FILES
    ${INCLUDE_DIR}/data/*.hpp
    ${INCLUDE_DIR}/data/*.h
    ${INCLUDE_DIR}/macos/*.hpp
    ${INCLUDE_DIR}/macos/*.h
    ${INCLUDE_DIR}/math/*.hpp
    ${INCLUDE_DIR}/math/*.h
    ${INCLUDE_DIR}/rendering/*.hpp
    ${INCLUDE_DIR}/rendering/*.h
    ${INCLUDE_DIR}/vendor/*.hpp
    ${INCLUDE_DIR}/vendor/*.h
    ${INCLUDE_DIR}/application.hpp
    ${INCLUDE_DIR}/color.hpp
    ${INCLUDE_DIR}/core.hpp
    ${INCLUDE_DIR}/fzui.hpp
    ${INCLUDE_DIR}/mouse.hpp
    ${INCLUDE_DIR}/uiButton.hpp
    ${INCLUDE_DIR}/uiContainer.hpp
    ${INCLUDE_DIR}/uiElement.hpp
    ${INCLUDE_DIR}/uiImage.hpp
    ${INCLUDE_DIR}/uiImageButton.hpp
    ${INCLUDE_DIR}/uiLabel.hpp
    ${INCLUDE_DIR}/vertex.hpp
    ${INCLUDE_DIR}/window.hpp
  )
endif()

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
  ${Vulkan_INCLUDE_DIRS}
  ${MSDFGEN_INCLUDE_DIR}
)

target_link_directories(${NAME} PUBLIC
  ${Vulkan_LIBRARIES}
  ${MSDFGEN_LIBRARIES}
)

if (WIN32)
  target_link_libraries(${NAME} PUBLIC
    OpenGL::GL
    comctl32.lib
    msimg32.lib
    Dwmapi.lib
    UxTheme
    msdf-atlas-gen
    ${Vulkan_LIBRARIES}
  )
elseif(APPLE)
  target_link_libraries(${NAME} PUBLIC
    OpenGL::GL
    msdf-atlas-gen
    "-framework Cocoa"
  )
endif()

# ------ Compiling shader files to SPIRV format ------
find_program(GLSL_VALIDATOR glslangValidator HINTS 
  ${Vulkan_GLSLANG_VALIDATOR_EXECUTABLE} 
  /usr/bin 
  /usr/local/bin 
  ${VULKAN_SDK_PATH}/Bin
  ${VULKAN_SDK_PATH}/Bin32
  $ENV{VULKAN_SDK}/Bin/ 
  $ENV{VULKAN_SDK}/Bin32/
)

# Retrieve all vertex and fragment shaders
file (GLOB_RECURSE GLSL_SOURCE_FILES
  "${CMAKE_HOME_DIRECTORY}/assets/shaders/*.vert"
  "${CMAKE_HOME_DIRECTORY}/assets/shaders/*.frag"
)

foreach(GLSL ${GLSL_SOURCE_FILES})
  get_filename_component(FILE_NAME ${GLSL} NAME)
  set(SPIRV "${CMAKE_HOME_DIRECTORY}/assets/shaders/${FILE_NAME}.spv")
  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
    DEPENDS ${GLSL})
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
  Shaders
  DEPENDS ${SPIRV_BINARY_FILES}
)

install(TARGETS ${NAME}
  LIBRARY DESTINATION ${CMAKE_BINARY_DIR}/lib
  ARCHIVE DESTINATION ${CMAKE_BINARY_DIR}/lib)

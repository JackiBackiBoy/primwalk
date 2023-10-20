cmake_minimum_required(VERSION 3.20.0)

set(NAME primwalk)
set(SOURCE_DIR ${CMAKE_HOME_DIRECTORY}/${NAME}/src) 
set(INCLUDE_DIR ${CMAKE_HOME_DIRECTORY}/${NAME}/include) 

set(SOURCE_FILES
  ${SOURCE_DIR}/common/application.cpp
  ${SOURCE_DIR}/common/color.cpp
  ${SOURCE_DIR}/common/shader.cpp
  ${SOURCE_DIR}/common/components/camera.cpp
  ${SOURCE_DIR}/common/components/component.cpp
  ${SOURCE_DIR}/common/components/entity.cpp
  ${SOURCE_DIR}/common/data/font.cpp
  ${SOURCE_DIR}/common/data/model.cpp
  ${SOURCE_DIR}/common/data/mesh.cpp
  ${SOURCE_DIR}/common/input/input.cpp
  ${SOURCE_DIR}/common/input/rawInput.cpp
  ${SOURCE_DIR}/common/managers/componentManager.cpp
  ${SOURCE_DIR}/common/managers/entityManager.cpp
  ${SOURCE_DIR}/common/managers/resourceManager.cpp
  ${SOURCE_DIR}/common/managers/systemManager.cpp
  ${SOURCE_DIR}/common/math/hitbox.cpp
  ${SOURCE_DIR}/common/math/pwmath.cpp
  ${SOURCE_DIR}/common/rendering/buffer.cpp
  ${SOURCE_DIR}/common/rendering/descriptors.cpp
  ${SOURCE_DIR}/common/rendering/framebuffer.cpp
  ${SOURCE_DIR}/common/rendering/graphicsDevice_Vulkan.cpp
  ${SOURCE_DIR}/common/rendering/graphicsPipeline.cpp
  ${SOURCE_DIR}/common/rendering/image.cpp
  ${SOURCE_DIR}/common/rendering/renderer.cpp
  ${SOURCE_DIR}/common/rendering/renderpass.cpp
  ${SOURCE_DIR}/common/rendering/texture2D.cpp
  ${SOURCE_DIR}/common/rendering/systems/uiRenderSystem.cpp
  ${SOURCE_DIR}/common/systems/renderSystem3d.cpp
  ${SOURCE_DIR}/common/systems/system.cpp
  ${SOURCE_DIR}/common/ui/GUI.cpp
  ${SOURCE_DIR}/common/ui/menuWidget.cpp
  ${SOURCE_DIR}/common/ui/subView.cpp
  ${SOURCE_DIR}/common/ui/uiButton.cpp
  ${SOURCE_DIR}/common/ui/uiContainer.cpp
  ${SOURCE_DIR}/common/ui/uiElement.cpp
  ${SOURCE_DIR}/common/ui/uiEvent.cpp
  ${SOURCE_DIR}/common/ui/uiIconButton.cpp
  ${SOURCE_DIR}/common/ui/uiImage.cpp
  ${SOURCE_DIR}/common/ui/uiLabel.cpp
  ${SOURCE_DIR}/common/ui/uiListBox.cpp
  ${SOURCE_DIR}/common/ui/uiPanel.cpp
  ${SOURCE_DIR}/common/ui/uiSlider.cpp
  ${SOURCE_DIR}/common/ui/uiTextField.cpp

  ${SOURCE_DIR}/windows/resource.hpp
  ${SOURCE_DIR}/windows/core/window_win32.cpp
)

set(HEADER_FILES
  ${INCLUDE_DIR}/${NAME}/application.hpp
  ${INCLUDE_DIR}/${NAME}/color.hpp
  ${INCLUDE_DIR}/${NAME}/core.hpp
  ${INCLUDE_DIR}/${NAME}/hitbox.hpp
  ${INCLUDE_DIR}/${NAME}/mouseButtons.hpp
  ${INCLUDE_DIR}/${NAME}/primwalk.hpp
  ${INCLUDE_DIR}/${NAME}/window.hpp
  ${INCLUDE_DIR}/${NAME}/components/camera.hpp
  ${INCLUDE_DIR}/${NAME}/components/component.hpp
  ${INCLUDE_DIR}/${NAME}/components/componentArray.hpp
  ${INCLUDE_DIR}/${NAME}/components/entity.hpp
  ${INCLUDE_DIR}/${NAME}/components/renderable.hpp
  ${INCLUDE_DIR}/${NAME}/components/tag.hpp
  ${INCLUDE_DIR}/${NAME}/components/transform.hpp
  ${INCLUDE_DIR}/${NAME}/data/font.hpp
  ${INCLUDE_DIR}/${NAME}/data/fontWeight.hpp
  ${INCLUDE_DIR}/${NAME}/data/mesh.hpp
  ${INCLUDE_DIR}/${NAME}/data/model.hpp
  ${INCLUDE_DIR}/${NAME}/data/shader.hpp
  ${INCLUDE_DIR}/${NAME}/input/keycode.hpp
  ${INCLUDE_DIR}/${NAME}/input/input.hpp
  ${INCLUDE_DIR}/${NAME}/input/rawInput.hpp
  ${INCLUDE_DIR}/${NAME}/managers/componentManager.hpp
  ${INCLUDE_DIR}/${NAME}/managers/entityManager.hpp
  ${INCLUDE_DIR}/${NAME}/managers/resourceManager.hpp
  ${INCLUDE_DIR}/${NAME}/managers/systemManager.hpp
  ${INCLUDE_DIR}/${NAME}/math/pwmath.hpp
  ${INCLUDE_DIR}/${NAME}/math/vector2d.hpp
  ${INCLUDE_DIR}/${NAME}/rendering/buffer.hpp
  ${INCLUDE_DIR}/${NAME}/rendering/descriptors.hpp
  ${INCLUDE_DIR}/${NAME}/rendering/framebuffer.hpp
  ${INCLUDE_DIR}/${NAME}/rendering/frameInfo.hpp
  ${INCLUDE_DIR}/${NAME}/rendering/graphicsAPI.hpp
  ${INCLUDE_DIR}/${NAME}/rendering/graphicsDevice.hpp
  ${INCLUDE_DIR}/${NAME}/rendering/graphicsDevice_Vulkan.hpp
  ${INCLUDE_DIR}/${NAME}/rendering/graphicsPipeline.hpp
  ${INCLUDE_DIR}/${NAME}/rendering/image.hpp
  ${INCLUDE_DIR}/${NAME}/rendering/renderer.hpp
  ${INCLUDE_DIR}/${NAME}/rendering/renderpass.hpp
  ${INCLUDE_DIR}/${NAME}/rendering/texture2D.hpp
  ${INCLUDE_DIR}/${NAME}/rendering/vertex.hpp
  ${INCLUDE_DIR}/${NAME}/rendering/vertex3d.hpp
  ${INCLUDE_DIR}/${NAME}/rendering/systems/uiRenderSystem.hpp
  ${INCLUDE_DIR}/${NAME}/systems/renderSystem3d.hpp
  ${INCLUDE_DIR}/${NAME}/systems/system.hpp
  ${INCLUDE_DIR}/${NAME}/ui/GUI.hpp
  ${INCLUDE_DIR}/${NAME}/ui/menuWidget.hpp
  ${INCLUDE_DIR}/${NAME}/ui/mouseCursor.hpp
  ${INCLUDE_DIR}/${NAME}/ui/subView.hpp
  ${INCLUDE_DIR}/${NAME}/ui/uiButton.hpp
  ${INCLUDE_DIR}/${NAME}/ui/uiContainer.hpp
  ${INCLUDE_DIR}/${NAME}/ui/uiElement.hpp
  ${INCLUDE_DIR}/${NAME}/ui/uiEvent.hpp
  ${INCLUDE_DIR}/${NAME}/ui/uiIconButton.hpp
  ${INCLUDE_DIR}/${NAME}/ui/uiImage.hpp
  ${INCLUDE_DIR}/${NAME}/ui/uiLabel.hpp
  ${INCLUDE_DIR}/${NAME}/ui/uiListBox.hpp
  ${INCLUDE_DIR}/${NAME}/ui/uiPanel.hpp
  ${INCLUDE_DIR}/${NAME}/ui/uiSlider.hpp
  ${INCLUDE_DIR}/${NAME}/ui/uiTextField.hpp
  ${INCLUDE_DIR}/${NAME}/vendor/stb/stb_image.hpp

  ${INCLUDE_DIR}/${NAME}/windows/resource.hpp
  ${INCLUDE_DIR}/${NAME}/windows/core/window_win32.hpp
)

# Platform specifics
if (WIN32)
  set(CMAKE_INSTALL_PREFIX ${CMAKE_HOME_DIRECTORY}/build/install_config)
  set(PLATFORM "windows")
  add_definitions(-DPW_WIN32 -DWIN32 -D_WINDOWS -D_UNICODE -DUNICODE -DWIN32_LEAN_AND_MEAN -DNOMINMAX)
elseif(APPLE)
  set(PLATFORM "macos")
  enable_language(OBJCXX CXX)
  add_definitions(-DPW_MACOS)
endif()

# Binary output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Compiler specification
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

project(${NAME})

include_directories(
  ${SOURCE_DIR}
  ${INCLUDE_DIR}
  ${CMAKE_HOME_DIRECTORY}/vendor/stb
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

add_library(${NAME} SHARED ${SOURCE_FILES} ${HEADER_FILES} ${STB_FILES})

# Eliminate target prefixes
set_target_properties(${NAME} PROPERTIES PREFIX "")
set_target_properties(${NAME} PROPERTIES IMPORT_PREFIX "")

#target_compile_features(${NAME} PUBLIC cxx_std_17)
#target_compile_options(${NAME} PUBLIC "-std=c++17")

if (WIN32)
  target_compile_definitions(${NAME} PRIVATE
    PW_BUILD_LIB
    BASE_DIR="${CMAKE_HOME_DIRECTORY}/"
    GLM_FORCE_RADIANS
    WINVER=0x0A00
  )
elseif(APPLE)
  target_compile_definitions(${NAME} PRIVATE
    PW_BUILD_LIB
    BASE_DIR="${CMAKE_HOME_DIRECTORY}/"
    GLM_FORCE_RADIANS
  )
endif()


target_include_directories(${NAME} PUBLIC
  ${GLM_INCLUDE_DIR}
  ${Vulkan_INCLUDE_DIRS}
  ${ASSIMP_INCLUDE_DIR}
  ${MSDFGEN_INCLUDE_DIR}
  ${STB_IMAGE_INCLUDE_DIR}
)

target_link_directories(${NAME} PUBLIC
  ${ASSIMP_LIBRARIES}
  ${MSDFGEN_LIBRARIES}
)

if (WIN32)
  target_link_libraries(${NAME}
    assimp
    msdf-atlas-gen
    ${Vulkan_LIBRARIES}
  )
elseif(APPLE)
  target_link_libraries(${NAME}
    msdf-atlas-gen
    "-framework Cocoa"
    ${Vulkan_LIBRARIES}
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

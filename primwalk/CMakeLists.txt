cmake_minimum_required(VERSION 3.20.0)

set(NAME primwalk)
set(SOURCE_DIR ${CMAKE_HOME_DIRECTORY}/${NAME}/src) 

set(SOURCE_FILES
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/core.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/primwalk.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/window.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/application.cpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/application.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/color.cpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/color.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/components/camera.cpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/components/camera.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/components/component.cpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/components/component.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/components/componentArray.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/components/directionLight.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/components/entity.cpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/components/entity.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/components/pointLight.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/components/renderable.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/components/tag.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/components/transform.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/data/font.cpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/data/font.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/data/fontWeight.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/data/model.cpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/data/model.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/data/mesh.cpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/data/mesh.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/data/shader.cpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/data/shader.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/input/input.cpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/input/input.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/input/keycode.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/input/mouseButtons.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/input/rawInput.cpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/input/rawInput.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/managers/componentManager.cpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/managers/componentManager.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/managers/entityManager.cpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/managers/entityManager.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/managers/resourceManager.cpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/managers/resourceManager.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/managers/systemManager.cpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/managers/systemManager.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/math/hitbox.cpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/math/hitbox.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/math/pwmath.cpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/math/pwmath.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/rendering/buffer.cpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/rendering/buffer.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/rendering/descriptors.cpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/rendering/descriptors.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/rendering/framebuffer.cpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/rendering/framebuffer.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/rendering/frameInfo.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/rendering/graphicsAPI.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/rendering/graphicsDevice.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/rendering/graphicsDevice_Vulkan.cpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/rendering/graphicsDevice_Vulkan.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/rendering/graphicsPipeline.cpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/rendering/graphicsPipeline.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/rendering/image.cpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/rendering/image.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/rendering/renderer.cpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/rendering/renderer.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/rendering/renderpass.cpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/rendering/renderpass.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/rendering/texture2D.cpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/rendering/texture2D.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/rendering/vertex.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/rendering/vertex3d.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/rendering/systems/uiRenderSystem.cpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/rendering/systems/uiRenderSystem.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/systems/renderSystem3d.cpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/systems/renderSystem3d.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/systems/system.cpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/systems/system.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/ui/GUI.cpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/ui/GUI.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/ui/menuWidget.cpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/ui/menuWidget.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/ui/mouseCursor.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/ui/subView.cpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/ui/subView.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/ui/uiButton.cpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/ui/uiButton.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/ui/uiContainer.cpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/ui/uiContainer.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/ui/uiElement.cpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/ui/uiElement.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/ui/uiEvent.cpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/ui/uiEvent.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/ui/uiIconButton.cpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/ui/uiIconButton.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/ui/uiImage.cpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/ui/uiImage.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/ui/uiLabel.cpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/ui/uiLabel.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/ui/uiListBox.cpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/ui/uiListBox.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/ui/uiPanel.cpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/ui/uiPanel.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/ui/uiSlider.cpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/ui/uiSlider.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/ui/uiTextField.cpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/common/ui/uiTextField.hpp

  # Windows specific
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/windows/resource.hpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/windows/window_win32.cpp
  ${CMAKE_HOME_DIRECTORY}/primwalk/src/windows/window_win32.hpp
)

# Platform specifics
if (WIN32)
  set(CMAKE_INSTALL_PREFIX ${CMAKE_HOME_DIRECTORY}/build/install_config)
  set(PLATFORM "windows")
  add_definitions(-DPW_WIN32 -DWIN32 -D_WINDOWS -D_UNICODE -DUNICODE -DWIN32_LEAN_AND_MEAN -DNOMINMAX)
elseif(APPLE)
set(CMAKE_INSTALL_PREFIX ${CMAKE_HOME_DIRECTORY}/build/install_config)
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

add_library(${NAME} SHARED ${SOURCE_FILES} ${STB_FILES})

# Eliminate target prefixes
set_target_properties(${NAME} PROPERTIES PREFIX "")
set_target_properties(${NAME} PROPERTIES IMPORT_PREFIX "")

#target_compile_features(${NAME} PUBLIC cxx_std_17)
#target_compile_options(${NAME} PUBLIC "-std=c++17")

if (WIN32)
  target_compile_definitions(${NAME} PRIVATE
    PW_BUILD_LIB
    BASE_DIR="${CMAKE_HOME_DIRECTORY}/"
    GLM_FORCE_DEPTH_ZERO_TO_ONE
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

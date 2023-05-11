#pragma once

// std
#include <string>
#include <unordered_map>
#include <vector>

// FZUI
#include "fzui/core.hpp"

// vendor
#include <glm/glm.hpp>

namespace fz {
  // ------ Vulkan ------
  class FZ_API Shader_Vulkan {
    public:
      Shader_Vulkan() {};
      ~Shader_Vulkan() {};

      static std::vector<char> readFile(const std::string& path);

    private:

  };
}

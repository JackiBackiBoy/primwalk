#pragma once

// std
#include <string>
#include <unordered_map>
#include <vector>

// primwalk
#include "primwalk/core.hpp"

// vendor
#include <glm/glm.hpp>

namespace pw {
  // ------ Vulkan ------
  class PW_API Shader_Vulkan {
    public:
      Shader_Vulkan() {};
      ~Shader_Vulkan() {};

      static std::vector<char> readFile(const std::string& path);

    private:

  };
}

#pragma once

// primwalk
#include "primwalk/core.hpp"

// std
#include <cstdint>

// vendor
#include <vulkan/vulkan.h>

namespace pw {
  struct PW_API Mesh {
    uint32_t indices;
    uint32_t materialIndex;
    uint32_t baseVertex;
    uint32_t baseIndex;
  };
}
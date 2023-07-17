#ifndef PW_GRAPHICS_API_HEADER
#define PW_GRAPHICS_API_HEADER

// primwalk
#include "primwalk/core.hpp" 

namespace fz {
  enum class GraphicsAPI {
    OpenGL,
    Vulkan,
    DX11,
    DX12,
  };
}
#endif
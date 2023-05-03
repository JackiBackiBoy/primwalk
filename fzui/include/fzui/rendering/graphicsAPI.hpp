#ifndef FZ_GRAPHICS_API_HEADER
#define FZ_GRAPHICS_API_HEADER

// FZUI
#include "fzui/core.hpp" 

namespace fz {
  enum class FZ_API GraphicsAPI {
    OpenGL,
    Vulkan,
    DX11,
    DX12,
  };
}
#endif
#ifndef FZ_UI_FONT_MANAGER
#define FZ_UI_FONT_MANAGER

// FZUI
#include "fzui/core.hpp"
#include "fzui/data/font.hpp"
#include "fzui/rendering/graphicsDevice.hpp"

// std
#include <memory>
#include <vector>

namespace fz {
  class FZ_API FontManager {
    public:
      // Forbid copy and move semantics
      FontManager(const FontManager&) = delete;
      FontManager& operator=(const FontManager&) = delete;
      FontManager(FontManager&&) = delete;
      FontManager& operator=(FontManager&&) = delete;
      ~FontManager() = default;

      static FontManager& Instance();

      Font& loadFont(GraphicsDevice_Vulkan& device, const std::string& path, double fontSize);

    private:
      FontManager();

      std::vector<std::unique_ptr<Font>> m_Fonts;
  };
}
#endif
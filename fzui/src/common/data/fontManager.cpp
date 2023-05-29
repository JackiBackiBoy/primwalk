#include "fzui/data/fontManager.hpp"

namespace fz {

  FontManager& FontManager::Instance()
  {
    // Meyers singleton
    static FontManager instance;
    return instance;
  }

  Font& FontManager::loadFont(GraphicsDevice_Vulkan& device, const std::string& path, double fontSize)
  {
    auto font = Font::createFromFile(device, path, fontSize);
    auto& ref = *font;

    m_Fonts.emplace_back(std::move(font));
    return ref;
  }

  FontManager::FontManager()
  {

  }

}
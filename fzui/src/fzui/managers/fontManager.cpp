#include "fzui/managers/fontManager.hpp"

namespace fz {
  FontManager::FontManager() {
  }

  FontManager::~FontManager() {
  }

  FontManager& FontManager::getInstance() {
    static FontManager instance;
    return instance;
  }

  HFONT FontManager::getFont(const std::string& name, const int& size, const int& flags) {
    // Load font if not already loaded

    FontInfo fontInfo;
    fontInfo.size = size;
    fontInfo.flags = flags;
    fontInfo.name = name;

    loadFont(fontInfo);

    return m_Fonts[fontInfo];
  }

  void FontManager::loadFont(const FontInfo& fontInfo) {
    if (m_Fonts.find(fontInfo) == m_Fonts.end()) { // key does not exist in map
      m_Fonts.insert({ fontInfo, CreateFont(fontInfo.size, 0, 0, 0, fontInfo.flags, 0, 0, 0, 0, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI") });
    }
  }
}

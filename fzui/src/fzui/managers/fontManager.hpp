#pragma once

// std
#include <string>
#include <unordered_map>

// FZUI
#include "fzui/core/core.hpp"
#include "fzui/data/fonts/fontInfo.hpp"

// Windows
#include <windows.h>

namespace fz {
  class FZ_API FontManager {
    public:
      FontManager(const FontManager&) = delete;
      FontManager(FontManager&&) = delete;
      FontManager& operator=(const FontManager&) = delete;
      FontManager& operator=(FontManager&&) = delete;

      static FontManager& getInstance();
      HFONT getFont(const std::string& name, const int& size, const int& flags);

    private:
      FontManager();
      ~FontManager();

      void loadFont(const FontInfo& fontInfo);

      std::unordered_map<FontInfo, HFONT> m_Fonts;
  };
}

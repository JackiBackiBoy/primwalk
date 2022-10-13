#pragma once

#include <string>
#include <unordered_map>
#include <windows.h>
#include "fzui/core/core.hpp"
#include "fzui/data/fonts/fontInfo.hpp"

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
      void loadFont(const FontInfo& fontInfo);
      FontManager();
      ~FontManager();

      std::unordered_map<FontInfo, HFONT> m_Fonts;
  };
}

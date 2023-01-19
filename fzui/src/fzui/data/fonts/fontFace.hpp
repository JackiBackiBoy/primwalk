#pragma once

// std
#include <memory>
#include <unordered_map>
#include <string>

// FZUI
#include "fzui/core/core.hpp"
#include "fzui/data/texture.hpp"
#include "fzui/data/fonts/fontCharacter.hpp"

namespace fz {
  class FontFace {
    public:
      ~FontFace();

      static FontFace* create(const std::string& fontPath);

      // Getters
      int getBoundingBoxY() const;
      int getLowestOffsetY() const;
      FontCharacter getFontChar(const char& character) const;
      Texture* m_TextureAtlas = nullptr;

    private:
      FontFace() {};

      std::unordered_map<int, FontCharacter> m_FontCharacters;
      int m_FontBoundingBoxY = 0;
      int m_FontLowestOffsetY = 0;
  };
}

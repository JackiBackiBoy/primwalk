#pragma once

// std
#include <memory>
#include <unordered_map>
#include <string>

// vendor
#include <glm/glm.hpp>

// FZUI
#include "fzui/core.hpp"
#include "fzui/windows/data/texture.hpp"
#include "fzui/windows/data/fonts/fontCharacter.hpp"

namespace fz {
  class FZ_API FontFace {
    public:
      static FontFace* create(const std::string& fontPath);

      // Getters
      int getMaxHeight() const;
      GlyphData* getGlyph(const char& c);
      unsigned int m_TextureAtlas = 0;

    private:
      ~FontFace();
      FontFace() {};

      std::unordered_map<char, GlyphData*> m_GlyphData;
      glm::ivec2 m_BoundingBox = { 0, 0 };
      int m_MaxHeight = 0;
      int m_FontBoundingBoxY = 0;
      int m_FontLowestOffsetY = 0;
  };
}

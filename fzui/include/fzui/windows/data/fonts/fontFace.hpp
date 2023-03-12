#pragma once

// std
#include <memory>
#include <unordered_map>
#include <string>
#include <vector>

// vendor
#include <glm/glm.hpp>
#include <msdf-atlas-gen/msdf-atlas-gen.h>

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
      GlyphData* getGlyph(const msdf_atlas::unicode_t& c);
      unsigned int m_TextureAtlas = 0;
      std::vector<msdf_atlas::GlyphGeometry> glyphs;
      int atlasWidth = 0;
      int atlasHeight = 0;
      double m_FontSize = 0.0f;

    private:
      ~FontFace();
      FontFace() {};

      bool submitAtlasBitmapAndLayout(const msdf_atlas::BitmapAtlasStorage<msdf_atlas::byte, 3>& atlas,
                                      std::vector<msdf_atlas::GlyphGeometry> glyphs);

      std::unordered_map<msdf_atlas::unicode_t, GlyphData*> m_GlyphData;
      glm::ivec2 m_BoundingBox = { 0, 0 };
      int m_MaxHeight = 0;
      int m_FontBoundingBoxY = 0;
      int m_FontLowestOffsetY = 0;
      
  };
}

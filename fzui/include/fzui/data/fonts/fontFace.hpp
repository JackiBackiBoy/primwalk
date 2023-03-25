#pragma once

// std
#include <memory>
#include <unordered_map>
#include <string>
#include <vector>

// vendor
#include <glm/glm.hpp>
#include "msdf-atlas-gen/msdf-atlas-gen.h"

// FZUI
#include "fzui/core.hpp"
#include "fzui/data/texture.hpp"
#include "fzui/data/fonts/fontCharacter.hpp"

namespace fz {
  class FZ_API FontFace {
    public:
      ~FontFace();

      static FontFace* create(const std::string& fontPath, const double& fontSize);

      // Getters
      int getTextWidth(const std::string& text, const float& fontSize);
      int getMaxHeight() const;
      unsigned int getTextureID() const;
      GlyphData getGlyph(const msdf_atlas::unicode_t& c);
      const double& getFontSize() const;
      
      std::vector<msdf_atlas::GlyphGeometry> glyphs;
      int atlasWidth = 0;
      int atlasHeight = 0;

    private:
      FontFace() {};

      bool submitAtlasBitmapAndLayout(const msdf_atlas::BitmapAtlasStorage<msdf_atlas::byte, 3>& atlas,
                                      std::vector<msdf_atlas::GlyphGeometry> glyphs);

      std::unordered_map<msdf_atlas::unicode_t, GlyphData> m_GlyphData;
      glm::ivec2 m_BoundingBox = { 0, 0 };
      int m_MaxHeight = 0;
      int m_FontBoundingBoxY = 0;
      int m_FontLowestOffsetY = 0;
      double m_FontSize = 0.0f;
      unsigned int m_TextureAtlas = 0;
  };
}

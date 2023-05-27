#ifndef FZ_FONT_HEADER
#define FZ_FONT_HEADER

// FZUI
#include "fzui/core.hpp"
#include "fzui/rendering/graphicsDevice.hpp"
#include "fzui/rendering/texture2D.hpp"

// std
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// vendor
#include <glm/glm.hpp>
#include "msdf-atlas-gen/msdf-atlas-gen.h"

namespace fz {
  struct FZ_API GlyphData {
    unsigned int width = 0;
    unsigned int height = 0;
    double bearingX = 0;
    double bearingY = 0;
    double advanceX = 0;
    float texLeftX = 0.0f;
    float texTopY = 0.0f;
    float texRightX = 0.0f;
    float texBottomY = 0.0f;
    double bearingUnderline = 0.0;
  };

  class FZ_API Font {
    public:
      Font(GraphicsDevice_Vulkan& device, const std::string& path, double fontSize);
      ~Font() {};

      // Getters
      const Texture2D& getTextureAtlas() const;

      static std::unique_ptr<Font> createFromFile(GraphicsDevice_Vulkan& device, const std::string& path, double fontSize);

    private:
      GraphicsDevice_Vulkan& m_Device;

      bool submitAtlasBitmapAndLayout(
        const msdf_atlas::BitmapAtlasStorage<msdf_atlas::byte, 4>& atlas,
        std::vector<msdf_atlas::GlyphGeometry> glyphs);

      std::unordered_map<msdf_atlas::unicode_t, GlyphData> m_GlyphData;
      glm::ivec2 m_BoundingBox = { 0, 0 };
      int m_MaxHeight = 0;
      int m_FontBoundingBoxY = 0;
      int m_FontLowestOffsetY = 0;
      double m_FontSize = 0.0f;
      std::unique_ptr<Texture2D> m_TextureAtlas;
      std::vector<msdf_atlas::GlyphGeometry> glyphs;
      int atlasWidth = 0;
      int atlasHeight = 0;
  };
}

#endif
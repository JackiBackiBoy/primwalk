#include "fzui/data/font.hpp"

namespace fz {

  Font::Font(GraphicsDevice_Vulkan& device, const std::string& path, double fontSize) : m_Device{device}
  {
    glyphs = std::vector<msdf_atlas::GlyphGeometry>();
    m_GlyphData = std::unordered_map<msdf_atlas::unicode_t, GlyphData>();
    m_FontSize = fontSize;
    std::string truePath = BASE_DIR + path;

    bool success = false;
    // Initialize instance of FreeType library
    if (msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype()) {
      // Load font file
      if (msdfgen::FontHandle* font = msdfgen::loadFont(ft, truePath.c_str())) {
        // Storage for glyph geometry and their coordinates in the atlas
        // FontGeometry is a helper class that loads a set of glyphs from a single font.
        msdf_atlas::FontGeometry fontGeometry(&glyphs);
        fontGeometry.loadCharset(font, 1.0, msdf_atlas::Charset::ASCII);

        // Apply MSDF edge coloring. See edge-coloring.h for other coloring strategies.
        const double maxCornerAngle = 3.0;
        for (msdf_atlas::GlyphGeometry& glyph : glyphs)
          glyph.edgeColoring(&msdfgen::edgeColoringInkTrap, maxCornerAngle, 0);

        // TightAtlasPacker class computes the layout of the atlas.
        msdf_atlas::TightAtlasPacker packer;

        packer.setDimensionsConstraint(msdf_atlas::TightAtlasPacker::DimensionsConstraint::SQUARE);
        packer.setMinimumScale(m_FontSize);
        packer.setScale(m_FontSize); // TODO: Make dynamic scale
        packer.setPixelRange(2.5);
        packer.setMiterLimit(1.0);

        // Compute atlas layout - pack glyphs
        packer.pack(glyphs.data(), glyphs.size());
        // Get final atlas dimensions
        int width = 0, height = 0;
        packer.getDimensions(width, height);
        // The ImmediateAtlasGenerator class facilitates the generation of the atlas bitmap.
        msdf_atlas::ImmediateAtlasGenerator<
          float, // pixel type of buffer for individual glyphs depends on generator function
          4, // number of atlas color channels
          &msdf_atlas::mtsdfGenerator, // function to generate bitmaps for individual glyphs
          msdf_atlas::BitmapAtlasStorage<msdf_atlas::byte, 4> // class that stores the atlas bitmap
          // For example, a custom atlas storage class that stores it in VRAM can be used.
        > generator(width, height);
        // GeneratorAttributes can be modified to change the generator's default settings.
        msdf_atlas::GeneratorAttributes attributes;
        generator.setAttributes(attributes);
        generator.setThreadCount(4);
        // Generate atlas bitmap
        generator.generate(glyphs.data(), glyphs.size());
        // The atlas bitmap can now be retrieved via atlasStorage as a BitmapConstRef.
        // The glyphs array (or fontGeometry) contains positioning data for typesetting text.
        success = submitAtlasBitmapAndLayout(generator.atlasStorage(), glyphs);
        // Cleanup
        msdfgen::destroyFont(font);
      }
      msdfgen::deinitializeFreetype(ft);
    }
  }

  const Texture2D& Font::getTextureAtlas() const
  {
    return *m_TextureAtlas;
  }

  std::unique_ptr<Font> Font::createFromFile(GraphicsDevice_Vulkan& device, const std::string& path, double fontSize)
  {
    return std::make_unique<Font>(device, path, fontSize);
  }

  bool Font::submitAtlasBitmapAndLayout(const msdf_atlas::BitmapAtlasStorage<msdf_atlas::byte, 4>& atlas, std::vector<msdf_atlas::GlyphGeometry> glyphs)
  {
    msdfgen::BitmapConstRef<unsigned char, 4> bitmap = (msdfgen::BitmapConstRef<unsigned char, 4>)atlas;
    atlasWidth = bitmap.width;
    atlasHeight = bitmap.height;

    uint32_t code = 0;

    //glyph
    for (msdf_atlas::GlyphGeometry g : glyphs) {
      double pLeft, pBottom, pRight, pTop;
      double aLeft, aBottom, aRight, aTop;
      g.getQuadPlaneBounds(pLeft, pBottom, pRight, pTop);
      g.getQuadAtlasBounds(aLeft, aBottom, aRight, aTop);

      GlyphData glyphData = GlyphData();
      glyphData.advanceX = g.getAdvance() * m_FontSize;
      glyphData.width = aRight - aLeft;
      glyphData.height = aTop - aBottom;
      glyphData.texLeftX = static_cast<float>(aLeft) / bitmap.width;
      glyphData.texTopY = static_cast<float>(aTop) / bitmap.height;
      glyphData.texRightX = static_cast<float>(aRight) / bitmap.width;
      glyphData.texBottomY = static_cast<float>(aBottom) / bitmap.height;
      glyphData.bearingX = pLeft * m_FontSize;
      glyphData.bearingY = pTop * m_FontSize;
      glyphData.bearingUnderline = pBottom * m_FontSize;

      m_GlyphData.insert({ g.getCodepoint(), glyphData });

      if (glyphData.height > m_MaxHeight) {
        code = g.getCodepoint();
      }

      m_MaxHeight = std::max(m_MaxHeight, (int)((float)glyphData.height + glyphData.bearingUnderline));
    }

    m_TextureAtlas = Texture2D::create(m_Device, atlasWidth, atlasHeight, (unsigned char*)bitmap.pixels);

    return true; // TODO: Make return value actually reflect whether it worked or not
  }

}
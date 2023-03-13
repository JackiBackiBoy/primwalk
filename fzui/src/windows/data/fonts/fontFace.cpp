// FZUI
#include "fzui/windows/data/fonts/fontFace.hpp"

// std
#include <iostream>
#include <algorithm>

// vendor
#include <glad/glad.h>

namespace fz {
  FontFace::~FontFace() {
    
  }

  FontFace* FontFace::create(const std::string& fontPath) {
    FontFace* fontFace = new FontFace();
    fontFace->glyphs = std::vector<msdf_atlas::GlyphGeometry>();
    fontFace->m_GlyphData = std::unordered_map<msdf_atlas::unicode_t, GlyphData*>();
    fontFace->m_FontSize = 12.0;
    std::string truePath = BASE_DIR + fontPath;

    bool success = false;
    // Initialize instance of FreeType library
    if (msdfgen::FreetypeHandle *ft = msdfgen::initializeFreetype()) {
        // Load font file
        if (msdfgen::FontHandle *font = msdfgen::loadFont(ft, truePath.c_str())) {
            // Storage for glyph geometry and their coordinates in the atlas
            // FontGeometry is a helper class that loads a set of glyphs from a single font.
            msdf_atlas::FontGeometry fontGeometry(&fontFace->glyphs);
            fontGeometry.loadCharset(font, 1.0, msdf_atlas::Charset::ASCII);

            // Apply MSDF edge coloring. See edge-coloring.h for other coloring strategies.
            const double maxCornerAngle = 3.0;
            for (msdf_atlas::GlyphGeometry &glyph : fontFace->glyphs)
                glyph.edgeColoring(&msdfgen::edgeColoringInkTrap, maxCornerAngle, 0);

            // TightAtlasPacker class computes the layout of the atlas.
            msdf_atlas::TightAtlasPacker packer;

            packer.setDimensionsConstraint(msdf_atlas::TightAtlasPacker::DimensionsConstraint::SQUARE);
            packer.setMinimumScale(fontFace->m_FontSize);
            packer.setScale(fontFace->m_FontSize);
            packer.setPixelRange(2.0);
            packer.setMiterLimit(1.0);

            // Compute atlas layout - pack glyphs
            packer.pack(fontFace->glyphs.data(), fontFace->glyphs.size());
            // Get final atlas dimensions
            int width = 0, height = 0;
            packer.getDimensions(width, height);
            // The ImmediateAtlasGenerator class facilitates the generation of the atlas bitmap.
            msdf_atlas::ImmediateAtlasGenerator<
                float, // pixel type of buffer for individual glyphs depends on generator function
                3, // number of atlas color channels
                &msdf_atlas::msdfGenerator, // function to generate bitmaps for individual glyphs
                msdf_atlas::BitmapAtlasStorage<msdf_atlas::byte, 3> // class that stores the atlas bitmap
                // For example, a custom atlas storage class that stores it in VRAM can be used.
            > generator(width, height);
            // GeneratorAttributes can be modified to change the generator's default settings.
            msdf_atlas::GeneratorAttributes attributes;
            generator.setAttributes(attributes);
            generator.setThreadCount(4);
            // Generate atlas bitmap
            generator.generate(fontFace->glyphs.data(), fontFace->glyphs.size());
            // The atlas bitmap can now be retrieved via atlasStorage as a BitmapConstRef.
            // The glyphs array (or fontGeometry) contains positioning data for typesetting text.
            success = fontFace->submitAtlasBitmapAndLayout(generator.atlasStorage(), fontFace->glyphs);
            // Cleanup
            msdfgen::destroyFont(font);
        }
        msdfgen::deinitializeFreetype(ft);
    }

    return fontFace;
  }

  bool FontFace::submitAtlasBitmapAndLayout(const msdf_atlas::BitmapAtlasStorage<msdf_atlas::byte, 3>& atlas,
                                            std::vector<msdf_atlas::GlyphGeometry> glyphs) {
    msdfgen::BitmapConstRef<unsigned char, 3> bitmap = (msdfgen::BitmapConstRef<unsigned char, 3>)atlas;
    std::cout << "Bitmap width: " << bitmap.width << std::endl;
    std::cout << "Bitmap height: " << bitmap.height << std::endl;
    atlasWidth = bitmap.width;
    atlasHeight = bitmap.height;

    //glyph
    for (msdf_atlas::GlyphGeometry g : glyphs) {
      double pLeft, pBottom, pRight, pTop;
      double aLeft, aBottom, aRight, aTop;
      g.getQuadPlaneBounds(pLeft, pBottom, pRight, pTop);
      g.getQuadAtlasBounds(aLeft, aBottom, aRight, aTop);

      GlyphData* glyphData = new GlyphData();
      glyphData->advanceX = g.getAdvance();
      glyphData->width = aRight - aLeft;
      glyphData->height = aTop - aBottom;
      glyphData->texLeftX = static_cast<float>(aLeft) / bitmap.width;
      glyphData->texTopY = static_cast<float>(aTop) / bitmap.height;
      glyphData->texRightX = static_cast<float>(aRight) / bitmap.width;
      glyphData->texBottomY = static_cast<float>(aBottom) / bitmap.height;
      glyphData->bearingX = pLeft;
      glyphData->bearingY = pTop;
      glyphData->pl = pLeft;
      glyphData->pb = pBottom;
      glyphData->pr = pRight;
      glyphData->pt = pTop;
      glyphData->il = aLeft;
      glyphData->ib = aBottom;
      glyphData->ir = aRight;
      glyphData->it = aTop;

      m_GlyphData.insert({ g.getCodepoint(), glyphData });

      std::cout << "BearingX: " << pLeft * m_FontSize << std::endl;
      std::cout << "BearingY: " << pTop * m_FontSize << std::endl;
      m_MaxHeight = std::max(m_MaxHeight, (int)glyphData->height);
    }

    std::cout << "Max font height: " << m_MaxHeight << std::endl;

    glBindTexture(GL_TEXTURE_2D, 0); // bind
    glGenTextures(1, &m_TextureAtlas);
    glBindTexture(GL_TEXTURE_2D, m_TextureAtlas);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bitmap.width, bitmap.height, 0, GL_RGB, GL_UNSIGNED_BYTE, bitmap.pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR); // TODO: Dangerous - mipmapping?!
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0); // unbind

    return true; // TODO: Make return value actually reflect whether it worked or not
  }

  // Getters
  int FontFace::getMaxHeight() const {
    return m_MaxHeight;
  }

  GlyphData* FontFace::getGlyph(const msdf_atlas::unicode_t& c) {
    return m_GlyphData[c];
  }
}
// FZUI
#include "fzui/windows/data/fonts/fontFace.hpp"

// std
#include <iostream>

// vendor
#include <glad/glad.h>
#include "ft2build.h"
#include FT_FREETYPE_H

namespace fz {
  FontFace::~FontFace() {

  }

  FontFace* FontFace::create(const std::string& fontPath) {
    FontFace* fontFace = new FontFace();
    fontFace->m_GlyphData = std::unordered_map<char, GlyphData*>();

    FT_Library tempFT;
		if (FT_Init_FreeType(&tempFT))
		{
			std::cout << "Error (Freetype): Could not initialize the FreeType library." << std::endl;
		}

    std::string truePath = BASE_DIR + fontPath;
    FT_Face tempFace;
		if (FT_New_Face(tempFT, truePath.c_str(), 0, &tempFace))
		{
			std::cout << "Error (Freetype): Could not load the desired font." << std::endl;
		}
    else {
			FT_Set_Pixel_Sizes(tempFace, 0, 30);

      glBindTexture(GL_TEXTURE_2D, 0);

      int atlasWidth = 0;
      int atlasOffset = 0;
      int atlasHeight = 0;
      unsigned char numChars = 128;
      unsigned int maxWidth = 0;
      int maxHeight = 0;

      for (unsigned char i = 32; i < 127; i++) {
				// Load the character glyph
				if (FT_Load_Char(tempFace, i, FT_LOAD_RENDER))
				{
					std::cout << "Error (Freetype): Failed to load the glyph." << std::endl;
          continue;
				}

        GlyphData* glyphData = new GlyphData();
        glyphData->width = tempFace->glyph->bitmap.width;
        glyphData->height = tempFace->glyph->bitmap.rows;
        glyphData->bearingX = tempFace->glyph->bitmap_left;
        glyphData->bearingY = tempFace->glyph->bitmap_top;
        glyphData->advanceX = tempFace->glyph->advance.x; // divide by 64 to convert to pixels
        
        fontFace->m_GlyphData.insert({ char(i), glyphData });

        atlasWidth += glyphData->width;
        atlasOffset += 10;
        maxWidth = std::max(maxWidth, glyphData->width);
        maxHeight = std::max(maxHeight, (int)tempFace->glyph->bitmap.rows);
      }

      fontFace->m_MaxHeight = maxHeight;

      // Create a texture atlas for all the glyphs
      int currentX = 0;
      int currentY = 0;

      // We can approximate a square like texture atlas
      int dim = (int)std::ceil(sqrt((float)numChars));

      glBindTexture(GL_TEXTURE_2D, 0);
      glGenTextures(1, &fontFace->m_TextureAtlas);
      glBindTexture(GL_TEXTURE_2D, fontFace->m_TextureAtlas);
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

      glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlasWidth + atlasOffset, maxHeight, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
      
      for (unsigned char i = 32; i < 127; i++) {
        if (FT_Load_Char(tempFace, i, FT_LOAD_RENDER))
				{
					std::cout << "Error (Freetype): Failed to load the glyph." << std::endl;
          continue;
				}

        GlyphData* glyphData = fontFace->m_GlyphData[char(i)];
        glyphData->texLeftX = float(currentX) / float(atlasWidth + atlasOffset);
        glyphData->texTopY = 0.0f;
        glyphData->texRightX = glyphData->texLeftX + float(tempFace->glyph->bitmap.width) / float(atlasWidth + atlasOffset);
        glyphData->texBottomY = float(glyphData->height) / float(maxHeight);

        glTexSubImage2D(GL_TEXTURE_2D, 0, currentX, 0, tempFace->glyph->bitmap.width, tempFace->glyph->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, tempFace->glyph->bitmap.buffer);

        currentX += tempFace->glyph->bitmap.width + 10;
      }

      std::cout << "Atlas width: " << atlasWidth << std::endl;
      std::cout << "Max height: " << maxHeight << std::endl;
      

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glBindTexture(GL_TEXTURE_2D, 0);
		}

		FT_Done_Face(tempFace);
		FT_Done_FreeType(tempFT);

    return fontFace;
  }

  // Getters
  int FontFace::getMaxHeight() const {
    return m_MaxHeight;
  }

  GlyphData* FontFace::getGlyph(const char& c) {
    return m_GlyphData[c];
  }
}
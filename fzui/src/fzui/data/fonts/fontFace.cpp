// FZUI
#include "fzui/data/fonts/fontFace.hpp"

// std
#include <utility>
#include <iostream>
#include <fstream>
#include <sstream>

namespace fz {
  FontFace::~FontFace() {
  }

  FontFace* FontFace::create(const std::string& fontPath) {
    FontFace* fontFace = new FontFace();

    // Load texture atlas from the font path (relative to base directory)
    std::string truePath = BASE_DIR + fontPath;

    // Get the font character data from the .fnt file
    int fontTexWidth = 0;

    std::ifstream file;
    file.open(truePath);

    std::unordered_map<std::string, int> variables;

    int highestChar = 0;
    int lowestOffsetY = INT_MAX;

    if (file.is_open()) {
      std::string line;
      int row = 1;

      while (std::getline(file, line)) {
        std::string junk; // store data we do not care about
        std::stringstream ss;
        ss << line;

        // Get texture path from the third row of the .fnt file
        if (row == 3) {
          std::string texName;
          ss >> junk >> junk >> texName;
          size_t texStart = texName.find_first_of('"') + 1;
          size_t texEnd = texName.find_last_of('"');
          texName = texName.substr(texStart, texEnd - texStart);

          size_t dirPos = fontPath.find_last_of('/');
          std::string dirPath(fontPath);
          dirPath.erase(dirPos + 1);
          dirPath += texName;

          std::cout << "Path: " << dirPath << std::endl;


          fontFace->m_TextureAtlas = Texture::create(dirPath);
          fontTexWidth = fontFace->m_TextureAtlas->getWidth();
        }
        else if (row > 4) {
          ss >> junk;

          int variables[8] = { 0 }; // the 8 variables we care about
          int varCount = 0;
          std::string delimLine;

          while (std::getline(ss, delimLine, ' ') && varCount < 8) {
            if (delimLine.empty()) {
              continue;
            }

            std::size_t pos = delimLine.find('=');
            if (pos != std::string::npos) {
              std::stringstream value_ss(delimLine.substr(pos + 1));
              value_ss >> variables[varCount];
            }

            varCount++;
          }

          FontCharacter fontChar;
          fontChar.id = variables[0];
          fontChar.sizeX = variables[3];
          fontChar.sizeY = variables[4];
          fontChar.xOffset = variables[5];
          fontChar.yOffset = variables[6];
          fontChar.xAdvance = variables[7];
          fontChar.xLeftTexCoord = variables[1] / (double)fontTexWidth;
          fontChar.yBottomTexCoord = (fontTexWidth - variables[2] - fontChar.sizeY) / (double)fontTexWidth;
          fontChar.xRightTexCoord = (variables[1] + fontChar.sizeX) / (double)fontTexWidth;
          fontChar.yTopTexCoord = (fontTexWidth - variables[2]) / (double)fontTexWidth;

          std::cout << char(fontChar.id) << ": " << fontChar.xLeftTexCoord << ", " << fontChar.yTopTexCoord << std::endl;

          fontFace->m_FontCharacters[variables[0]] = fontChar;

          if (fontChar.sizeY + fontChar.yOffset > highestChar) {
            highestChar = fontChar.sizeY + fontChar.yOffset;
          }

          if (fontChar.yOffset < lowestOffsetY) {
            lowestOffsetY = fontChar.yOffset;
          }
        }

        row++;
      }

      fontFace->m_FontBoundingBoxY = highestChar - lowestOffsetY;
      fontFace->m_FontLowestOffsetY = lowestOffsetY;
      std::cout << fontFace->m_FontBoundingBoxY << std::endl;
    }
    else {
      std::cout << "Unable to open file at: " << fontPath << "\n";
    }

    file.close();

    return fontFace;
  }

  // Getters
  int FontFace::getBoundingBoxY() const {
    return m_FontBoundingBoxY;
  }

  int FontFace::getLowestOffsetY() const {
    return m_FontLowestOffsetY;
  }

  FontCharacter FontFace::getFontChar(const char& character) const {
    return m_FontCharacters.at((int)character);
  }
}

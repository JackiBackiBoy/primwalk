#include "primwalk/data/font.hpp"
#include "primwalk/math/math.hpp"
#include "primwalk/rendering/graphicsDevice.hpp"
#include "primwalk/managers/resourceManager.hpp"

// std
#include <cstdint>
#include <fstream>
#include <iostream>

namespace pw {

  Font::Font(const std::string& path, double fontSize, FontWeight weight)
  {
    m_FontWeight = weight;

    GraphicsDevice_Vulkan* device = pw::GetDevice();

    glyphs = std::vector<msdf_atlas::GlyphGeometry>();
    m_GlyphData = std::unordered_map<msdf_atlas::unicode_t, GlyphData>();
    m_FontSize = fontSize;
    std::string truePath = BASE_DIR + path;

    loadMetadata(truePath);

    bool success = false;
    // Initialize instance of FreeType library
    if (msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype()) {
      // Load font file
      if (msdfgen::FontHandle* font = msdfgen::loadFont(ft, truePath.c_str())) {
        // Set variable width (if applicable)
        std::vector<msdfgen::FontVariationAxis> axes;
        msdfgen::listFontVariationAxes(axes, ft, font);

        if (!axes.empty()) {
          bool success = msdfgen::setFontVariationAxis(ft, font, "Weight", (int)m_FontWeight);

          if (success) {
            m_IsVariableWeight = true;
          }
        }
        else {
          m_FontWeight = getWeightFromString(m_MetaData.subFamily);
        }

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
        packer.setPixelRange(2.0);
        packer.setMiterLimit(1.0);

        // Compute atlas layout - pack glyphs
        packer.pack(glyphs.data(), glyphs.size());
        // Get final atlas dimensions
        packer.getDimensions(width, height);
        // The ImmediateAtlasGenerator class facilitates the generation of the atlas bitmap.
        msdf_atlas::ImmediateAtlasGenerator<float, 4, &msdf_atlas::mtsdfGenerator,
          msdf_atlas::BitmapAtlasStorage<msdf_atlas::byte, 4>
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

    std::cout << "Successfully loaded font at: " << truePath << '\n';
    std::cout << '\t' << "Family: " << m_MetaData.family << '\n';
    std::cout << '\t' << "Sub-Family: " << m_MetaData.subFamily << '\n';
  }

  Texture2D& Font::getTextureAtlas() const
  {
    return *m_TextureAtlas;
  }

  int Font::getTextWidth(const std::string& text, float fontSize) const
  {
    float width = 0.0f;
    float scaling = fontSize / m_FontSize;

    for (size_t i = 0; i < text.length(); i++) {
      GlyphData glyph = m_GlyphData.at(static_cast<uint32_t>(text[i]));

      if (i < text.length() - 1) {
        width += glyph.advanceX * fontSize;
      }
      else {
        width += (glyph.bearingX + glyph.width) * scaling;
      }
    }

    return static_cast<int>(width);
  }

  int Font::getMaxHeight() const
  {
    return m_MaxHeight;
  }

  GlyphData Font::getGlyph(const msdf_atlas::unicode_t& c) const
  {
    return m_GlyphData.at(c);
  }

  double Font::getFontSize() const
  {
    return m_FontSize;
  }

  std::shared_ptr<Font> Font::create(const std::string& path, double fontSize, FontWeight weight)
  {
    return ResourceManager::Get().loadFont(path, fontSize, weight);
  }

  bool Font::submitAtlasBitmapAndLayout(
    const msdf_atlas::BitmapAtlasStorage<msdf_atlas::byte, 4>& atlas, std::vector<msdf_atlas::GlyphGeometry> glyphs)
  {
    msdfgen::BitmapConstRef<unsigned char, 4> bitmap = (msdfgen::BitmapConstRef<unsigned char, 4>)atlas;
    atlasWidth = bitmap.width;
    atlasHeight = bitmap.height;

    uint32_t code = 0;

    auto fWidth = static_cast<float>(width);
    auto fHeight = static_cast<float>(height);

    //glyph
    for (msdf_atlas::GlyphGeometry g : glyphs) {
      GlyphData glyphData = GlyphData();

      double pLeft, pBottom, pRight, pTop;
      double aLeft, aBottom, aRight, aTop;
      g.getQuadPlaneBounds(pLeft, pBottom, pRight, pTop);
      g.getQuadAtlasBounds(aLeft, aBottom, aRight, aTop);
      
      glyphData.atlasBounds = { aLeft, aBottom, aRight, aTop };
      glyphData.planeBounds = { pLeft, pBottom, pRight, pTop };
      glyphData.advanceX = static_cast<float>(g.getAdvance());
      glyphData.width = aRight - aLeft;
      glyphData.height = aTop - aBottom;
      glyphData.bearingUnderline = pBottom * m_FontSize;

      m_GlyphData.insert({ g.getCodepoint(), glyphData });

      if (glyphData.height > m_MaxHeight) {
        code = g.getCodepoint();
      }

      m_MaxHeight = std::max(m_MaxHeight, (int)((float)glyphData.height + glyphData.bearingUnderline));
    }

    m_TextureAtlas = std::make_unique<Texture2D>(atlasWidth, atlasHeight, (unsigned char*)bitmap.pixels, 4, VK_FORMAT_R8G8B8A8_UNORM);

    return true; // TODO: Make return value actually reflect whether it worked or not
  }

  void Font::loadMetadata(const std::string& path)
  {
    OffsetTableTTF offsetTable{};
    TableDirTTF tableDir{};
    NameTableTTF nameTableHeader{};
    NameRecordTTF nameRecord{};

    std::ifstream file;
    file.open(path, std::ios_base::binary);

    if (!file.is_open()) {
      std::cout << "Failed to open TTF file\n";
      return;
    }

    // Header data
    file.read(reinterpret_cast<char*>(&offsetTable), sizeof(offsetTable));
    uint16_t* ptr = reinterpret_cast<uint16_t*>(&offsetTable);

    for (size_t i = 0; i < sizeof(offsetTable) / sizeof(uint16_t); i++) {
      *(ptr++) = Math::byteSwap(*ptr); // big endian to little endian
    }

    // Offsets table
    bool nameTableFound = false;
    for (size_t i = 0; i < offsetTable.numTables; i++) {
      file.read(reinterpret_cast<char*>(&tableDir), sizeof(tableDir));
      std::string name = std::string(tableDir.szTag.begin(), tableDir.szTag.end());

      if (name == "name") { // found the font name table
        nameTableFound = true;
        tableDir.length = Math::byteSwap(tableDir.length);
        tableDir.offset = Math::byteSwap(tableDir.offset);
        break;
      }
    }

    if (!nameTableFound) {
      file.close();
      return;
    }

    // Read names table header
    file.seekg(tableDir.offset); // jump to the names table header
    file.read(reinterpret_cast<char*>(&nameTableHeader), sizeof(nameTableHeader));
    nameTableHeader.numNameRecords = Math::byteSwap(nameTableHeader.numNameRecords);
    nameTableHeader.storageOffset = Math::byteSwap(nameTableHeader.storageOffset);

    // Run through all records to find font metadata
    MetadataTTF fontMetadata{};
    std::string* dataPtr = reinterpret_cast<std::string*>(&fontMetadata);

    for (uint16_t i = 0; i < nameTableHeader.numNameRecords; i++) {
      file.read(reinterpret_cast<char*>(&nameRecord), sizeof(nameRecord));
      nameRecord.nameID = Math::byteSwap(nameRecord.nameID);
      nameRecord.stringLength = Math::byteSwap(nameRecord.stringLength);
      nameRecord.stringOffset = Math::byteSwap(nameRecord.stringOffset);

      // Save file position for later continuation of search
      std::streampos nPos = file.tellg();
      file.seekg(tableDir.offset + nameRecord.stringOffset + nameTableHeader.storageOffset);

      std::vector<char> name(nameRecord.stringLength);
      file.read(name.data(), nameRecord.stringLength);

      std::string trueName = std::string(name.begin(), name.end());
      trueName.erase(std::remove(trueName.begin(), trueName.end(), '\0'), trueName.end());

      if (nameRecord.nameID < 15) { // TODO: Add more font metadata beyond NameID 14
        *(dataPtr + nameRecord.nameID) = trueName;
      }

      file.seekg(nPos);
    }

    file.close();

    m_MetaData = fontMetadata;
  }

}
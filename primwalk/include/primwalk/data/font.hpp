#pragma once

// primwalk
#include "primwalk/core.hpp"
#include "primwalk/data/fontWeight.hpp"
#include "primwalk/rendering/texture2D.hpp"

// std
#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// vendor
#include <glm/glm.hpp>
#include "msdf-atlas-gen/msdf-atlas-gen.h"

namespace pw {
  struct PW_API GlyphData {
    double width = 0;
    double height = 0;
    double bearingX = 0;
    double bearingY = 0;
    double advanceX = 0;
    double bearingUnderline = 0.0;
    glm::vec4 atlasBounds{};
    glm::vec4 planeBounds{};
  };

  class PW_API Font {
    public:
      Font(const std::string& path, double fontSize, FontWeight weight = FontWeight::Regular);
      ~Font() {};

      // Getters
      Texture2D& getTextureAtlas() const;
      float getTextWidth(const std::string& text, float fontSize) const;
      int getMaxHeight() const;
      GlyphData getGlyph(const msdf_atlas::unicode_t& c) const;
      double getFontSize() const;
      inline FontWeight getWeight() const { return m_FontWeight; }
      inline std::string getFamily() const { return m_MetaData.family; }

      int atlasWidth = 0;
      int atlasHeight = 0;
      static std::shared_ptr<Font> create(const std::string& path, double fontSize, FontWeight weight = FontWeight::Regular);

    private:
      struct OffsetTableTTF {
        uint16_t majorVersion;
        uint16_t minorVersion;
        uint16_t numTables;
        uint16_t searchRange;
        uint16_t entrySelector;
        uint16_t rangeShift;
      };

      struct TableDirTTF {
        std::array<char, 4> szTag; // table name
        uint32_t checkSum;         // check sum
        uint32_t offset;           // offset from beginning of file
        uint32_t length;           // length of the table in bytes
      };

      struct NameTableTTF {
        uint16_t formatSelector;
        uint16_t numNameRecords;
        uint16_t storageOffset;
      };

      struct NameRecordTTF {
        uint16_t platformID;
        uint16_t encodingID;
        uint16_t languageID;
        uint16_t nameID;
        uint16_t stringLength;
        uint16_t stringOffset; //from start of storage area
      };

      struct MetadataTTF {
        std::string copyright;
        std::string family;
        std::string subFamily;
        std::string uniqueSubFamilyID;
        std::string fullName;
        std::string nameTableVersion;
        std::string postScriptName;
        std::string trademarkNotice;
        std::string manufacturerName;
        std::string designerName;
        std::string description;
        std::string vendorURL;
        std::string designerURL;
        std::string licenseDescription;
        std::string licenseInfoURL;
      };

      std::string m_FamilyName;
      std::string m_SubFamilyName;
      FontWeight m_FontWeight;
      MetadataTTF m_MetaData;

      bool submitAtlasBitmapAndLayout(
        const msdf_atlas::BitmapAtlasStorage<msdf_atlas::byte, 4>& atlas,
        std::vector<msdf_atlas::GlyphGeometry> glyphs);
      void loadMetadata(const std::string& path);

      std::unordered_map<msdf_atlas::unicode_t, GlyphData> m_GlyphData;
      glm::ivec2 m_BoundingBox = { 0, 0 };
      int m_MaxHeight = 0;
      int m_FontBoundingBoxY = 0;
      int m_FontLowestOffsetY = 0;
      double m_FontSize = 0.0f;
      bool m_IsVariableWeight = false;

      std::unique_ptr<Texture2D> m_TextureAtlas;
      std::vector<msdf_atlas::GlyphGeometry> glyphs;
      
      int width = 0;
      int height = 0;

      friend class FontManager;
  };
}

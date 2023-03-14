#ifndef FZ_FONT_MANAGER_HEADER
#define FZ_FONT_MANAGER_HEADER

// FZUI
#include "fzui/core.hpp"
#include "fzui/windows/data/fonts/fontFace.hpp"

// std
#include <unordered_map>

namespace fz {
  // Singleton design pattern
  class FZ_API FontManager {
    public:
      // Disable copy/move
      FontManager(const FontManager&) = delete;
      FontManager(FontManager&&) = delete;
      FontManager& operator=(const FontManager&) = delete;
      FontManager& operator=(FontManager&&) = delete;

      static FontManager& Instance();

      /* ------ Methods ------ */
      void addFont(const std::string& name, const int& size,
                   const std::string& path);

      // Getters
      FontFace* getFont(const std::string& name) const;
      FontFace* getDefaultSmallFont() const;
      FontFace* getDefaultMediumFont() const;
      FontFace* getDefaultLargeFont() const;
      
      // Setters
      void setDefaultSmallFont(FontFace* fontFace);
      void setDefaultMediumFont(FontFace* fontFace);
      void setDefaultLargeFont(FontFace* fontFace);

    private:
      FontManager();
      ~FontManager();

      std::unordered_map<std::string, FontFace*> m_Fonts;
      FontFace* m_DefaultSmallFont = nullptr;
      FontFace* m_DefaultMediumFont = nullptr;
      FontFace* m_DefaultLargeFont = nullptr;
  };
}
#endif
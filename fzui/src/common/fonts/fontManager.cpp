// FZUI
#include "fzui/data/fonts/fontManager.hpp"

namespace fz {
  FontManager::FontManager() {
    m_Fonts = std::unordered_map<std::string, FontFace*>();
  }

  FontManager::~FontManager() {
    for (auto& f : m_Fonts) {
      delete f.second;
    }

    m_Fonts.clear();
  }

  FontManager& FontManager::Instance() {
    static FontManager instance;
    return instance;
  }

  /* ------ Methods ------ */
  void FontManager::addFont(const std::string& name, const int& size, const std::string& path) {
    auto found = m_Fonts.find(name);

    if (found != m_Fonts.end()) { // font already exists
      // TODO: Add to error log if font was not found
      return;
    }

    FontFace* fontFace = FontFace::create(path, size);

    m_Fonts.insert({ name, fontFace });
  }

  // Getters
  FontFace* FontManager::getFont(const std::string& name) const {
    auto found = m_Fonts.find(name);

    if (found == m_Fonts.end()) { // font not found
      return nullptr;
    }

    return found->second;
  }

  FontFace* FontManager::getDefaultSmallFont() const {
    return m_DefaultSmallFont;
  }

  FontFace* FontManager::getDefaultMediumFont() const {
    return m_DefaultMediumFont;
  }

  FontFace* FontManager::getDefaultLargeFont() const {
    return m_DefaultLargeFont;
  }

  // Setters
  void FontManager::setDefaultSmallFont(FontFace* fontFace) {
    m_DefaultSmallFont = fontFace;
  }

  void FontManager::setDefaultMediumFont(FontFace* fontFace) {
    m_DefaultMediumFont = fontFace;
  }

  void FontManager::setDefaultLargeFont(FontFace* fontFace) {
    m_DefaultLargeFont = fontFace;
  }
}

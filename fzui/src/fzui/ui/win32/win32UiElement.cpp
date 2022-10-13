#include "fzui/ui/win32/win32UiElement.hpp"
#include "fzui/managers/fontManager.hpp"

namespace fz {
  Win32UiElement::Win32UiElement(const std::wstring& text, const int& x, const int& y,
      HWND hWnd, const std::string& fontName, const int& fontSize, const int& fontFlags)
    : m_Text{text}, m_X(x), m_Y(y), m_OnClick{[]() -> void {}}, m_ParentHandle{hWnd} {
      m_Font = FontManager::getInstance().getFont(fontName, fontSize, fontFlags);
  }

  void Win32UiElement::setOnClick(std::function<void()> onClick) {
    m_OnClick = onClick;
  }
}

#include "fzui/ui/win32/win32UiElement.hpp"
#include "fzui/managers/fontManager.hpp"

namespace fz {
  Win32UiElement::Win32UiElement()
    : m_OnClick{[]() -> void {}} {
      m_Font = FontManager::getInstance().getFont("Segoe Ui", 16, FW_NORMAL);
  }

  // Getters
  int Win32UiElement::getPositionX() const {
    return m_X;
  }

  // Setters
  void Win32UiElement::setPosition(const int& x, const int& y) {
    m_X = x;
    m_Y = y;
  }

  void Win32UiElement::setOnClick(std::function<void()> onClick) {
    m_OnClick = onClick;
  }
}

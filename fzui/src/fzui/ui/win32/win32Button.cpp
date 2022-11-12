#include "fzui/ui/win32/win32Button.hpp"
#include "fzui/utilities.hpp"
#include "fzui/ui/uiStyle.hpp"

namespace fz {
  Win32Button::Win32Button() : Win32UiElement() {
      m_BorderRadius = 0;
      m_BorderThickness = 0;

      m_DefaultColor = UiStyle::darkButtonDefaultColor;
      m_HoverColor = UiStyle::darkButtonHoverColor;
      m_BorderColor = UiStyle::darkButtonDefaultColor;
      m_TextColor = UiStyle::darkButtonTextColor;
    }

  // Getters
  CREATESTRUCT Win32Button::getCreateStruct() {
    CREATESTRUCT cs;
    ZeroMemory(&cs, sizeof(cs));
    cs.dwExStyle = 0;
    cs.lpszClass = L"BUTTON"; // Predefined system class name
    cs.lpszName = m_Text.c_str();  // Window title
    cs.style = WS_VISIBLE | WS_CHILD | BS_OWNERDRAW;
    cs.x = m_X; // X position
    cs.y = m_Y; // Y position
    cs.cx = m_Width; // Width
    cs.cy = m_Height; // Height
    cs.hMenu = NULL; // Leave ID undefined
    cs.lpCreateParams = NULL;

    return cs;
  }

  std::string Win32Button::getTypeString() const {
    return "Win32Button";
  }

  // Setters
  void Win32Button::setBackgroundColor(const Color& color) {
    m_DefaultColor = color;
  }

  void Win32Button::setBorderColor(const Color& color) {
    m_BorderColor = color;
  }

  void Win32Button::setText(const std::wstring& text) {
    m_Text = text;
  }

  void Win32Button::setTextColor(const Color& color) {
    m_TextColor = color;
  }

  void Win32Button::setWidth(const int& width) {
    m_Width = width;
  }

  void Win32Button::setHeight(const int& height) {
    m_Height = height;
  }
}

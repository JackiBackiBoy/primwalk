#include "fzui/ui/win32/win32Button.hpp"
#include "fzui/utilities.hpp"
#include "fzui/ui/uiStyle.hpp"

namespace fz {
  Win32Button::Win32Button(const std::wstring& text, const int& x, const int& y,
      const int& width, const int& height, HWND hWnd,
      const std::string& fontName, const int& fontSize, const int& fontFlags)
        : Win32UiElement(text, x, y, hWnd, fontName, fontSize, fontFlags),
        m_Width{width}, m_Height(height) {

      m_BorderRadius = 0;
      m_BorderThickness = 0;

      m_DefaultColor = UiStyle::darkButtonDefaultColor;
      m_HoverColor = UiStyle::darkButtonHoverColor;
      m_BorderColor = UiStyle::darkButtonDefaultColor;
      m_TextColor = UiStyle::darkButtonTextColor;
    }

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
    cs.hwndParent = m_ParentHandle;
    cs.hMenu = NULL; // Leave ID undefined
    cs.hInstance = (HINSTANCE)GetWindowLongPtr(m_ParentHandle, GWLP_HINSTANCE);
    cs.lpCreateParams = NULL;

    return cs;
  }

  // Setters
  void Win32Button::setBackgroundColor(const Color& color) {
    m_DefaultColor = color;
  }

  void Win32Button::setBorderColor(const Color& color) {
    m_BorderColor = color;
  }

  void Win32Button::setTextColor(const Color& color) {
    m_TextColor = color;
  }
}

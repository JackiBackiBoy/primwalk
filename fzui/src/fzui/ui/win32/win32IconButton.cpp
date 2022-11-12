// std
#include <cassert>
#include <iostream>

// FZUI
#include "fzui/ui/win32/win32IconButton.hpp"
#include "fzui/utilities.hpp"
#include "fzui/ui/uiStyle.hpp"

namespace fz {
  Win32IconButton::Win32IconButton() : Win32UiElement() {
    m_BorderRadius = 0;
    m_BorderThickness = 0;

    m_DefaultColor = UiStyle::darkButtonDefaultColor;
    m_HoverColor = UiStyle::darkButtonHoverColor;
    m_BorderColor = UiStyle::darkButtonDefaultColor;
    m_TextColor = UiStyle::darkButtonTextColor;
  }

  Win32IconButton::~Win32IconButton() {
    DestroyIcon(m_Icon);
  }

  // Getters
  CREATESTRUCT Win32IconButton::getCreateStruct() {
    CREATESTRUCT cs;
    ZeroMemory(&cs, sizeof(cs));
    cs.dwExStyle = 0;
    cs.lpszClass = L"BUTTON"; // Predefined system class name
    cs.lpszName = L"";  // Window title
    cs.style = WS_VISIBLE | WS_CHILD | BS_OWNERDRAW;
    cs.x = m_X; // X position
    cs.y = m_Y; // Y position
    cs.cx = m_Width; // Width
    cs.cy = m_Height; // Height
    cs.hMenu = NULL; // Leave ID undefined
    cs.lpCreateParams = NULL;

    return cs;
  }

  int Win32IconButton::getWidth() const {
    return m_Width;
  }

  int Win32IconButton::getHeight() const {
    return m_Height;
  }

  HICON Win32IconButton::getIcon() const {
    return m_Icon;
  }

  std::string Win32IconButton::getTypeString() const {
    return "Win32IconButton";
  }

  // Setters
  void Win32IconButton::setBackgroundColor(const Color& color) {
    m_DefaultColor = color;
  }

  void Win32IconButton::setBorderColor(const Color& color) {
    m_BorderColor = color;
  }

  void Win32IconButton::setText(const std::wstring& text) {
    m_Text = text;
  }

  void Win32IconButton::setTextColor(const Color& color) {
    m_TextColor = color;
  }

  void Win32IconButton::setWidth(const int& width) {
    m_Width = width;
  }

  void Win32IconButton::setHeight(const int& height) {
    m_Height = height;
  }

  void Win32IconButton::setPosition(const int& x, const int& y) {
    m_X = x;
    m_Y = y;

    if (m_Handle != NULL) {
      SetWindowPos(m_Handle, NULL, m_X, m_Y, m_Width, m_Height, SWP_NOZORDER | SWP_NOREDRAW);
    }
  }

  void Win32IconButton::setIcon(const std::wstring& path) {
    size_t extensionIndex = path.find_last_of('.');
    std::wstring extension = path.substr(extensionIndex);
    //printf("Icon info: %ls, %ls \n", extension.c_str(), path.c_str());

    assert(extension == L".ico" && "ASSERTION FAILED: Icons must have the .ico extension");

    if (m_Icon != NULL) {
      DestroyIcon(m_Icon);
    }

    m_Icon = (HICON)LoadImage(NULL, path.c_str(), IMAGE_ICON, m_Width, m_Height, LR_LOADFROMFILE);
  }
}

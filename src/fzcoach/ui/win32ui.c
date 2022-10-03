#include "win32ui.h"

int createWin32Label(HWND window, HWND* targetLabel, const wchar_t* text,
    const int x, const int y, HFONT font) {
  HDC hdc = GetDC(window);

  // Calculate required width and height of element
  SIZE requiredSize;
  HFONT oldFont = (HFONT)SelectObject(hdc, font);
  GetTextExtentPoint32(hdc, text, wcslen(text), &requiredSize);

  SelectObject(hdc, oldFont);
  DeleteObject(oldFont);
  ReleaseDC(window, hdc);

  *targetLabel = CreateWindow(
    L"STATIC",  // Predefined class; Unicode assumed 
    text,      // Button text 
    WS_VISIBLE | WS_CHILD | SS_LEFT,  // Styles 
    x,         // x position 
    y,         // y position 
    requiredSize.cx,        // Button width
    requiredSize.cy,        // Button height
    window,     // Parent window
    NULL,       // No menu.
    (HINSTANCE)GetWindowLongPtr(window, GWLP_HINSTANCE), 
    NULL);      // Pointer not needed.

  SendMessage(*targetLabel, WM_SETFONT, (LPARAM)font, TRUE);

  return *targetLabel != NULL ? 1 : 0;
}

int createWin32Button(HWND window, HWND* targetButton, const wchar_t* text,
    const int x, const int y, const int width, const int height, HFONT font) {
  *targetButton = CreateWindow(
    L"BUTTON",  // Predefined class; Unicode assumed 
    text,      // Button text 
    WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,  // Styles 
    x,         // x position 
    y,         // y position 
    width,        // Button width
    height,        // Button height
    window,     // Parent window
    NULL,       // No menu.
    (HINSTANCE)GetWindowLongPtr(window, GWLP_HINSTANCE), 
    NULL);      // Pointer not needed.

  SendMessage(*targetButton, WM_SETFONT, (LPARAM)font, TRUE);

  return *targetButton != NULL ? 1 : 0;
}

int createWin32CheckBox(HWND window, HWND* targetCheckBox, const wchar_t* text,
    const int x, const int y, HFONT font, const INT_PTR id) {
  HDC hdc = GetDC(window);

  // Calculate required width and height of element
  SIZE requiredSize;
  HFONT oldFont = (HFONT)SelectObject(hdc, font);
  GetTextExtentPoint32(hdc, text, wcslen(text), &requiredSize);

  int checkBoxWidth  = 12 * GetDeviceCaps(hdc, LOGPIXELSX ) / 96 + 1;
  int checkBoxHeight = 12 * GetDeviceCaps(hdc, LOGPIXELSY ) / 96 + 1;
  int textOffset;
  GetCharWidth(hdc, '0', '0', &textOffset);
  textOffset /= 2;
  requiredSize.cx += checkBoxWidth + textOffset;
  if (requiredSize.cy < checkBoxHeight) {
      requiredSize.cy = checkBoxHeight;
  }

  // Cleanup
  SelectObject(hdc, oldFont);
  DeleteObject(oldFont);
  ReleaseDC(window, hdc);

  // Create the checkbox control
  *targetCheckBox = CreateWindowEx(
    0,
    L"BUTTON",  // Predefined class; Unicode assumed 
    text,      // Button text 
    WS_VISIBLE | WS_CHILD | BS_CHECKBOX,  // Styles 
    x,         // x position 
    y,         // y position 
    requiredSize.cx,        // Button width
    requiredSize.cy,        // Button height
    window,     // Parent window
    (HMENU)id,       // No menu.
    (HINSTANCE)GetWindowLongPtr(window, GWLP_HINSTANCE), 
    NULL);

  // Change the checkbox display font
  SendMessage(*targetCheckBox, WM_SETFONT, (LPARAM)font, TRUE);

  return *targetCheckBox != NULL ? 1 : 0;
}

int createWin32HorzLine(HWND window, HWND* targetLine,
    const int x, const int y, HFONT font) {
  *targetLine = CreateWindowEx(
    0,
    L"STATIC",  // Predefined class; Unicode assumed 
    L"",      // Button text 
    WS_VISIBLE | WS_CHILD | SS_ETCHEDHORZ,  // Styles 
    x,         // x position 
    y,         // y position 
    600,        // Button width
    1,        // Button height
    window,     // Parent window
    NULL,       // No menu.
    (HINSTANCE)GetWindowLongPtr(window, GWLP_HINSTANCE), 
    NULL);

  return *targetLine ? 1 : 0;
}

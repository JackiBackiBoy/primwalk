#include "fzui/ui/win32/win32Utilities.hpp"

namespace fz {
  SIZE calcReqCheckBoxSize(const std::wstring& text, HFONT font, HWND hWnd) {
    HDC hdc = GetDC(hWnd);

    SIZE reqSize;
    HFONT oldFont = (HFONT)SelectObject(hdc, font);
    GetTextExtentPoint32(hdc, text.c_str(), text.length(), &reqSize);

    int checkBoxWidth  = 12 * GetDeviceCaps(hdc, LOGPIXELSX ) / 96 + 1;
    int checkBoxHeight = 12 * GetDeviceCaps(hdc, LOGPIXELSY ) / 96 + 1;
    int textOffset;
    GetCharWidth(hdc, '0', '0', &textOffset);
    textOffset /= 2;
    reqSize.cx += checkBoxWidth + textOffset;

    if (reqSize.cy < checkBoxHeight) {
        reqSize.cy = checkBoxHeight;
    }

    // Cleanup
    SelectObject(hdc, oldFont);
    DeleteObject(oldFont);
    ReleaseDC(hWnd, hdc);

    return reqSize;
  }

  SIZE calcReqLabelSize(const std::wstring& text, HFONT font, HWND hWnd) {
    HDC hdc = GetDC(hWnd);

    SIZE reqSize;
    HFONT oldFont = (HFONT)SelectObject(hdc, font);
    GetTextExtentPoint32(hdc, text.c_str(), text.length(), &reqSize);

    // Cleanup
    SelectObject(hdc, oldFont);
    DeleteObject(oldFont);
    ReleaseDC(hWnd, hdc);

    return reqSize;
  }

  COLORREF getColorRef(const Color& color) {
    return RGB(color.r, color.g, color.b);
  }
}

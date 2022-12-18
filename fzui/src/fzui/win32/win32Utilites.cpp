#include "fzui/win32/win32Utilities.hpp"

namespace fz {
  Win32Utilities& Win32Utilities::Instance() {
    static Win32Utilities instance;
    return instance;
  }

  std::string Win32Utilities::getWindowsVersionString() {
    // Acquire Windows version information if not already done
    if (m_WindowsVersionString == "") {
      NTSTATUS(WINAPI *RtlGetVersion)(LPOSVERSIONINFOEXW);

      OSVERSIONINFOEXW osInfo;

      *(FARPROC*)&RtlGetVersion = GetProcAddress(GetModuleHandle(L"ntdll"), "RtlGetVersion");
      if (NULL != RtlGetVersion) {
        osInfo.dwOSVersionInfoSize = sizeof(osInfo);
        RtlGetVersion(&osInfo);
      }

      m_WindowsVersionString = osInfo.dwBuildNumber >= 22000 ? "Windows 11" : "Windows 10";
    }

    return m_WindowsVersionString;
  }

  SIZE Win32Utilities::calcReqCheckBoxSize(const std::wstring& text, HFONT font, HWND hWnd) {
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

  SIZE Win32Utilities::calcReqLabelSize(const std::wstring& text, HFONT font, HWND hWnd) {
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

  SIZE Win32Utilities::calcReqButtonSize(const std::wstring& text, const int& marginX,
      const int& marginY, HFONT font, HWND hWnd) {
    HDC hdc = GetDC(hWnd);

    SIZE reqSize;
    HFONT oldFont = (HFONT)SelectObject(hdc, font);
    GetTextExtentPoint32(hdc, text.c_str(), text.length(), &reqSize);
    reqSize.cx += marginX * 2;
    reqSize.cy += marginY * 2;

    // Cleanup
    SelectObject(hdc, oldFont);
    DeleteObject(oldFont);
    ReleaseDC(hWnd, hdc);

    return reqSize;
  }

  COLORREF Win32Utilities::getColorRef(const Color& color) {
    return RGB(color.r, color.g, color.b);
  }

  RECT Win32Utilities::getRelativeClientRect(HWND child, HWND base) {
    RECT windowRect{};
    RECT clientRect{};
    POINT pos{};
    int width;
    int height;

    GetWindowRect(child, &windowRect);
    pos.x = windowRect.left;
    pos.y = windowRect.top;

    GetClientRect(child, &clientRect);
    width = clientRect.right - clientRect.left;
    height = clientRect.bottom - clientRect.top;

    ScreenToClient(base, &pos);
    clientRect.left = pos.x;
    clientRect.top = pos.y;
    clientRect.right = pos.x + width;
    clientRect.bottom = pos.y + height;

    return clientRect;
  }
}

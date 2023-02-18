#define WINVER 0x0A00
#define _WIN32_WINNT  0x0A00

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// std
#include <iostream>

// Windows
#include <sdkddkver.h>
#include <windows.h>
#include <dwmapi.h>

// FZUI
#include "fzui/core/window.hpp"
#include "fzui/fzui.hpp"

int main() {
  INITCOMMONCONTROLSEX icc;

  // Initialise common controls.
  icc.dwSize = sizeof(icc);
  icc.dwICC = ICC_WIN95_CLASSES;
  InitCommonControlsEx(&icc);

  fz::Application& app = fz::Application::Instance(); // acquire app instance

  // ------ Main Window ------
  fz::Window* fzMain = new fz::Window(L"Forza Coach (Beta)", 720, 360);

  HINSTANCE hInstance = GetModuleHandle(0);
  fzMain->run(hInstance);

  delete fzMain;

  return 0;
}

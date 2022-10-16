#pragma once

#define WINVER 0x0A00
#define _WIN32_WINNT  0x0A00

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <sdkddkver.h>
#include <windows.h>
#include <dwmapi.h>

#include "fzui/core/window.hpp"
#include "fzui/ai/auctionBot.hpp"

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

extern fz::Window* targetApp();

// Main thread (window thread)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {

  INITCOMMONCONTROLSEX icc;

  // Initialise common controls.
  icc.dwSize = sizeof(icc);
  icc.dwICC = ICC_WIN95_CLASSES;
  InitCommonControlsEx(&icc);

  // Create popup start window
  fz::Window* app = targetApp();
  app->run(hInstance);

  return 0;
}


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
#include "fzui/ai/auctionBot.hpp"
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
  fz::WindowInfo windowInfo = fz::WindowInfo::DefaultDark;
  fzMain->setWindowInfo(windowInfo);
  
  // ------ Side Bar Window ------
  fz::Window* fzSideBar = new fz::Window(L"FZUI.SideBar", 47, fzMain->getHeight() - windowInfo.titleBarHeight);
  fzSideBar->setPosition(0, windowInfo.titleBarHeight);
  fzSideBar->setBackground({ 51, 51, 51 });

  fz::Win32IconButton* settingsButton = fzSideBar->addElement<fz::Win32IconButton>();
  settingsButton->setWidth(24);
  settingsButton->setHeight(24);
  settingsButton->setPosition(fzSideBar->getWidth() / 2 - settingsButton->getWidth() / 2, fzSideBar->getHeight() - 2 * settingsButton->getHeight());
  settingsButton->setDefaultColor({ 255, 255, 255 });
  settingsButton->setHoverColor({ 10, 164, 164 });
  settingsButton->setIcon(L"assets/icons/settings24x24.ico");

  fzMain->setOnResize([fzSideBar, fzMain, windowInfo, settingsButton]() {
      fzSideBar->setHeight(fzMain->getHeight() - windowInfo.titleBarHeight);
      settingsButton->setPosition(settingsButton->getPositionX(), fzSideBar->getHeight() - 2 * settingsButton->getHeight());
  });

  //fz::Win32Button* settingsButton = fzSideBar->addElement<fz::Win32Button>();
  //settingsButton->setText(L"SO");
  //settingsButton->setPosition(0, 0);
  //settingsButton->setWidth(80);
  //settingsButton->setHeight(50);
  //settingsButton->setTextColor({ 255, 255, 255 });

  fzMain->addSubWindow(fzSideBar);

  // TODO: Pass application instead
  HINSTANCE hInstance = GetModuleHandle(0);
  fzMain->run(hInstance);

  delete fzMain;

  return 0;
}

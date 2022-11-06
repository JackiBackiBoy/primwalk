#define WINVER 0x0A00
#define _WIN32_WINNT  0x0A00

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

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
  
  // Bot start button
  fz::Win32Button* botStartButton = fzMain->addElement<fz::Win32Button>();
  botStartButton->setText(L"Start");
  botStartButton->setPosition(60, 80);
  botStartButton->setWidth(100);
  botStartButton->setHeight(50);
  botStartButton->setTextColor({ 255, 255, 255 });

  // Bot stop button
  fz::Win32Button* botStopButton = fzMain->addElement<fz::Win32Button>();
  botStopButton->setText(L"Stop");
  botStopButton->setPosition(180, 80);
  botStopButton->setWidth(100);
  botStopButton->setHeight(50);
  botStopButton->setTextColor({ 255, 255, 255 });

  // ------ Side Bar Window ------
  fz::Window* fzSideBar = new fz::Window(L"FZUI.SideBar", 47, 500);
  fzSideBar->setPosition(0, windowInfo.titleBarHeight);
  fzSideBar->setBackground({ 51, 51, 51 });

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

#include "fzui/fzui.hpp"
#include <iostream>

class FZCoach : public fz::Window {
  private:
    fz::Win32Button* botStartButton;
    fz::Win32Button* botStopButton;
    fz::Win32Button* fileButton;

    fz::Win32MenuItem* fileMenu;
    fz::Win32MenuItem* viewMenu;

  public:
    FZCoach() : fz::Window::Window(L"Forza Coach (Beta)", 720, 360) {};

    void onCreate(HWND hWnd) override {
      // Create main window on main thread
      static HWND hForzaHandle = FindWindow(NULL, L"Forza Horizon 5");

      if (hForzaHandle != NULL) {
        SetForegroundWindow(hForzaHandle);
      }

      fz::WindowInfo windowInfo = fz::WindowInfo::DefaultDark;
      setWindowInfo(windowInfo);

      // Menu area
      fileMenu = new fz::Win32MenuItem(L"File");
      viewMenu = new fz::Win32MenuItem(L"View");

      // Bot control area
      fz::Win32Label auctionBotTitle = { L"Forza Coach - Auction Bot", 20, 30, hWnd, "Segoe UI", 32, FW_BOLD };
      fz::Win32HorzLine botTitleLine = { 20, 65, 600, hWnd };

      botStartButton = new fz::Win32Button(L"Start", 20, 80, 100, 50, hWnd);
      botStartButton->setOnClick([]() -> void { startAuctionBot(hForzaHandle, true); });

      botStopButton = new fz::Win32Button(L"Stop", 140, 80, 100, 50, hWnd);
      botStopButton->setOnClick([]() -> void { stopAuctionBot(); });

      // Bot settings area
      fz::Win32Label botSettingsTitle = { L"Bot settings", 20, 160, hWnd, "Segoe UI", 24, FW_BOLD };
      fz::Win32HorzLine botSettingsTitleLine = { 20, 190, 600, hWnd };
      fz::Win32CheckBox livePreviewCheck = { L"Live Preview", 20, 200, hWnd };
      fz::Win32CheckBox overDriveModeCheck = { L"Overdrive Mode", 200, 180, hWnd };

      // Menu area
      addMenuItem(fileMenu);
      addMenuItem(viewMenu);

      // UI elements
      addUiElement(&auctionBotTitle);
      addUiElement(&botTitleLine);
      addUiElement(botStartButton);
      addUiElement(botStopButton);
      addUiElement(&botSettingsTitle);
      addUiElement(&botSettingsTitleLine);
      addUiElement(&livePreviewCheck);
      addUiElement(&overDriveModeCheck);
    }
};

fz::Window* targetApp() {
  return new FZCoach();
}

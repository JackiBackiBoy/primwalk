#include "fzui/fzui.hpp"
#include <iostream>

class FZCoach : public fz::Window {
  private:
    fz::Win32Button* botStartButton;
    fz::Win32Button* botStopButton;

  public:
    FZCoach() : fz::Window::Window(L"Forza Coach (Beta)", 720, 360) {};

    void onCreate(HWND hWnd) override {
      setDarkMode(true);

      // Create main window on main thread
      static HWND hForzaHandle = FindWindow(NULL, L"Forza Horizon 5");

      if (hForzaHandle != NULL) {
        SetForegroundWindow(hForzaHandle);
      }

      // Bot control area
      fz::Win32Label auctionBotTitle = { L"Forza Coach - Auction Bot", 20, 10, hWnd, "Segoe UI", 32, FW_BOLD };
      fz::Win32HorzLine botTitleLine = { 20, 45, 600, hWnd };

      botStartButton = new fz::Win32Button(L"Start", 20, 60, 100, 50, hWnd);
      botStartButton->setOnClick([]() -> void { startAuctionBot(hForzaHandle, true); });

      botStopButton = new fz::Win32Button(L"Stop", 140, 60, 100, 50, hWnd);
      botStopButton->setOnClick([]() -> void { stopAuctionBot(); });

      // Bot settings area
      fz::Win32Label botSettingsTitle = { L"Bot settings", 20, 140, hWnd, "Segoe UI", 24, FW_BOLD };
      fz::Win32HorzLine botSettingsTitleLine = { 20, 170, 600, hWnd };
      fz::Win32CheckBox livePreviewCheck = { L"Live Preview", 20, 180, hWnd };
      fz::Win32CheckBox overDriveModeCheck = { L"Overdrive Mode", 180, 180, hWnd };

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

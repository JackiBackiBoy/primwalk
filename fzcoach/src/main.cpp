// std
#include <cstdint>
#include <iostream>
#include <memory>
#include <windows.h>
#include <vector>

// FZUI
#include "fzui/fzui.hpp"

class FzCoachWindow : public fz::Window {
  public:
    FzCoachWindow() : fz::Window("Forza Coach", 1080, 720) {};
    virtual ~FzCoachWindow() {}

    void captureFH5Window() {
      HDC hScreenDC = GetDC(NULL);
      HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

      HWND hForzaHandle = FindWindow(NULL, L"@Beaffs - Discord");

      RECT forzaWindowRect;

      GetWindowRect(hForzaHandle, &forzaWindowRect);

      // Capture screen area
      HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, forzaWindowRect.right - forzaWindowRect.left, forzaWindowRect.bottom - forzaWindowRect.top);
      HBITMAP hOldBitmap = (HBITMAP)(SelectObject(hMemoryDC, hBitmap));
      BitBlt(
        hMemoryDC,
        0,
        0,
        forzaWindowRect.right - forzaWindowRect.left,
        forzaWindowRect.bottom - forzaWindowRect.top,
        hScreenDC,
        forzaWindowRect.left,
        forzaWindowRect.top,
        SRCCOPY);

      // Get the pixel data
      BITMAP bmp;
      GetObject(hBitmap, sizeof(BITMAP), &bmp);
      bitmapWidth = bmp.bmWidth;
      bitmapHeight = bmp.bmHeight;

      BITMAPINFO bmi = { sizeof(BITMAPINFOHEADER), bitmapWidth, -bitmapHeight, 1, 32, BI_RGB };
      pixelData = std::vector<unsigned char>(bitmapWidth * bitmapHeight * 4); // Assuming 32-bit (4 bytes per pixel) RGB format

      GetDIBits(
        hMemoryDC,
        hBitmap,
        0,
        bitmapHeight,
        pixelData.data(),
        &bmi,
        DIB_RGB_COLORS
      );

      // Use the pixelData array as needed

      // Clean up
      SelectObject(hMemoryDC, hOldBitmap);
      DeleteObject(hBitmap);
      DeleteDC(hMemoryDC);
      ReleaseDC(NULL, hScreenDC);
    }

    void onCreate() override {
      // Setup
      captureFH5Window();
      auto gameframe = std::make_shared<fz::Texture2D>(bitmapWidth, bitmapHeight, pixelData.data(), 4, VK_FORMAT_B8G8R8A8_UNORM);
      m_GameFrame = gameframe.get();

      // Textures
      auto homeIcon = fz::Texture2D::create("assets/icons/home.png");
      auto auctionIcon = fz::Texture2D::create("assets/icons/auction.png");
      auto brushIcon = fz::Texture2D::create("assets/icons/brush.png");
      auto settingsIcon = fz::Texture2D::create("assets/icons/settings.png");
      auto logoIcon = fz::Texture2D::create("assets/icons/fzcoach_logo.png", 4, VK_FORMAT_R8G8B8A8_UNORM);

      float aspectRatio = (float)gameframe->getWidth() / gameframe->getHeight();
      auto& test = makeElement<fz::UIIconButton>("h", glm::vec2(150, 300), 500 * aspectRatio, 500, gameframe);

      // Fonts
      auto headerFont = fz::Font::create("assets/fonts/opensans.ttf", 32, fz::FontWeight::ExtraBold);

      // Dashboard container
      dashboardGroup = &makeElement<fz::UIContainer>("DG", glm::vec2(0, 0), 50, m_Height);
      dashboardGroup->setBackgroundColor({ 30, 30, 30 });
      
      titleBar = &makeElement<fz::UIContainer>("h", glm::vec2(0, 0), m_Width, 30);
      titleBar->setBackgroundColor({ 10, 10, 10 });

      auto& auctionBotTitle = makeElement<fz::UILabel>("h", "Designs & Paints", glm::vec2(150, 70), headerFont);
      auto& logo = makeElement<fz::UIIconButton>("Logo", glm::vec2(1, 1), logoIcon->getWidth(), 32, logoIcon);

      auto& dashboardButton = dashboardGroup->makeElement<fz::UIIconButton>("D2", glm::vec2(10, 160), 30, 30, homeIcon);
      dashboardButton.setBackgroundColor({ 70, 70, 70, 0 });

      auto& auctionsButton = dashboardGroup->makeElement<fz::UIIconButton>("D3", glm::vec2(10, 210), 30, 30, auctionIcon);
      auctionsButton.setBackgroundColor({ 70, 70, 70, 0 });

      auto& designPaintsButton = dashboardGroup->makeElement<fz::UIIconButton>("D4", glm::vec2(10, 260), 30, 30, brushIcon);
      designPaintsButton.setBackgroundColor({ 70, 70, 70, 0 });

      auto& settingsButton = dashboardGroup->makeElement<fz::UIIconButton>("D5", glm::vec2(10, 310), 30, 30, settingsIcon);
      settingsButton.setBackgroundColor({ 70, 70, 70, 0 });
    }

    void onUpdate(float dt) override {
      if (m_ShowPreview) {
        captureFH5Window();
        m_GameFrame->updateData(pixelData.data());
      }

      titleBar->setWidth(m_Width + 100);
      dashboardGroup->setHeight(m_Height + 100); // add some margins to prevent resize artifacts
    }

    private:
      fz::UIContainer* dashboardGroup = nullptr;
      fz::UIContainer* mainView = nullptr;
      fz::UIContainer* titleBar = nullptr;
      HWND m_FH5Handle = NULL;
      int bitmapWidth;
      int bitmapHeight;
      std::vector<unsigned char> pixelData;
      fz::Texture2D* m_GameFrame;
      bool m_ShowPreview = false;
};

int main() {
  fz::Application& app = fz::Application::Instance(); // acquire app instance
  FzCoachWindow* fzMain = new FzCoachWindow();
  fzMain->setBackgroundColor({ 20, 20, 20 });
  fzMain->run();

  delete fzMain;
  return 0;
}

// std
#include <cstdint>
#include <iostream>
#include <memory>
#include <windows.h>
#include <vector>
#include <string>
#include <fstream>
#include "formats/bc7decomp.h"

// primwalk
#include "primwalk/primwalk.hpp"

// Forza Coach
#include "formats/swatchbin.hpp"

int BC7_Decode(const uint8_t* pBlock, uint8_t* pPixelsRGBA, int pPixelsRGBAWidth, int pPixelsRGBAHeight)
{
  uint8_t block[4 * 4 * 4]{};

  for (int y = 0; y < pPixelsRGBAHeight; y += 4)
  {
    for (int x = 0; x < pPixelsRGBAWidth; x += 4)
    {
      int maxJ = std::min(pPixelsRGBAHeight - y, 4);
      int maxI = std::min(pPixelsRGBAWidth - x, 4);

      bc7decomp::unpack_bc7(pBlock, (bc7decomp::color_rgba*)block);

      for (int j = 0; j < maxJ; j++)
      {
        for (int i = 0; i < maxI; i++)
        {
          int currentJ = j;
          int currentI = i;

          int sourceOffset = (y + currentJ) * pPixelsRGBAWidth * 4 + (x + currentI) * 4;
          int destinationOffset = j * 4 * 4 + i * 4;

          *(unsigned int*)(pPixelsRGBA + sourceOffset) = *(unsigned int*)(block + destinationOffset);

          pPixelsRGBA[sourceOffset + 2] = block[destinationOffset + 0];
          pPixelsRGBA[sourceOffset + 0] = block[destinationOffset + 2];
        }
      }

      pBlock += 16;
    }
  }

  return 0;
}

class FzCoachWindow : public pw::Window {
  public:
    FzCoachWindow() : pw::Window("Forza Coach", 1080, 720) {};
    virtual ~FzCoachWindow() {}

    void loadSwatchbinFile(const std::string& path) {
      std::string truePath = "C:/Code/primwalk/" + path;
      std::ifstream file(truePath, std::ios_base::binary);

      if (!file.is_open()) {
        std::cout << "Could not open swatchbin file!\n";
        return;
      }

      /* --- File validity checks --- */
      // Header file identifier
      uint32_t fileID;
      file.read(reinterpret_cast<char*>(&fileID), sizeof(uint32_t));

      uint32_t reqID;
      char reqChars[] = "burG";
      memcpy(&reqID, &reqChars, sizeof(uint32_t));

      if (fileID ^ reqID) {
        std::cout << "Header tags are NOT the same" << std::endl;
        file.close();
        return;
      }

      // TXQU
      int8_t compression;
      uint8_t mipmapCount;
      uint32_t dataOffset; // tells the index of beginning of image data
      uint32_t width;
      uint32_t height;
      uint32_t numPixels;
      uint32_t bc16LinearSize;

      file.seekg(SB_COMPRESSION.startIndex);
      file.read(reinterpret_cast<char*>(&compression), SB_COMPRESSION.byteCount());

      file.seekg(SB_MIPMAPCOUNT.startIndex);
      file.read(reinterpret_cast<char*>(&mipmapCount), SB_MIPMAPCOUNT.byteCount());

      file.seekg(SB_HEADERLENGTH.startIndex);
      file.read(reinterpret_cast<char*>(&dataOffset), SB_HEADERLENGTH.byteCount());

      file.seekg(SB_WIDTH.startIndex);
      file.read(reinterpret_cast<char*>(&width), SB_WIDTH.byteCount());

      file.seekg(SB_HEIGHT.startIndex);
      file.read(reinterpret_cast<char*>(&height), SB_HEIGHT.byteCount());

      file.seekg(SB_LINEARSIZE.startIndex);
      file.read(reinterpret_cast<char*>(&numPixels), SB_LINEARSIZE.byteCount());

      file.seekg(SB_BC16LINEARSIZE.startIndex);
      file.read(reinterpret_cast<char*>(&bc16LinearSize), SB_BC16LINEARSIZE.byteCount());
      
      swatchWidth = width;
      swatchHeight = height;
      SwatchbinCompression trueCompression = swatchbinCompressions.at(compression);

      swatchData = std::vector<unsigned char>(numPixels * 4, 255);

      if (isBlockType(trueCompression)) {
        std::vector<uint8_t> block(16); // 16 byte block
        file.seekg(dataOffset);

        int readPixels = 0;
        while (file.tellg() != -1) {
          file.read(reinterpret_cast<char*>(block.data()), 16);

          std::vector<uint8_t> texel(4 * 4 * 4); // 4x4 pixel cluster
          BC7_Decode(block.data(), texel.data(), 4, 4);
          
          if (readPixels < numPixels) {
            size_t offset = readPixels % (swatchWidth * 4) + (readPixels / (swatchWidth * 4)) * swatchWidth * 16;
            size_t swatchIndex = 0;
            size_t iTimes4 = 0;

            for (size_t i = 0; i < 16; i++) {
              iTimes4 = i << 2;
              swatchIndex = ((i >> 2) << 2) * swatchWidth - ((i >> 2) << 4) + (i << 2);

              swatchData[offset + swatchIndex + 0] = texel[iTimes4 + 2];
              swatchData[offset + swatchIndex + 1] = texel[iTimes4 + 1];
              swatchData[offset + swatchIndex + 2] = texel[iTimes4 + 0];
              swatchData[offset + swatchIndex + 3] = texel[iTimes4 + 3];
            }
          }

          readPixels += 16;
        }
      }
      else {
        file.seekg(dataOffset); // jump to offset
        std::vector<uint8_t> data(numPixels);
        file.read(reinterpret_cast<char*>(data.data()), numPixels);

        for (size_t i = 0; i < numPixels; i++) {
          swatchData[i] = data[i];
        }
      }

      std::cout << "Swatchbin file read complete\n";

      file.close();
    }

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
      loadSwatchbinFile("assets/other/test2.swatchbin");

      // Textures
      auto homeIcon = pw::Texture2D::create("assets/icons/home.png");
      auto auctionIcon = pw::Texture2D::create("assets/icons/auction.png");
      auto brushIcon = pw::Texture2D::create("assets/icons/brush.png");
      auto settingsIcon = pw::Texture2D::create("assets/icons/settings.png");
      auto logoIcon = pw::Texture2D::create("assets/icons/fzcoach_logo.png", 4, VK_FORMAT_R8G8B8A8_UNORM);

      // Fonts
      auto headerFont = pw::Font::create("assets/fonts/opensans.ttf", 32, pw::FontWeight::ExtraBold);

      // Dashboard container
      dashboardGroup = &makeElement<pw::UIContainer>("DG", glm::vec2(0, 0), 50, m_Height);
      dashboardGroup->setBackgroundColor({ 30, 30, 30 });
      
      titleBar = &makeElement<pw::UIContainer>("h", glm::vec2(0, 0), m_Width, 30);
      titleBar->setBackgroundColor({ 10, 10, 10 });

      auto& auctionBotTitle = makeElement<pw::UILabel>("h", "Designs & Paints", glm::vec2(150, 70), headerFont);
      auto& logo = makeElement<pw::UIIconButton>("Logo", glm::vec2(1, 1), logoIcon->getWidth(), 32, logoIcon);

      auto& dashboardButton = dashboardGroup->makeElement<pw::UIIconButton>("D2", glm::vec2(10, 160), 30, 30, homeIcon);
      dashboardButton.setBackgroundColor({ 70, 70, 70, 0 });

      auto& auctionsButton = dashboardGroup->makeElement<pw::UIIconButton>("D3", glm::vec2(10, 210), 30, 30, auctionIcon);
      auctionsButton.setBackgroundColor({ 70, 70, 70, 0 });

      auto& designPaintsButton = dashboardGroup->makeElement<pw::UIIconButton>("D4", glm::vec2(10, 260), 30, 30, brushIcon);
      designPaintsButton.setBackgroundColor({ 70, 70, 70, 0 });

      auto& settingsButton = dashboardGroup->makeElement<pw::UIIconButton>("D5", glm::vec2(10, 310), 30, 30, settingsIcon);
      settingsButton.setBackgroundColor({ 70, 70, 70, 0 });

      slider = &makeElement<pw::UISlider>("slider", glm::vec2(800, 200), 200, 0, 2, 1);
      makeElement<pw::UITextField>("textField", glm::vec2(800, 300), 200, 30);

      // Design sub view window
      auto& designEditor = makeSubView(512, 512, { 200, 200 });
    }

    void onUpdate(float dt) override {
      if (m_ShowPreview) {
        captureFH5Window();
        m_GameFrame->updateData(pixelData.data());
      }

      //float ratio = swatchWidth / swatchHeight;
      //textureView->setHeight(swatchHeight * slider->getSliderValue());
      //textureView->setWidth(textureView->getHeight() * ratio);

      titleBar->setWidth(m_Width + 100);
      dashboardGroup->setHeight(m_Height + 100); // add some margins to prevent resize artifacts
    }

    private:
      pw::UIContainer* dashboardGroup = nullptr;
      pw::UIContainer* mainView = nullptr;
      pw::UIContainer* titleBar = nullptr;
      pw::UISlider* slider = nullptr;
      pw::UIIconButton* textureView = nullptr;
      HWND m_FH5Handle = NULL;
      int bitmapWidth;
      int bitmapHeight;
      int swatchWidth;
      int swatchHeight;
      std::vector<unsigned char> pixelData;
      std::vector<unsigned char> swatchData;
      pw::Texture2D* m_GameFrame;
      bool m_ShowPreview = false;
};

int main() {
  
  //loadSwatchbinFile("assets/other/test5.swatchbin");

  pw::Application& app = pw::Application::Instance(); // acquire app instance
  FzCoachWindow* fzMain = new FzCoachWindow();
  fzMain->setBackgroundColor({ 20, 20, 20 });
  fzMain->run();

  delete fzMain;
  return 0;
}

// std
#include <iostream>

// FZUI
#include "fzui/fzui.hpp"

class FzCoachWindow : public fz::Window {
  public:
    FzCoachWindow() : fz::Window("Forza Coach", 1080, 720) {};
    virtual ~FzCoachWindow() {
      delete m_HomeIcon;
      delete m_AuctionIcon;
      delete m_BrushIcon;
    }

    void onCreate() override {
      m_HomeIcon = fz::Texture::create("assets/icons/home.png");
      m_AuctionIcon = fz::Texture::create("assets/icons/auction.png");
      m_BrushIcon = fz::Texture::create("assets/icons/brush.png");
      m_BannerImage = fz::Texture::create("assets/textures/fh5_banner.jpg");

      // UI Elements
      dashboardButton = new fz::UIImageButton(m_HomeIcon, { 8, 100 }, 32, 32);
      dashboardButton->setBackgroundColor({ 0, 0, 0, 255 });
      dashboardButton->setHoverColor({ 128, 128, 158 });
      dashboardButton->setText("Dashboard");
  
      auctionButton = new fz::UIImageButton(m_AuctionIcon, { 8, 150 }, 32, 32);
      auctionButton->setBackgroundColor({ 255, 0, 0 });
      auctionButton->setHoverColor({ 255, 0, 0 });

      brushButton = new fz::UIImageButton(m_BrushIcon, { 8, 200 }, 32, 32);
      brushButton->setBackgroundColor({ 255, 255, 255 });

      roundedRect = new fz::UIImageButton(nullptr, { 200, 200 }, 200, 200);
      roundedRect->setBackgroundColor({ 255, 0, 0 });
      roundedRect->setBorderRadius(8);

      banner = new fz::UIImage(m_BannerImage, { 100, 0 }, 500 * m_BannerImage->getAspectRatio(), 500);
      banner->setBorderRadius(32);

      //addElement(title);
      addElement(dashboardButton);
      addElement(auctionButton);
      addElement(brushButton);
      addElement(banner);
      addElement(roundedRect);
    }

  private:
    fz::Texture* m_HomeIcon = nullptr;
    fz::Texture* m_AuctionIcon = nullptr;
    fz::Texture* m_BrushIcon = nullptr;
    fz::Texture* m_BannerImage = nullptr;

    // UI Elements
    fz::UILabel* title = nullptr;
    fz::UIImage* banner = nullptr;
    fz::UIImageButton* dashboardButton = nullptr;
    fz::UIImageButton* auctionButton = nullptr;
    fz::UIImageButton* brushButton = nullptr;
    fz::UIImageButton* roundedRect = nullptr;
};

int main() {
  fz::Application& app = fz::Application::Instance(); // acquire app instance
  FzCoachWindow* fzMain = new FzCoachWindow();
  
  fzMain->run();

  delete fzMain;
  return 0;
}

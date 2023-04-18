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
      // Resources
      m_MainFont = fz::FontFace::create("assets/fonts/catamaranb.ttf", 14);
      m_TitleFont = fz::FontFace::create("assets/fonts/catamaranb.ttf", 45);

      m_HomeIcon = fz::Texture::create("assets/icons/home.png");
      m_AuctionIcon = fz::Texture::create("assets/icons/auction.png");
      m_BrushIcon = fz::Texture::create("assets/icons/brush.png");
      m_BannerImage = fz::Texture::create("assets/textures/fh5_banner.jpg");

      // # UI Elements #
      banner = new fz::UIImage(m_BannerImage, { 500, 100 }, 240 * m_BannerImage->getAspectRatio(), 240);
      banner->setBorderRadius(32);

      dashboardTitle = new fz::UILabel("Dashboard", { 260, 50 }, { 255, 255 ,255 }, 50, m_TitleFont);

      addElement(banner);
      addElement(dashboardTitle);

      // Left navigation bar
      navBar = new fz::UIContainer(230, getHeight() - 29, {0, 29});
      navBar->setBackgroundColor({ 42, 42, 42 });

      // Dashboard button
      dashButtonContainer = new fz::UIContainer(200, 30, { 10, 30 });
      dashButtonContainer->setBackgroundColor({ 58, 58, 65 });
      dashButtonContainer->setBorderRadius(8);

      dashboardButton = new fz::UIImageButton(m_HomeIcon, { 0, 0 }, 32, 32);
      dashboardButton->setBackgroundColor({ 255, 255, 255 });
      dashboardButton->setHoverColor({ 255, 255, 255 });
      dashboardText = new fz::UILabel("DASHBOARD", { 50, 30 / 2 - m_MainFont->getMaxHeight() / 2}, {255, 255, 255}, 14, m_MainFont);

      dashButtonContainer->addElement(dashboardButton);
      dashButtonContainer->addElement(dashboardText);

      // Auction button
      auctionButtonContainer = new fz::UIContainer(200, 30, { 10, 90 });
      auctionButtonContainer->setBackgroundColor({ 58, 58, 65 });
      auctionButtonContainer->setBorderRadius(8);

      auctionButton = new fz::UIImageButton(m_AuctionIcon, { 0, 0 }, 32, 32);
      auctionButton->setBackgroundColor({ 255, 255, 255 });
      auctionButton->setHoverColor({ 255, 0, 0 });
      auctionText = new fz::UILabel("AUCTION HOUSE", { 50, 30 / 2 - m_MainFont->getMaxHeight() / 2 }, { 255, 255, 255 }, 14, m_MainFont);

      auctionButtonContainer->addElement(auctionButton);
      auctionButtonContainer->addElement(auctionText);

      // Brush button
      brushButtonContainer = new fz::UIContainer(200, 30, { 10, 150 });
      brushButtonContainer->setBackgroundColor({ 58, 58, 65 });
      brushButtonContainer->setBorderRadius(8);

      brushButton = new fz::UIImageButton(m_BrushIcon, { 0, 0 }, 32, 32);
      brushButton->setBackgroundColor({ 255, 255, 255 });
      brushText = new fz::UILabel("DESIGNS & PAINTS", { 50, 30 / 2 - m_MainFont->getMaxHeight() / 2 }, { 255, 255, 255 }, 14, m_MainFont);

      brushButtonContainer->addElement(brushButton);
      brushButtonContainer->addElement(brushText);
      
      navBar->addContainer(dashButtonContainer);
      navBar->addContainer(auctionButtonContainer);
      navBar->addContainer(brushButtonContainer);
      addContainer(navBar);
    }

    void onUpdate(float dt) override {
      navBar->setHeight(getHeight());
    }

  private:
    // Resources
    fz::FontFace* m_MainFont= nullptr;
    fz::FontFace* m_TitleFont = nullptr;

    fz::Texture* m_HomeIcon = nullptr;
    fz::Texture* m_AuctionIcon = nullptr;
    fz::Texture* m_BrushIcon = nullptr;
    fz::Texture* m_BannerImage = nullptr;

    // UI Elements
    fz::UIContainer* navBar = nullptr;
    fz::UIContainer* dashButtonContainer = nullptr;
    fz::UIContainer* auctionButtonContainer = nullptr;
    fz::UIContainer* brushButtonContainer = nullptr;

    fz::UILabel* dashboardTitle = nullptr;
    fz::UILabel* dashboardText = nullptr;
    fz::UILabel* auctionText = nullptr;
    fz::UILabel* brushText = nullptr;
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

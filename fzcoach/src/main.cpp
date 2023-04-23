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
      m_SettingsIcon = fz::Texture::create("assets/icons/settings.png");

      // # UI Elements #
      banner = new fz::UIImage(m_BannerImage, { 500, 100 }, 240 * m_BannerImage->getAspectRatio(), 240);
      banner->setBorderRadius(32);

      dashboardTitle = new fz::UILabel("Dashboard", { 260, 50 }, { 255, 255 ,255 }, 50, m_TitleFont);

      //addElement(banner);
      addElement(dashboardTitle);

      // Left navigation bar
      navBar = new fz::UIContainer(230, getHeight() - 29, {0, 29});
      navBar->setBackgroundColor({ 42, 42, 42 });

      // Dashboard button
      dashButtonContainer = new fz::UIContainer(200, 30, { 10, 30 });
      dashButtonContainer->setBackgroundColor({ 58, 58, 65 });
      dashButtonContainer->setBorderRadius(8);

      dashboardButton = new fz::UIImage(m_HomeIcon, { 0, 0 }, 32, 32);
      dashboardButton->setColor({ 255, 255, 255 });
      dashboardText = new fz::UILabel("DASHBOARD", { 50, 30 / 2 - m_MainFont->getMaxHeight() / 2}, {255, 255, 255}, 14, m_MainFont);

      dashButtonContainer->addElement(dashboardButton);
      dashButtonContainer->addElement(dashboardText);

      // Auction button
      auctionButtonContainer = new fz::UIContainer(200, 30, { 10, 90 });
      auctionButtonContainer->setBackgroundColor({ 58, 58, 65 });
      auctionButtonContainer->setBorderRadius(8);

      auctionButton = new fz::UIImage(m_AuctionIcon, { 0, 0 }, 32, 32);
      auctionButton->setColor({ 255, 255, 255 });
      auctionText = new fz::UILabel("BUY AND SELL", { 50, 30 / 2 - m_MainFont->getMaxHeight() / 2 }, { 255, 255, 255 }, 14, m_MainFont);

      auctionButtonContainer->addElement(auctionButton);
      auctionButtonContainer->addElement(auctionText);

      // Brush button
      brushButtonContainer = new fz::UIContainer(200, 30, { 10, 150 });
      brushButtonContainer->setBackgroundColor({ 58, 58, 65 });
      brushButtonContainer->setBorderRadius(8);

      brushButton = new fz::UIImage(m_BrushIcon, { 0, 0 }, 32, 32);
      brushButton->setColor({ 255, 255, 255 });
      brushText = new fz::UILabel("DESIGNS", { 50, 30 / 2 - m_MainFont->getMaxHeight() / 2 }, { 255, 255, 255 }, 14, m_MainFont);

      brushButtonContainer->addElement(brushButton);
      brushButtonContainer->addElement(brushText);

      // Settings button
      settingsButtonContainer = new fz::UIContainer(200, 30, { 10, 210 });
      settingsButtonContainer->setBackgroundColor({ 58, 58, 65 });
      settingsButtonContainer->setBorderRadius(8);

      settingsButton = new fz::UIImage(m_SettingsIcon, { 0, 0 }, 32, 32);
      settingsButton->setColor({ 255, 255, 255 });
      settingsText = new fz::UILabel("SETTINGS", { 50, 30 / 2 - m_MainFont->getMaxHeight() / 2 }, { 255, 255, 255 }, 14, m_MainFont);

      settingsButtonContainer->addElement(settingsButton);
      settingsButtonContainer->addElement(settingsText);
      
      navBar->addContainer(dashButtonContainer);
      navBar->addContainer(auctionButtonContainer);
      navBar->addContainer(brushButtonContainer);
      navBar->addContainer(settingsButtonContainer);
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
    fz::Texture* m_SettingsIcon = nullptr;

    // UI Elements
    fz::UIContainer* navBar = nullptr;
    fz::UIContainer* dashButtonContainer = nullptr;
    fz::UIContainer* auctionButtonContainer = nullptr;
    fz::UIContainer* brushButtonContainer = nullptr;
    fz::UIContainer* settingsButtonContainer = nullptr;

    fz::UILabel* dashboardTitle = nullptr;
    fz::UILabel* dashboardText = nullptr;
    fz::UILabel* auctionText = nullptr;
    fz::UILabel* brushText = nullptr;
    fz::UILabel* settingsText = nullptr;
    fz::UIImage* banner = nullptr;
    fz::UIImage* dashboardButton = nullptr;
    fz::UIImage* auctionButton = nullptr;
    fz::UIImage* brushButton = nullptr;
    fz::UIImage* settingsButton = nullptr;
    fz::UIImageButton* roundedRect = nullptr;
};

int main() {
  fz::Application& app = fz::Application::Instance(); // acquire app instance
  FzCoachWindow* fzMain = new FzCoachWindow();
  
  fzMain->run();

  delete fzMain;
  return 0;
}

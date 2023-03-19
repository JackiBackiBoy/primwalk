// std
#include <iostream>

// FZUI
#include "fzui/window.hpp"
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
      fz::FontManager::Instance().addFont("Big Font", 72, "assets/fonts/segoeui.ttf");
      m_BigFont = fz::FontManager::Instance().getFont("Big Font");
      m_HomeIcon = fz::Texture::create("assets/icons/home.png");
      m_AuctionIcon = fz::Texture::create("assets/icons/auction.png");
      m_BrushIcon = fz::Texture::create("assets/icons/brush.png");

      // UI Elements
      title = new fz::UILabel("Forza Coach", { getWidth() / 2 - m_BigFont->getTextWidth("Forza Coach", 72.0f) / 2, 100 }, { 255, 255, 255 }, 72.0f, m_BigFont);
      dashboardButton = new fz::UIImageButton(m_HomeIcon, { 8, 100 }, 32, 32);
      dashboardButton->setHoverColor({ 128, 128, 158 });
  
      auctionButton = new fz::UIImageButton(m_AuctionIcon, { 8, 150 }, 32, 32);
      brushButton = new fz::UIImageButton(m_BrushIcon, { 8, 200 }, 32, 32);

      addElement(title);
      addElement(dashboardButton);
      addElement(auctionButton);
      addElement(brushButton);
    }
    
    void onUpdate(const float& dt) override {
      title->setPosition({ (float)getWidth() / 2 - m_BigFont->getTextWidth("Forza Coach", 72.0f) / 2, title->getPosition().y });
    }

    void onRender(const float& dt) override {

    }

  private:
    fz::FontFace* m_BigFont;
    fz::Texture* m_HomeIcon;
    fz::Texture* m_AuctionIcon;
    fz::Texture* m_BrushIcon;

    // UI Elements
    fz::UILabel* title;
    fz::UIImageButton* dashboardButton;
    fz::UIImageButton* auctionButton;
    fz::UIImageButton* brushButton;
};

int main() {
  fz::Application& app = fz::Application::Instance(); // acquire app instance
  FzCoachWindow* fzMain = new FzCoachWindow();
  fzMain->run();

  delete fzMain;
  return 0;
}

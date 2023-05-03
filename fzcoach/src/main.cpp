// std
#include <iostream>

// FZUI
#include "fzui/fzui.hpp"

class FzCoachWindow : public fz::Window {
  public:
    FzCoachWindow() : fz::Window("Forza Coach", 1080, 720, fz::GraphicsAPI::Vulkan) {};
    virtual ~FzCoachWindow() {
      delete m_HomeIcon;
      delete m_AuctionIcon;
      delete m_BrushIcon;
    }

    void onCreate() override {
      
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

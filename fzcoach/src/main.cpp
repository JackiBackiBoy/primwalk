// std
#include <iostream>

// FZUI
#include "fzui/window.hpp"
#include "fzui/fzui.hpp"

int main() {
  fz::Application& app = fz::Application::Instance(); // acquire app instance

  // ------ Main Window ------
  fz::Window* fzMain = new fz::Window("Forza Coach", 1080, 720);

  // Resources
  fz::FontManager::Instance().addFont("Big Font", 72, "assets/fonts/segoeui.ttf");
  fz::FontFace* bigFont = fz::FontManager::Instance().getFont("Big Font");

  fz::UILabel* title = new fz::UILabel("Forza Coach", { fzMain->getWidth() / 2, 100 }, { 255, 255, 255 }, 72.0f, bigFont);

  // Left nav bar
  fz::Texture* homeIcon = fz::Texture::create("assets/icons/home.png");
  fz::Texture* auctionIcon = fz::Texture::create("assets/icons/auction.png");
  fz::Texture* brushIcon = fz::Texture::create("assets/icons/brush.png");
  fz::UIImageButton* dashboardButton = new fz::UIImageButton(homeIcon, { 8, 100 }, 32, 32);
  dashboardButton->setHoverColor({ 128, 128, 158 });
  
  fz::UIImageButton* auctionButton = new fz::UIImageButton(auctionIcon, { 8, 150 }, 32, 32);
  fz::UIImageButton* brushButton = new fz::UIImageButton(brushIcon, { 8, 200 }, 32, 32);

  fzMain->addElement(title);
  fzMain->addElement(dashboardButton);
  fzMain->addElement(auctionButton);
  fzMain->addElement(brushButton);

  fzMain->run();

  delete fzMain;

  return 0;
}

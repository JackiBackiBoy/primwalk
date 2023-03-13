// std
#include <iostream>

// FZUI
#include "fzui/window.hpp"
#include "fzui/fzui.hpp"

int main() {
  fz::Application& app = fz::Application::Instance(); // acquire app instance

  // ------ Main Window ------
  fz::Window* fzMain = new fz::Window(L"Forza Coach (Beta)", 1080, 720);

  // On start
  fz::UIButton* button = new fz::UIButton("OpenGL test 2!", { 400, 30 }, 100, 50);
  button->setHoverColor({ 255, 0, 0 });
  fz::UILabel* title = new fz::UILabel("Forza Coach", { 100, 100 });

  fz::Texture* homeIcon = fz::Texture::create("assets/icons/home.png");
  fz::UIImage* dashboardButton = new fz::UIImage(homeIcon, { 0, 100 }, 32, 32);

  fzMain->addElement(button);
  fzMain->addElement(title);
  fzMain->addElement(dashboardButton);

  fzMain->run();

  delete fzMain;

  return 0;
}

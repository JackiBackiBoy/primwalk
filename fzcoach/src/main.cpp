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

  fzMain->addElement(button);

  fzMain->run();

  delete fzMain;

  return 0;
}

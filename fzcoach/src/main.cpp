// std
#include <iostream>

// FZUI
#include "fzui/window.hpp"
#include "fzui/fzui.hpp"

int main() {

  fz::Application& app = fz::Application::Instance(); // acquire app instance

  // ------ Main Window ------
  fz::Window* fzMain = new fz::Window(L"Forza Coach (Beta)", 720, 360);
  fzMain->run();

  delete fzMain;

  return 0;
}

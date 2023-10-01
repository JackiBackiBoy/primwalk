#define NOMINMAX

// std
#include <memory>

// primwalk
#include "primwalk/primwalk.hpp"

int main() {
  std::unique_ptr<pw::Window> fzMain = std::make_unique<pw::Window>("Primwalk Engine", 1080, 720);
  fzMain->setBackgroundColor({ 255, 0, 0 });

  pw::Application application = pw::Application();
  application.setWindow(fzMain.get());

  application.run();

  return 0;
}

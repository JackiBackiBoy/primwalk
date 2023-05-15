// std
#include <iostream>

// FZUI
#include "fzui/fzui.hpp"

class FzCoachWindow : public fz::Window {
  public:
    FzCoachWindow() : fz::Window("Forza Coach", 1080, 720) {};
    virtual ~FzCoachWindow() {
    }

    void onCreate() override {
      
    }

    void onUpdate(float dt) override {
    }

  private:
};

int main() {
  fz::Application& app = fz::Application::Instance(); // acquire app instance
  FzCoachWindow* fzMain = new FzCoachWindow();
  
  fzMain->run();

  delete fzMain;
  return 0;
}

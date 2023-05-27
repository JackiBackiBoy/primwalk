// std
#include <iostream>
#include <memory>

// FZUI
#include "fzui/fzui.hpp"

class FzCoachWindow : public fz::Window {
  public:
    FzCoachWindow() : fz::Window("Forza Coach", 1080, 720) {};
    virtual ~FzCoachWindow() {}

    void onCreate() override {
      m_DashboardButton = &makeElement<fz::UIButton>("D1", "Text", glm::vec2(30, 30), 100, 100);
      makeElement<fz::UIButton>("D2", "Text", glm::vec2(0, 0), getWidth(), 30);
    }

    void onUpdate(float dt) override {
      static float time = 0.0f;
      time += dt;

      float factorR = sin(time) * 0.5f + 0.5f;
      float factorG = cos(time) * 0.5f + 0.5f;

      m_DashboardButton->setBackgroundColor({ (uint8_t)(factorR * 255), (uint8_t)(factorG * 255), 0});
      m_DashboardButton->setPosition({ factorG * 300, factorR * 300 });
    }

  private:
    fz::UIButton* m_DashboardButton = nullptr;
};

int main() {
  fz::Application& app = fz::Application::Instance(); // acquire app instance
  FzCoachWindow* fzMain = new FzCoachWindow();
  fzMain->run();

  delete fzMain;
  return 0;
}

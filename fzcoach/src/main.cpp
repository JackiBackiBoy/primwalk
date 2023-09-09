// std
#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <fstream>

// primwalk
#include "primwalk/primwalk.hpp"

class FzCoachWindow : public pw::Window {
  public:
    FzCoachWindow() : pw::Window("Forza Coach", 1080, 720) {};
    virtual ~FzCoachWindow() {}

    void onCreate() override {
    }

    void onUpdate(float dt) override {
    }

    private:
      int bitmapWidth;
      int bitmapHeight;
      int swatchWidth;
      int swatchHeight;
      std::vector<unsigned char> pixelData;
      std::vector<unsigned char> swatchData;
      bool m_ShowPreview = false;
};

int main() {
  std::unique_ptr<FzCoachWindow> fzMain = std::make_unique<FzCoachWindow>();
  fzMain->setBackgroundColor({ 255, 0, 0 });

  pw::Application application = pw::Application();
  application.setWindow(fzMain.get());

  application.run();

  return 0;
}

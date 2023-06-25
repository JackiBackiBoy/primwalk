#ifndef FZ_UI_EVENT_HEADER
#define FZ_UI_EVENT_HEADER

// FZUI
#include "fzui/core.hpp"
#include "fzui/mouseButtons.hpp"

// std
#include <cassert>
#include <memory>
#include <cstdint>

// vendor
#include <glm/glm.hpp>

namespace fz {
  enum class UIEventType : uint32_t {
    None = 0x0000,

    // Mouse
    MouseMove = 0x0001,
    MouseDrag = 0x0002,
    MouseDown = 0x0003,
    MouseUp = 0x0004,
    MouseWheel = 0x0005,
    MouseEnter = 0x0006,
    MouseExit = 0x0007,
    MouseExitWindow = 0x0008,

    // Pure keyboard events
    KeyboardDown = 0x0010,
    KeyboardUp = 0x0020,
    KeyboardEnter = 0x0030,
    KeyboardExit = 0x0040,
  };

  struct FZ_API MouseEventData {
    glm::vec2 position = {};
    MouseButtons causeButtons = {};
    MouseButtons downButtons = {};
    int clickCount = 0;
  };

  class FZ_API UIEvent {
  public:
    UIEvent(UIEventType type) {
      setType(type);
    };

    // Getters
    inline UIEventType getType() const { return m_Type; }
    inline MouseEventData& getMouseData() const {
      assert(m_EventMask == MOUSE_EVENT_MASK && "ASSERTION FAILED: Can not acquire mouse data from non-mouse event!");
      return *std::static_pointer_cast<MouseEventData>(m_Data);
    }

    // Setters
    void setType(UIEventType type);

    static constexpr uint32_t MOUSE_EVENT_MASK = 0x0F;
    static constexpr uint32_t KEYBOARD_EVENT_MASK = 0xF0;

  private:
    UIEventType m_Type = UIEventType::None;
    std::shared_ptr<void> m_Data;
    uint32_t m_EventMask = 0x0000;
  };
}
#endif
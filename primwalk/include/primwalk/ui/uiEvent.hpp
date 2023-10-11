#pragma once

// primwalk
#include "primwalk/core.hpp"
#include "primwalk/mouseButtons.hpp"
#include "primwalk/input/keycode.hpp"

// std
#include <cassert>
#include <memory>
#include <cstdint>

// vendor
#include <glm/glm.hpp>

namespace pw {
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

    // Non-pure keyboard events
    KeyboardChar = 0x0500,

    // Navigation and focus
    FocusLost = 0x1000,
  };

  struct PW_API MouseEventData {
    glm::vec2 position = {};
    glm::vec2 wheelDelta = {};
    MouseButtons causeButtons = {};
    MouseButtons downButtons = {};
    int clickCount = 0;
  };

  struct PW_API KeyboardEventData {
    KeyCode pressedKey = KeyCode::None;
    KeyModifier modifier = KeyModifier::None;
  };

  struct PW_API KeyboardCharData {
    uint32_t codePoint = 0;
  };

  class PW_API UIEvent {
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
    inline KeyboardEventData& getKeyboardData() const {
      assert(m_EventMask == KEYBOARD_EVENT_MASK && "ASSERTION FAILED: Can not acquire keyboard data from non-keyboard event!");
      return *std::static_pointer_cast<KeyboardEventData>(m_Data);
    }
    inline KeyboardCharData& getCharData() const {
      assert(m_EventMask == NON_PURE_KEYBOARD_EVENT_MASK && "ASSERTION FAILED: Can not acquire char data, type must be non-pure!");
      return *std::static_pointer_cast<KeyboardCharData>(m_Data);
    }

    // Setters
    void setType(UIEventType type);

    static constexpr uint32_t MOUSE_EVENT_MASK = 0x000F;
    static constexpr uint32_t KEYBOARD_EVENT_MASK = 0x00F0;
    static constexpr uint32_t NON_PURE_KEYBOARD_EVENT_MASK = 0x0F00;

  private:
    UIEventType m_Type = UIEventType::None;
    std::shared_ptr<void> m_Data;
    uint32_t m_EventMask = 0x0000;
  };
}


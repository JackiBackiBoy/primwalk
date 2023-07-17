#include "primwalk/ui/uiEvent.hpp"

namespace pw {

  void UIEvent::setType(UIEventType type)
  {
    uint32_t numType = static_cast<uint32_t>(type);

    if (type == m_Type) { return; }

    m_Type = type;

    if ((numType & m_EventMask) != 0) { return; }

    if ((numType & MOUSE_EVENT_MASK) != 0) { // mouse-related type
      m_EventMask = MOUSE_EVENT_MASK;
      m_Data = std::make_shared<MouseEventData>();
    }
    else if ((numType & KEYBOARD_EVENT_MASK) != 0) { // keyboard-related type
      m_EventMask = KEYBOARD_EVENT_MASK;
    }
  }

}
#include "primwalk/input/input.hpp"
#include "primwalk/input/rawInput.hpp"

#include <windows.h>

namespace pw::input {
  pw::input::MouseState mouse;
  pw::input::KeyboardState keyboard;

  void initialize()
  {

  }

  void getKeyboardState(KeyboardState* state)
  {
    *state = keyboard;
  }

  void getMouseState(MouseState* state)
  {
    *state = mouse;
  }

  void update()
  {
    pw::input::rawinput::update();

#if defined(PW_WIN32)
    pw::input::rawinput::getMouseState(&mouse);
    pw::input::rawinput::getKeyboardState(&keyboard);

    mouse.leftDown |= (GetAsyncKeyState(VK_LBUTTON) < 0);
    mouse.rightDown |= (GetAsyncKeyState(VK_RBUTTON) < 0);
    mouse.middleDown |= (GetAsyncKeyState(VK_MBUTTON) < 0);
#endif
  }

  bool isDown(const KeyCode& button)
  {
    uint8_t keycode = (uint8_t)button;

    switch (button) {
    case KeyCode::MouseButtonLeft:
      return mouse.leftDown;
    case KeyCode::MouseButtonRight:
      return mouse.rightDown;
    case KeyCode::MouseButtonMiddle:
      return mouse.middleDown;
    }

#if defined(PW_WIN32)
    switch (button) {
    case KeyCode::KeyboardButtonUp:
      keycode = VK_UP;
      break;
    case KeyCode::KeyboardButtonDown:
      keycode = VK_DOWN;
      break;
    case KeyCode::KeyboardButtonLeft:
      keycode = VK_LEFT;
      break;
    case KeyCode::KeyboardButtonRight:
      keycode = VK_RIGHT;
      break;
    case KeyCode::KeyboardButtonSpace:
      keycode = VK_SPACE;
      break;
    case KeyCode::KeyboardButtonRShift:
      keycode = VK_RSHIFT;
      break;
    case KeyCode::KeyboardButtonLShift:
      keycode = VK_LSHIFT;
      break;
    case KeyCode::KeyboardButtonF1:
      keycode = VK_F1;
      break;
    case KeyCode::KeyboardButtonF2:
      keycode = VK_F2;
      break;
    case KeyCode::KeyboardButtonF3:
      keycode = VK_F3;
      break;
    case KeyCode::KeyboardButtonF4:
      keycode = VK_F4;
      break;
    case KeyCode::KeyboardButtonF5:
      keycode = VK_F5;
      break;
    case KeyCode::KeyboardButtonF6:
      keycode = VK_F6;
      break;
    case KeyCode::KeyboardButtonF7:
      keycode = VK_F7;
      break;
    case KeyCode::KeyboardButtonF8:
      keycode = VK_F8;
      break;
    case KeyCode::KeyboardButtonF9:
      keycode = VK_F9;
      break;
    case KeyCode::KeyboardButtonF10:
      keycode = VK_F10;
      break;
    case KeyCode::KeyboardButtonF11:
      keycode = VK_F11;
      break;
    case KeyCode::KeyboardButtonF12:
      keycode = VK_F12;
      break;
    case KeyCode::KeyboardButtonEnter:
      keycode = VK_RETURN;
      break;
    case KeyCode::KeyboardButtonEscape:
      keycode = VK_ESCAPE;
      break;
    case KeyCode::KeyboardButtonHome:
      keycode = VK_HOME;
      break;
    case KeyCode::KeyboardButtonLControl:
      keycode = VK_LCONTROL;
      break;
    case KeyCode::KeyboardButtonRControl:
      keycode = VK_RCONTROL;
      break;
    case KeyCode::KeyboardButtonDelete:
      keycode = VK_DELETE;
      break;
    case KeyCode::KeyboardButtonBackspace:
      keycode = VK_BACK;
      break;
    case KeyCode::KeyboardButtonPageDown:
      keycode = VK_NEXT;
      break;
    case KeyCode::KeyboardButtonPageUp:
      keycode = VK_PRIOR;
      break;
    case KeyCode::KeyboardButtonNumpad0:
      keycode = VK_NUMPAD0;
      break;
    case KeyCode::KeyboardButtonNumpad1:
      keycode = VK_NUMPAD1;
      break;
    case KeyCode::KeyboardButtonNumpad2:
      keycode = VK_NUMPAD2;
      break;
    case KeyCode::KeyboardButtonNumpad3:
      keycode = VK_NUMPAD3;
      break;
    case KeyCode::KeyboardButtonNumpad4:
      keycode = VK_NUMPAD4;
      break;
    case KeyCode::KeyboardButtonNumpad5:
      keycode = VK_NUMPAD5;
      break;
    case KeyCode::KeyboardButtonNumpad6:
      keycode = VK_NUMPAD6;
      break;
    case KeyCode::KeyboardButtonNumpad7:
      keycode = VK_NUMPAD7;
      break;
    case KeyCode::KeyboardButtonNumpad8:
      keycode = VK_NUMPAD8;
      break;
    case KeyCode::KeyboardButtonNumpad9:
      keycode = VK_NUMPAD9;
      break;
    case KeyCode::KeyboardButtonMultiply:
      keycode = VK_MULTIPLY;
      break;
    case KeyCode::KeyboardButtonAdd:
      keycode = VK_ADD;
      break;
    case KeyCode::KeyboardButtonSeparator:
      keycode = VK_SEPARATOR;
      break;
    case KeyCode::KeyboardButtonSubtract:
      keycode = VK_SUBTRACT;
      break;
    case KeyCode::KeyboardButtonDecimal:
      keycode = VK_DECIMAL;
      break;
    case KeyCode::KeyboardButtonDivide:
      keycode = VK_DIVIDE;
      break;
    default:
      break;
    }

    return (GetAsyncKeyState((uint8_t)keycode) < 0);
#endif
  }

}
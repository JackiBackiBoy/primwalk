#pragma once

// std
#include <cstdint>

#ifdef WIN32
#include <windows.h>
#endif

namespace pw {
  enum class KeyModifier : uint8_t {
    None = 0x00,
    Shift = 0x01, ///< The shift key is being held.
    Control = 0x02, ///< The control key is being held.
    Alt = 0x04, ///< The alt-key, option-key or meta-key is being held.
    Super = 0x08, ///< The windows-key, key-key or super-key is being held.
  };

  constexpr KeyModifier operator|(KeyModifier const& lhs, KeyModifier const& rhs) noexcept
  {
    return static_cast<KeyModifier>(uint8_t(lhs) | uint8_t(rhs));
  }

  constexpr KeyModifier operator&(KeyModifier const& lhs, KeyModifier const& rhs) noexcept
  {
    return static_cast<KeyModifier>(uint8_t(lhs) & uint8_t(rhs));
  }

  constexpr KeyModifier& operator|=(KeyModifier& lhs, KeyModifier const& rhs) noexcept
  {
    return lhs = lhs | rhs;
  }

  enum class KeyCode : int {
    Unknown = -1,

    // Printable keys
    Space = 32,
    Apostrophe = 39,
    Comma = 44,
    Minus = 45,
    Period = 46,
    Slash = 47,
    Alpha0 = 48,
    Alpha1 = 49,
    Alpha2 = 50,
    Alpha3 = 51,
    Alpha4 = 52,
    Alpha5 = 53,
    Alpha6 = 54,
    Alpha7 = 55,
    Alpha8 = 56,
    Alpha9 = 57,
    SemiColon = 59,
    Equals = 61,
    A = 65,
    B = 66,
    C = 67,
    D = 68,
    E = 69,
    F = 70,
    G = 71,
    H = 72,
    I = 73,
    J = 74,
    K = 75,
    L = 76,
    M = 77,
    N = 78,
    O = 79,
    P = 80,
    Q = 81,
    R = 82,
    S = 83,
    T = 84,
    U = 85,
    V = 86,
    W = 87,
    X = 88,
    Y = 89,
    Z = 90,
    LeftBracket = 91,
    Backslash = 92,
    RightBracket = 93,
    GraveAccent = 96,
    //WORLD_1 = 161,
    //WORLD_2 = 162,

    // Function keys
    Escape = 256,
    Enter = 257,
    Tab = 258,
    Backspace = 259,
    Insert = 260,
    Delete = 261,
    Right = 262,
    Left = 263,
    Down = 264,
    Up = 265,
    PageUp = 266,
    PageDown = 267,
    Home = 268,
    End = 269,
    CapsLock = 280,
    ScrollLock = 281,
    NumLock = 282,
    PrintScreen = 283,
    Pause = 284,
    F1 = 290,
    F2 = 291,
    F3 = 292,
    F4 = 293,
    F5 = 294,
    F6 = 295,
    F7 = 296,
    F8 = 297,
    F9 = 298,
    F10 = 299,
    F11 = 300,
    F12 = 301,
    F13 = 302,
    F14 = 303,
    F15 = 304,
    F16 = 305,
    F17 = 306,
    F18 = 307,
    F19 = 308,
    F20 = 309,
    F21 = 310,
    F22 = 311,
    F23 = 312,
    F24 = 313,
    F25 = 314,
    Keypad0 = 320,
    Keypad1 = 321,
    Keypad2 = 322,
    Keypad3 = 323,
    Keypad4 = 324,
    Keypad5 = 325,
    Keypad6 = 326,
    Keypad7 = 327,
    Keypad8 = 328,
    Keypad9 = 329,
    KeypadDecimal = 330,
    KeypadDivide = 331,
    KeypadMultiply = 332,
    KeypadSubtract = 333,
    KeypadAdd = 334,
    KeypadEnter = 335,
    KeypadEqual = 336,
    LeftShift = 340,
    LeftControl = 341,
    LeftAlt = 342,
    LeftSuper = 343,
    RightShift = 344,
    RightControl = 345,
    RightAlt = 346,
    RightSuper = 347,
    Menu = 348
  };
}

namespace pw::input {
#ifdef WIN32
    inline static KeyCode toKeyCode(int keyCode) {
      switch (keyCode) {
      case VK_MENU:
        return KeyCode::Menu;
      case VK_BACK:
        return KeyCode::Backspace;
      case VK_TAB:
        return KeyCode::Tab;
      case VK_RETURN:
        return KeyCode::Enter;
      case VK_PAUSE:
        return KeyCode::Pause;
      case VK_ESCAPE:
        return KeyCode::Escape;
      case VK_SPACE:
        return KeyCode::Space;
      case VK_PRIOR:
        return KeyCode::PageUp;
      case VK_NEXT:
        return KeyCode::PageDown;
      case VK_END:
        return KeyCode::End;
      case VK_HOME:
        return KeyCode::Home;
      case VK_LEFT:
        return KeyCode::Left;
      case VK_UP:
        return KeyCode::Up;
      case VK_RIGHT:
        return KeyCode::Right;
      case VK_DOWN:
        return KeyCode::Down;
      case VK_PRINT:
        return KeyCode::PrintScreen;
      case VK_SNAPSHOT:
        return KeyCode::PrintScreen;
      case VK_INSERT:
        return KeyCode::Insert;
      case VK_DELETE:
        return KeyCode::Delete;
      case '0':
        return KeyCode::Alpha0;
      case '1':
        return KeyCode::Alpha1;
      case '2':
        return KeyCode::Alpha2;
      case '3':
        return KeyCode::Alpha3;
      case '4':
        return KeyCode::Alpha4;
      case '5':
        return KeyCode::Alpha5;
      case '6':
        return KeyCode::Alpha6;
      case '7':
        return KeyCode::Alpha7;
      case '8':
        return KeyCode::Alpha8;
      case '9':
        return KeyCode::Alpha9;
      case 'A':
        return KeyCode::A;
      case 'B':
        return KeyCode::B;
      case 'C':
        return KeyCode::C;
      case 'D':
        return KeyCode::D;
      case 'E':
        return KeyCode::E;
      case 'F':
        return KeyCode::F;
      case 'G':
        return KeyCode::G;
      case 'H':
        return KeyCode::H;
      case 'I':
        return KeyCode::I;
      case 'J':
        return KeyCode::J;
      case 'K':
        return KeyCode::K;
      case 'L':
        return KeyCode::L;
      case 'M':
        return KeyCode::M;
      case 'N':
        return KeyCode::N;
      case 'O':
        return KeyCode::O;
      case 'P':
        return KeyCode::P;
      case 'Q':
        return KeyCode::Q;
      case 'R':
        return KeyCode::R;
      case 'S':
        return KeyCode::S;
      case 'T':
        return KeyCode::T;
      case 'U':
        return KeyCode::U;
      case 'V':
        return KeyCode::V;
      case 'W':
        return KeyCode::W;
      case 'X':
        return KeyCode::X;
      case 'Y':
        return KeyCode::Y;
      case 'Z':
        return KeyCode::Z;
      case VK_NUMPAD0:
        return KeyCode::Alpha0;
      case VK_NUMPAD1:
        return KeyCode::Alpha1;
      case VK_NUMPAD2:
        return KeyCode::Alpha2;
      case VK_NUMPAD3:
        return KeyCode::Alpha3;
      case VK_NUMPAD4:
        return KeyCode::Alpha4;
      case VK_NUMPAD5:
        return KeyCode::Alpha5;
      case VK_NUMPAD6:
        return KeyCode::Alpha6;
      case VK_NUMPAD7:
        return KeyCode::Alpha7;
      case VK_NUMPAD8:
        return KeyCode::Alpha8;
      case VK_NUMPAD9:
        return KeyCode::Alpha9;
      case VK_MULTIPLY:
        return KeyCode::KeypadMultiply;
      case VK_ADD:
        return KeyCode::KeypadAdd;
      case VK_SUBTRACT:
        return KeyCode::KeypadSubtract;
      case VK_DECIMAL:
        return KeyCode::Period;
      case VK_DIVIDE:
        return KeyCode::Slash;
      case VK_F1:
        return KeyCode::F1;
      case VK_F2:
        return KeyCode::F2;
      case VK_F3:
        return KeyCode::F3;
      case VK_F4:
        return KeyCode::F4;
      case VK_F5:
        return KeyCode::F5;
      case VK_F6:
        return KeyCode::F6;
      case VK_F7:
        return KeyCode::F7;
      case VK_F8:
        return KeyCode::F8;
      case VK_F9:
        return KeyCode::F9;
      case VK_F10:
        return KeyCode::F10;
      case VK_F11:
        return KeyCode::F11;
      case VK_F12:
        return KeyCode::F12;
      case VK_F13:
        return KeyCode::F13;
      case VK_F14:
        return KeyCode::F14;
      case VK_F15:
        return KeyCode::F15;
      case VK_F16:
        return KeyCode::F16;
      case VK_F17:
        return KeyCode::F17;
      case VK_F18:
        return KeyCode::F18;
      case VK_F19:
        return KeyCode::F19;
      case VK_F20:
        return KeyCode::F20;
      case VK_F21:
        return KeyCode::F21;
      case VK_F22:
        return KeyCode::F22;
      case VK_F23:
        return KeyCode::F23;
      case VK_F24:
        return KeyCode::F24;
      default:
        return KeyCode::Unknown;
      }
    }
  }
#endif

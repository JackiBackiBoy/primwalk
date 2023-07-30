#include "primwalk/ui/uiTextField.hpp"
#include "primwalk/input/keycode.hpp"
#include <iostream>

namespace pw {

  void UITextField::onRender(UIRenderSystem& renderer)
  {
    // Input field box
    renderer.drawRect(getAbsolutePosition(), m_Width, m_Height, m_DisplayBorderColor); // border
    renderer.drawRect(getAbsolutePosition() + glm::vec2(1, 1), m_Width - 2, m_Height - 2, m_DisplayBackgroundColor); // background

    // Text rendering
    if (!m_Text.empty()) {
      glm::vec2 textPos = { 1 + 10, 1 + (m_Height - 2) / 2 - m_Font->getMaxHeight() / 2 };
      renderer.drawText(getAbsolutePosition() + textPos, m_Text, 15, Color::White);
    }

    // Text highlighting
    if (m_IsHighlighting) {
      float textWidth = m_Font->getTextWidth(m_Text, 15);
      float leftTextX = getAbsolutePosition().x + 1 + 10;
      float rightTextX = leftTextX + textWidth;

      if (m_EndDragPos.x >= leftTextX || m_EndDragPos.x <= rightTextX) {
        float adjustedHeight = 0.8f * (m_Height - 2);
        float leftMostX = std::min(m_StartDragPos.x, m_EndDragPos.x) - leftTextX;
        float rightMostX = std::max(m_StartDragPos.x, m_EndDragPos.x) - leftTextX;

        float highlightWidth = 0.0f;
        float startPosX = leftTextX;
        float textPos = 0.0f;

        for (size_t i = 0; i < m_Text.length(); i++) {
          GlyphData glyph = m_Font->getGlyph(static_cast<uint32_t>(m_Text[i]));
          float glyphWidth = glyph.advanceX * 15.0f;

          if (glyphWidth == 0.0f) {
            continue;
          }

          if (std::max(0.0f, leftMostX - textPos) / glyphWidth >= 0.5f) {
            startPosX += glyphWidth;
          }
          else if (std::min(textWidth, rightMostX - textPos) / glyphWidth >= 0.5f) {
            highlightWidth += glyphWidth;
          }
          else {
            break;
          }

          textPos += glyphWidth;
        }

        if (highlightWidth > 0.0f) {
          renderer.drawRect({ startPosX, getAbsolutePosition().y + 1 + ((m_Height - 2) - adjustedHeight) / 2 },
            highlightWidth, adjustedHeight, { 0, 0, 255 });
          return;
        }
      }
    }

    // Text caret
    if (m_IsFocused) {
      if (m_CaretTimer >= m_CaretBlinkDelay && m_CaretTimer < m_CaretBlinkDelay * 2) {
        int adjustedHeight = int(0.8f * (m_Height - 2));
        float textPosX = 0.0f;

        for (size_t i = 0; i < std::min(m_CaretIndex, m_Text.length()); i++) {
          GlyphData glyph = m_Font->getGlyph(static_cast<uint32_t>(m_Text[i]));
          float glyphWidth = glyph.advanceX * 15.0f;

          textPosX += glyphWidth;
        }


        renderer.drawRect(getAbsolutePosition() + glm::vec2(1 + 10 + textPosX, 1 + ((m_Height - 2) - adjustedHeight) / 2),
          1, adjustedHeight, Color::White);
      }

      auto newTime = std::chrono::high_resolution_clock::now();
      m_CaretTimer += std::chrono::duration<float, std::chrono::seconds::period>(newTime - m_LastRenderTime).count();
      m_LastRenderTime = std::chrono::high_resolution_clock::now();

      if (m_CaretTimer >= m_CaretBlinkDelay * 2) {
        m_CaretTimer = 0.0f;
      }
    }
  }

  void UITextField::handleEvent(const UIEvent& event)
  {
    switch (event.getType()) {
    case UIEventType::MouseDown:
      {
        if (!m_IsFocused) {
          if (event.getMouseData().clickCount == 2) { // double click -> select all
            m_IsHighlighting = true;
            m_StartDragPos = getAbsolutePosition().x + glm::vec2(1 + 10, 0);
            m_EndDragPos = m_StartDragPos + m_Font->getTextWidth(m_Text, 15);
          }
          else {
            float textPosX = 0.0f;
            float leftTextX = getAbsolutePosition().x + 1 + 10;
            m_CaretIndex = 0;

            for (size_t i = 0; i < m_Text.length(); i++) {
              GlyphData glyph = m_Font->getGlyph(static_cast<uint32_t>(m_Text[i]));
              float glyphWidth = glyph.advanceX * 15.0f;

              if (glyphWidth == 0.0f) {
                continue;
              }

              if (std::max(0.0f, event.getMouseData().position.x - leftTextX - textPosX) / glyphWidth >= 0.5f) {
                m_CaretIndex++;
              }
              else {
                break;
              }

              textPosX += glyphWidth;
            }
          }

          m_IsFocused = true;
          m_DisplayBorderColor = m_BorderHoverColor;
          m_LastRenderTime = std::chrono::high_resolution_clock::now();
          m_CaretTimer = m_CaretBlinkDelay;
        }
      }
      break;
    case UIEventType::MouseDrag:
      {
        glm::vec2 mousePos = event.getMouseData().position;
        glm::vec2 relativePos = mousePos - (getAbsolutePosition() + glm::vec2(1 + 10, 1));

        if (!m_IsHighlighting) {
          m_IsHighlighting = true;
          m_StartDragPos = mousePos;
          m_EndDragPos = m_StartDragPos;
        }
        else {
          m_EndDragPos = mousePos;
        }
      }
      break;
    case UIEventType::FocusLost:
      {
        m_IsHighlighting = false;
        m_IsFocused = false;
        m_DisplayBorderColor = m_BorderColor;
      }
      break;
    case UIEventType::KeyboardDown:
      {
        if (m_Text.length() > 0) {
          switch (event.getKeyboardData().pressedKey) {
          case KeyCode::Backspace:
            m_Text.pop_back(); // remove last character
            break;
          case KeyCode::A:
            {
              KeyModifier modifier = event.getKeyboardData().modifier;
              if (modifier == KeyModifier::Control) { // Ctrl + A
                m_IsHighlighting = true;
                m_StartDragPos = getAbsolutePosition().x + glm::vec2(1 + 10, 0);
                m_EndDragPos = m_StartDragPos + m_Font->getTextWidth(m_Text, 15);
              }
            }
            break;
          case KeyCode::C: // copy to clipboard
            {

            }
          break;
          }
        }
      }
      break;
    case UIEventType::KeyboardChar:
      {
        m_Text += char(event.getCharData().codePoint);
        m_CaretTimer = m_CaretBlinkDelay;
        m_CaretIndex++;
      }
      break;
    }
  }

  Hitbox UITextField::hitboxTest(glm::vec2 position)
  {
    if (getHitbox().contains(position)) {
      return Hitbox(getAbsolutePosition(), m_Width, m_Height, this);
    }

    return Hitbox(getAbsolutePosition(), m_Width, m_Height, nullptr);
  }

  Hitbox UITextField::getHitbox()
  {
    return Hitbox(getAbsolutePosition(), m_Width, m_Height, nullptr);
  }

}
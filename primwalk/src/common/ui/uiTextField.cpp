#include "uiTextField.hpp"
#include "../input/keycode.hpp"
#include <iostream>

namespace pw {

	void UITextField::onRender(UIRenderSystem& renderer) {
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
			float leftTextX = getAbsolutePosition().x + 1 + 10;

			if (m_SelectionStart != m_SelectionEnd) {
			float adjustedHeight = 0.8f * (m_Height - 2);
			float highlightWidth = 0.0f;
			float highlightLeft = 0.0f;

			for (int i = 0; i < std::max(m_SelectionStart, m_SelectionEnd); i++) {
				GlyphData glyph = m_Font->getGlyph(static_cast<uint32_t>(m_Text[i]));

				if (i < std::min(m_SelectionStart, m_SelectionEnd)) {
				highlightLeft += glyph.advanceX * 15.0f;
				}
				else {
				highlightWidth += glyph.advanceX * 15.0f;
				}
			}

			if (highlightWidth > 0.0f) {
				renderer.drawRect({ highlightLeft + leftTextX, getAbsolutePosition().y + 1 + ((m_Height - 2) - adjustedHeight) / 2 },
				highlightWidth, adjustedHeight, { 0, 0, 255 });
			}
			}
		}
		// Text caret
		else if (m_IsFocused) {
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

			if (m_CaretTimer >= m_CaretBlinkDelay * 2) {
			m_CaretTimer = 0.0f;
			}
		}

		m_LastRenderTime = std::chrono::high_resolution_clock::now();
	}

	void UITextField::handleEvent(const UIEvent& event) {
		switch (event.getType()) {
		case UIEventType::MouseDown:
			{
				if (!m_IsFocused) {
					if (event.getMouseData().clickCount == 2) { // double click -> select all
						m_IsHighlighting = true;
						m_SelectionStart = 0;
						m_SelectionEnd = m_Text.length();
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

					// Compute selection start index
					float textPosX = 0.0f;
					float leftTextX = getAbsolutePosition().x + 1 + 10;
					m_SelectionStart= 0;

					for (size_t i = 0; i < m_Text.length(); i++) {
						GlyphData glyph = m_Font->getGlyph(static_cast<uint32_t>(m_Text[i]));
						float glyphWidth = glyph.advanceX * 15.0f;

						if (glyphWidth == 0.0f) {
							continue;
						}

						if (std::max(0.0f, mousePos.x - leftTextX - textPosX) / glyphWidth >= 0.5f) {
							m_SelectionStart++;
						}
						else {
							break;
						}

						textPosX += glyphWidth;
					}

					m_SelectionEnd = m_SelectionStart;
				}
				else {
					// Compute selection end index
					float textPosX = 0.0f;
					float leftTextX = getAbsolutePosition().x + 1 + 10;
					m_SelectionEnd = 0;

					for (size_t i = 0; i < m_Text.length(); i++) {
						GlyphData glyph = m_Font->getGlyph(static_cast<uint32_t>(m_Text[i]));
						float glyphWidth = glyph.advanceX * 15.0f;

						if (glyphWidth == 0.0f) {
							continue;
						}

						if (std::max(0.0f, event.getMouseData().position.x - leftTextX - textPosX) / glyphWidth >= 0.5f) {
							m_SelectionEnd++;
						}
						else {
							break;
						}

						textPosX += glyphWidth;
					}
				}
			}
			break;
		case UIEventType::FocusLost:
			{
				m_IsHighlighting = false;
				m_IsFocused = false;
				m_SelectionStart = 0;
				m_SelectionEnd = 0;
				m_CaretIndex = m_Text.length();
				m_DisplayBorderColor = m_BorderColor;
				std::cout << "focus lost" << std::endl;
			}
			break;
		case UIEventType::KeyboardDown:
			{
				if (m_Text.length() > 0) {
					switch (event.getKeyboardData().pressedKey) {
					case KeyCode::KeyboardButtonBackspace:
						{
							m_IsHighlighting = false;

							// 1. If no text is selected, then simply remove last char of string
							if (std::abs(m_SelectionEnd - m_SelectionStart) == 0 && m_CaretIndex > 0) {
								if (m_CaretIndex == m_Text.length()) {
									m_Text.pop_back(); // remove last character
								}
								else {
									m_Text.erase(m_CaretIndex - 1, 1);
								}

								m_CaretIndex--;
								return;
							}

							// 2. Remove text selection
							m_CaretIndex = (size_t)std::min(m_SelectionStart, m_SelectionEnd);
							m_Text.erase(m_CaretIndex, (size_t)std::abs(m_SelectionEnd - m_SelectionStart));
							m_SelectionStart = 0;
							m_SelectionEnd = 0;
						}
						break;
					case KeyCode::KeyboardButtonA:
						{
							KeyModifier modifier = event.getKeyboardData().modifier;
							if (modifier == KeyModifier::Control) { // Ctrl + A
								m_IsHighlighting = true;
								m_SelectionStart = 0;
								m_SelectionEnd = m_Text.length();
							}
						}
						break;
					case KeyCode::KeyboardButtonLeft:
						{
							KeyModifier modifier = event.getKeyboardData().modifier;
							if (modifier == KeyModifier::None && m_CaretIndex > 0) {
								m_CaretIndex--;
								m_CaretTimer = m_CaretBlinkDelay;
								m_IsHighlighting = false;
								m_SelectionStart = 0;
								m_SelectionEnd = 0;
							}
							else if (modifier == KeyModifier::Shift && m_CaretIndex > 0) { // Shift + Left
								m_CaretTimer = m_CaretBlinkDelay;

								if (!m_IsHighlighting) {
									size_t tempIndex = m_CaretIndex;
									m_CaretTimer = m_CaretBlinkDelay;
									m_SelectionStart = tempIndex;
									m_SelectionEnd = m_CaretIndex - 1;
									m_IsHighlighting = true;
								}
								else if (std::min(m_SelectionStart, m_SelectionEnd) > 0) {
									int selectDir = m_SelectionEnd - m_SelectionStart;

									if (selectDir != 0) {
										m_SelectionEnd--;

										if (m_SelectionEnd == m_SelectionStart) {
											m_IsHighlighting = false;
											m_SelectionStart = 0;
											m_SelectionEnd = 0;
										}
									}
								}
							}
						}
						break;
					case KeyCode::KeyboardButtonRight:
						{
							KeyModifier modifier = event.getKeyboardData().modifier;
							if (modifier == KeyModifier::None && m_CaretIndex < m_Text.length() && !m_IsHighlighting) {
								m_CaretIndex++;
								m_CaretTimer = m_CaretBlinkDelay;
								m_IsHighlighting = false;
								m_SelectionStart = 0;
								m_SelectionEnd = 0;
							}
							else if (modifier == KeyModifier::Shift && m_SelectionEnd < m_Text.length()) { // Shift + Right
								if (!m_IsHighlighting) {
									size_t tempIndex = m_CaretIndex;
									m_CaretTimer = m_CaretBlinkDelay;
									m_SelectionStart = tempIndex;
									m_SelectionEnd = m_CaretIndex + 1;
									m_IsHighlighting = true;
								}
								else {
									int selectDir = m_SelectionEnd - m_SelectionStart;

									if (selectDir != 0) {
										m_SelectionEnd++;
										m_CaretTimer = m_CaretBlinkDelay;

										if (m_SelectionEnd == m_SelectionStart) {
											m_IsHighlighting = false;
											m_SelectionStart = 0;
											m_SelectionEnd = 0;
										}
									}
								}
							}
						}
						break;
					}
				}
			}
			break;
		case UIEventType::KeyboardChar:
			{
				if (m_IsHighlighting) { // Remove highlighted text
					m_CaretIndex = (size_t)std::min(m_SelectionStart, m_SelectionEnd);
					m_Text.erase(m_CaretIndex, (size_t)std::abs(m_SelectionEnd - m_SelectionStart)); 
					m_IsHighlighting = false;
					m_SelectionStart = 0;
					m_SelectionEnd = 0;
				}

				// Insert character at caret index
				m_Text.insert(m_CaretIndex, { (char)event.getCharData().codePoint });
				m_CaretTimer = m_CaretBlinkDelay;
				m_CaretIndex++;
			}
			break;
		}
	}

	Hitbox UITextField::hitboxTest(glm::vec2 position) {
		if (Hitbox(getAbsolutePosition(), m_Width, m_Height, nullptr).contains(position)) {
			return Hitbox(getAbsolutePosition(), m_Width, m_Height, this);
		}

		return Hitbox(getAbsolutePosition(), m_Width, m_Height, nullptr);
	}

}
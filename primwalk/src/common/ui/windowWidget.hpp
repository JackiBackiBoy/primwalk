#pragma once

#include "widget.hpp"
#include "../color.hpp"
#include <algorithm>
#include <cstdint>
#include <optional>

namespace pw {
	class WindowWidget : public Widget {
	public:
		WindowWidget() = default;
		~WindowWidget() = default;

		void draw(UIRenderSystem& renderer) override;
		void processEvent(const UIEvent& event) override;
		bool hitTest(glm::vec2 mousePos) override;

		inline int getWidth() const { return m_Width; }
		inline int getHeight() const { return m_Height; }
		inline void setWidth(int width) { m_Width = std::max(width, minWidth); }
		inline void setHeight(int height) { m_Height = std::max(height, minHeight); }

		// TODO: replace booleans with flags
		bool resizeable = true;
		int minWidth = 50;
		int minHeight = 50;
		int borderPadding = 8;
		Color backgroundColor = { 52, 52, 52 };

	private:
		int m_Width = 100;
		int m_Height = 100;

		std::optional<glm::vec2> m_MouseDownOffset{};
		std::optional<float> m_MouseDownTopOffset{};
		std::optional<float> m_MouseDownRightOffset{};

		uint8_t m_BorderFlags = 0;
	};
}
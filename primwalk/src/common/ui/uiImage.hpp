#pragma once

// primwalk
#include "../../core.hpp"
#include "../color.hpp"
#include "../rendering/texture2D.hpp"
#include "uiElement.hpp"

// std
#include <memory>

// vendor
#include <glm/glm.hpp>

namespace pw {
	class PW_API UIImage: public UIElement {
	public:
		UIImage();
		virtual ~UIImage() {};

		virtual void onRender(UIRenderSystem& renderer) override;
		virtual void handleEvent(const UIEvent& event) override;
		virtual Hitbox hitboxTest(glm::vec2 position) override;

		// Getters
		inline int getWidth() const { return m_Width; }
		inline int getHeight() const { return m_Height; }

		// Setters
		inline void setWidth(int width) { m_Width = width; }
		inline void setHeight(int height) { m_Height = height; }
		inline void setImage(std::shared_ptr<Texture2D> image) { m_Image = image; }
		inline void setColor(Color color) { m_Color = color; }

	private:
		int m_Width = 0;
		int m_Height = 0;
		std::shared_ptr<Texture2D> m_Image = nullptr;
		Color m_Color = Color::White;
	};
}


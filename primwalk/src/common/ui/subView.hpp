#pragma once

// primwalk
#include "../../core.hpp"
#include "../color.hpp"
#include "../rendering/framebuffer.hpp"
#include "../rendering/frameInfo.hpp"
#include "../rendering/image.hpp"
#include "../rendering/renderpass.hpp"

// std
#include <memory>

// vendor
#include <glm/glm.hpp>

namespace pw {
	class PW_API SubView {
	public:
		SubView(int width, int height, glm::vec2 position);
		virtual ~SubView();

		void beginPass(VkCommandBuffer commandBuffer);
		virtual void onUpdate();
		virtual void onRender(const FrameInfo& frameInfo);
		void endPass(VkCommandBuffer commandBuffer);
		void resize(int width, int height);

		inline Image* getImage() const { return m_OffscreenImage.get(); }
		inline int getWidth() const { return m_Width; }
		inline int getHeight() const { return m_Height; }
		inline glm::vec2 getPosition() const { return m_Position; }

		inline void setPosition(glm::vec2 position) { m_Position = position; }
		inline void setBackgroundColor(Color color) { m_BackgroundColor = color; }

		std::unique_ptr<RenderPass> m_OffscreenPass;

	private:
		void createImages();
		void createRenderPass();
		void createFramebuffer();

		int m_Width;
		int m_Height;
		glm::vec2 m_Position;
		Color m_BackgroundColor = Color::Black;

		std::unique_ptr<Image> m_OffscreenImage;
		std::unique_ptr<Image> m_DepthImage;
		std::unique_ptr<Framebuffer> m_OffscreenFramebuffer;
	};
}

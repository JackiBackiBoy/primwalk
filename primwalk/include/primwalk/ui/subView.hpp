#pragma once

// primwalk
#include "primwalk/core.hpp"
#include "primwalk/rendering/image.hpp"
#include "primwalk/rendering/framebuffer.hpp"
#include "primwalk/rendering/frameInfo.hpp"
#include "primwalk/rendering/renderpass.hpp"

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

    inline Image* getImage() const { return m_OffscreenImage.get(); }
    inline int getWidth() const { return m_Width; }
    inline int getHeight() const { return m_Height; }
    inline glm::vec2 getPosition() const { return m_Position; }

    std::unique_ptr<RenderPass> m_OffscreenPass;

  private:
    void createFramebuffer();

    int m_Width;
    int m_Height;
    glm::vec2 m_Position;

    std::unique_ptr<Image> m_OffscreenImage;
    std::unique_ptr<Framebuffer> m_OffscreenFramebuffer;
  };
}
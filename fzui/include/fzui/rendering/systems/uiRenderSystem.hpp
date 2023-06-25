#ifndef FZ_UI_RENDER_SYSTEM_HEADER
#define FZ_UI_RENDER_SYSTEM_HEADER

// FZUI
#include "fzui/core.hpp"
#include "fzui/color.hpp"
#include "fzui/rendering/frameInfo.hpp"
#include "fzui/rendering/vertex.hpp"
#include "fzui/ui/uiEvent.hpp"

// std
#include <cstdint>
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>
#include <string>
#include <glm/glm.hpp>

namespace fz {
  struct FZ_API RenderParams {
    alignas(8) glm::vec2 position;
    alignas(8) glm::vec2 size;
    alignas(16) glm::vec4 color;
    alignas(4) uint32_t texIndex;
    alignas(4) uint32_t borderRadius;
  };

  struct FZ_API FontRenderParams {
    alignas(8) glm::vec2 position;
    alignas(8) glm::vec2 size;
    alignas(16) glm::vec4 color;
    alignas(4) uint32_t texIndex;
    alignas(8) glm::vec2 texCoords[4];
  };

  // Forward declarations
  class FZ_API GraphicsDevice_Vulkan;
  class FZ_API GraphicsPipeline;
  class FZ_API DescriptorPool;
  class FZ_API DescriptorSetLayout;
  class FZ_API Font;
  class FZ_API Texture2D;
  class FZ_API Buffer;
  class FZ_API UIElement;

  class FZ_API UIRenderSystem {
    public:
      UIRenderSystem(GraphicsDevice_Vulkan& device, VkRenderPass renderPass, std::vector<VkDescriptorSetLayout>& setLayouts);
      ~UIRenderSystem();

      // Events
      void processEvent(const UIEvent& event);
      void onUpdate(const FrameInfo& frameInfo);
      void onRender(const FrameInfo& frameInfo);
      void submitElement(std::unique_ptr<UIElement> element);

      void drawRect(glm::vec2 position, int width, int height, Color color, uint32_t borderRadius = 0, std::shared_ptr<Texture2D> texture = nullptr);
      void drawText(glm::vec2 position, const std::string& text, double fontSize, Color color, std::shared_ptr<Font> font = nullptr);

    private:
      void createDescriptorSetLayout();
      void createPipelineLayout(std::vector<VkDescriptorSetLayout>& setLayouts);
      void createPipeline(VkRenderPass renderPass);
      void createVertexBuffer();
      void createIndexBuffer();
      void createUniformBuffers();
      void createDescriptorPool();

      std::vector<RenderParams> m_RenderParams;
      std::vector<FontRenderParams> m_FontRenderParams;

      const std::vector<Vertex> m_Vertices = {
        {{0.0f, 0.0f}, { 0.0f, 0.0f }, 0 },
        {{1.0f, 0.0f}, { 1.0f, 0.0f }, 1 },
        {{1.0f, 1.0f}, { 1.0f, 1.0f }, 2 },
        {{0.0f, 1.0f}, { 0.0f, 1.0f }, 3 }
      };

      const std::vector<uint16_t> m_Indices = {
        0, 1, 2, 2, 3, 0,
      };

      GraphicsDevice_Vulkan& m_Device;

      std::unique_ptr<GraphicsPipeline> m_BasePipeline;
      std::unique_ptr<GraphicsPipeline> m_FontPipeline;
      VkPipelineLayout m_BasePipelineLayout = VK_NULL_HANDLE;
      VkPipelineLayout m_FontPipelineLayout = VK_NULL_HANDLE;

      std::vector<VkDescriptorSetLayout> m_BaseDescriptorSetLayouts;
      std::vector<VkDescriptorSetLayout> m_FontDescriptorSetLayouts;
      std::unique_ptr<DescriptorSetLayout> uniformSetLayout{};
      std::unique_ptr<DescriptorSetLayout> storageSetLayout{};
      std::unique_ptr<DescriptorSetLayout> textureSetLayout{};
      std::unique_ptr<DescriptorPool> m_DescriptorPool{};
      std::unique_ptr<DescriptorPool> m_BindlessDescriptorPool{};

      std::vector<std::shared_ptr<Texture2D>> m_Textures;
      std::vector<std::shared_ptr<Font>> m_Fonts;
      std::unordered_map<Texture2D*, uint32_t> m_TextureIDs;

      std::unique_ptr<Buffer> m_VertexBuffer;
      std::unique_ptr<Buffer> m_IndexBuffer;
      std::vector<std::unique_ptr<Buffer>> m_UniformBuffers;
      std::vector<std::unique_ptr<Buffer>> m_StorageBuffers;
      std::vector<std::unique_ptr<Buffer>> m_FontStorageBuffers;
      std::vector<VkDescriptorSet> m_UniformDescriptorSets;
      std::vector<VkDescriptorSet> m_StorageDescriptorSets;
      std::vector<VkDescriptorSet> m_FontStorageDescriptorSets;
      VkDescriptorSet m_TextureDescriptorSet = VK_NULL_HANDLE;

      std::map<int, std::unique_ptr<UIElement>> m_Elements;
      int m_ElementCount = 0;
      UIElement* m_TargetElement = nullptr;
  };
}
#endif
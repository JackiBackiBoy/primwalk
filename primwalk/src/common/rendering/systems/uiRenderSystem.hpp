#pragma once

// primwalk
#include "../../../core.hpp"
#include "../../color.hpp"
#include "../../math/hitbox.hpp"
#include "../frameInfo.hpp"
#include "../vertex.hpp"
#include "../sampler.hpp"

// std
#include <cstdint>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>

namespace pw {
	struct PW_API RenderParams {
		alignas(8) glm::vec2 position;
		alignas(8) glm::vec2 size;
		alignas(16) glm::vec4 color;
		alignas(4) uint32_t texIndex;
		alignas(4) uint32_t borderRadius;
		alignas(8) glm::vec2 texCoords[4];
	};

	struct PW_API FontRenderParams {
		alignas(8) glm::vec2 position;
		alignas(8) glm::vec2 size;
		alignas(16) glm::vec4 color;
		alignas(4) uint32_t texIndex;
		alignas(8) glm::vec2 texCoords[4];
	};

	// Forward declarations
	class GraphicsDevice_Vulkan;
	class GraphicsPipeline;
	class DescriptorPool;
	class DescriptorSetLayout;
	class Font;
	class Texture2D;
	class Buffer;
	class UIElement;

	class PW_API UIRenderSystem {
	public:
		UIRenderSystem(GraphicsDevice_Vulkan& device, VkRenderPass renderPass);
		~UIRenderSystem();

		// Events
		void onUpdate(const FrameInfo& frameInfo);
		void onRender(const FrameInfo& frameInfo);
		void removeImage(Image* image);

		void drawRect(glm::vec2 position,float width, float height,
			Color color, int borderRadius = 0, std::shared_ptr<Texture2D> texture = nullptr,
		glm::vec2 scissorPos = { 0, 0 }, int scissorWidth = 1, int scissorHeight = 1);
		void drawText(glm::vec2 position, const std::string& text, double fontSize, Color color, std::shared_ptr<Font> font = nullptr);
		void drawTextCentered(glm::vec2 position, const std::string& text, Color color, double fontSize = 0, std::shared_ptr<Font> font = nullptr);

		// Draw a framebuffer image, default width = 0, height = 0 means the image will be drawn at its original resolution
		void drawFramebuffer(Image* image, glm::vec2 position, int width = 0, int height = 0);

	private:
		struct UniformBufferObject {
			alignas(16) glm::mat4 proj;
		};

		void createDescriptorPool();
		void createUniformBuffers();
		void createDescriptorSetLayout();
		void createPipelineLayout();
		void createPipeline(VkRenderPass renderPass);
		void createVertexBuffer();
		void createIndexBuffer();
		void createSamplers();

		uint32_t addTexture(Image* image);

		// TODO: It will not remove texture from m_Textures if the related Vulkan image was bounded through Texture2D object
		void freeTextureID(Image* image);
		std::unordered_map<Image*, uint32_t> m_TextureIDs{};
		std::set<uint32_t> m_VacantTextureIDs{};

		std::vector<RenderParams> m_RenderParams;
		std::vector<FontRenderParams> m_FontRenderParams;

		GraphicsDevice_Vulkan& m_Device;

		std::unique_ptr<GraphicsPipeline> m_BasePipeline;
		std::unique_ptr<GraphicsPipeline> m_FontPipeline;
		VkPipelineLayout m_BasePipelineLayout = VK_NULL_HANDLE;
		VkPipelineLayout m_FontPipelineLayout = VK_NULL_HANDLE;

		std::vector<VkDescriptorSetLayout> m_BaseDescriptorSetLayouts;
		std::vector<VkDescriptorSetLayout> m_FontDescriptorSetLayouts;
		std::unique_ptr<DescriptorSetLayout> uniformSetLayout{};
		std::unique_ptr<DescriptorSetLayout> storageSetLayout{};
		std::unique_ptr<DescriptorSetLayout> m_TextureSetLayout{};

		std::vector<std::shared_ptr<Texture2D>> m_Textures;
		std::vector<std::shared_ptr<Font>> m_Fonts;

		std::unique_ptr<Buffer> m_VertexBuffer;
		std::unique_ptr<Buffer> m_IndexBuffer;
		std::vector<std::unique_ptr<Buffer>> m_UBOs;
		std::vector<std::unique_ptr<Buffer>> m_StorageBuffers;
		std::vector<std::unique_ptr<Buffer>> m_FontStorageBuffers;
		std::vector<VkDescriptorSet> m_UniformDescriptorSets;
		std::vector<VkDescriptorSet> m_StorageDescriptorSets;
		std::vector<VkDescriptorSet> m_FontStorageDescriptorSets;
		VkDescriptorSet m_TextureDescriptorSet = VK_NULL_HANDLE;
		std::unique_ptr<Sampler> m_Sampler;

		const std::vector<Vertex> m_Vertices = {
			{ { 0.0f, 0.0f }, { 0.0f, 0.0f }, 0 },
			{ { 1.0f, 0.0f }, { 1.0f, 0.0f }, 1 },
			{ { 1.0f, 1.0f }, { 1.0f, 1.0f }, 2 },
			{ { 0.0f, 1.0f }, { 0.0f, 1.0f }, 3 }
		};

		const std::vector<uint16_t> m_Indices = {
			0, 1, 2, 2, 3, 0,
		};
	};
}


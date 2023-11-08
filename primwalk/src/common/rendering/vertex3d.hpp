#pragma once

// primwalk
#include "../../core.hpp"

// std
#include <vector>

// vendor
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

namespace pw {
	struct PW_API Vertex3D {
		alignas(16) glm::vec3 position;
		alignas(16) glm::vec3 normal;
		alignas(16) glm::vec3 tangent;
		alignas(16) glm::vec3 bitangent;
		alignas(8) glm::vec2 texCoord;

		static std::vector<VkVertexInputBindingDescription> getBindingDescriptions() {
			std::vector<VkVertexInputBindingDescription> bindingDescriptions{};

			bindingDescriptions.push_back({ 0, sizeof(Vertex3D), VK_VERTEX_INPUT_RATE_VERTEX });

			return bindingDescriptions;
		}

		static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
			std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

			attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex3D, position) });
			attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex3D, normal) });
			attributeDescriptions.push_back({ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex3D, tangent) });
			attributeDescriptions.push_back({ 3, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex3D, bitangent) });
			attributeDescriptions.push_back({ 4, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex3D, texCoord) });

			return attributeDescriptions;
		}
	};
}
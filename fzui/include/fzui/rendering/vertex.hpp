#ifndef FZ_VERTEX_HEADER
#define FZ_VERTEX_HEADER

// FZUI
#include "fzui/core.hpp"

#ifdef FZ_WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

// vendor
#include <glm/glm.hpp>
#include <vector>
#include <vulkan/vulkan.h>

namespace fz {
  struct FZ_API Vertex {
    alignas(8) glm::vec2 pos;
    alignas(8) glm::vec2 texCoord;

    static std::vector<VkVertexInputBindingDescription> getBindingDescriptions() {
      std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
      bindingDescriptions[0].binding = 0;
      bindingDescriptions[0].stride = sizeof(Vertex);
      bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

      return bindingDescriptions;
    }

    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
      std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);

      attributeDescriptions[0].binding = 0;
      attributeDescriptions[0].location = 0;
      attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
      attributeDescriptions[0].offset = offsetof(Vertex, pos);

      attributeDescriptions[1].binding = 0;
      attributeDescriptions[1].location = 1;
      attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
      attributeDescriptions[1].offset = offsetof(Vertex, texCoord);

      return attributeDescriptions;
    }
  };
}
#endif
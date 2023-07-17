#ifndef PW_VERTEX_HEADER
#define PW_VERTEX_HEADER

// primwalk
#include "primwalk/core.hpp"

#ifdef PW_WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#endif

// vendor
#include <glm/glm.hpp>
#include <vector>
#include <vulkan/vulkan.h>

namespace pw {
  struct PW_API Vertex {
    alignas(8) glm::vec2 pos;
    alignas(8) glm::vec2 texCoord;
    alignas(4) uint32_t texCoordIndex;

    static std::vector<VkVertexInputBindingDescription> getBindingDescriptions() {
      std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
      bindingDescriptions[0].binding = 0;
      bindingDescriptions[0].stride = sizeof(Vertex);
      bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

      return bindingDescriptions;
    }

    static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
      std::vector<VkVertexInputAttributeDescription> attributeDescriptions(3);

      attributeDescriptions[0].binding = 0;
      attributeDescriptions[0].location = 0;
      attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
      attributeDescriptions[0].offset = offsetof(Vertex, pos);

      attributeDescriptions[1].binding = 0;
      attributeDescriptions[1].location = 1;
      attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
      attributeDescriptions[1].offset = offsetof(Vertex, texCoord);

      attributeDescriptions[2].binding = 0;
      attributeDescriptions[2].location = 2;
      attributeDescriptions[2].format = VK_FORMAT_R32_UINT;
      attributeDescriptions[2].offset = offsetof(Vertex, texCoordIndex);

      return attributeDescriptions;
    }
  };
}
#endif
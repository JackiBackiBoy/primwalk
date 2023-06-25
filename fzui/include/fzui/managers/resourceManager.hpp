#ifndef FZ_RESOURCE_MANAGER_HEADER
#define FZ_RESOURCE_MANAGER_HEADER

// FZUI
#include "fzui/core.hpp"
#include "fzui/rendering/texture2D.hpp"
#include "fzui/data/font.hpp"

// std
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan.h>

namespace fz {
  class FZ_API ResourceManager {
  public:
    ResourceManager() {};
    ~ResourceManager();

    static ResourceManager& Get();
    std::shared_ptr<Texture2D> loadTexture(const std::string& path, VkFormat format = VK_FORMAT_R8G8B8A8_SRGB);
    std::shared_ptr<Font> loadFont(const std::string& path, double fontSize, FontWeight weight = FontWeight::Regular);

    std::shared_ptr<Font> findFont(const std::string& family, const FontWeight& weight, bool italic = false);

  private:
    std::unordered_map<std::string, std::weak_ptr<Texture2D>> m_Textures;
    std::vector<std::weak_ptr<Font>> m_Fonts;
  };
}
#endif
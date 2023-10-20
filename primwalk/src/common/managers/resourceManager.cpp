#include "primwalk/managers/resourceManager.hpp"

// std
#include <unordered_map>

namespace pw {

	ResourceManager::~ResourceManager() {
		int i = 0;
	}

	ResourceManager& ResourceManager::Get() {
		static std::unique_ptr<ResourceManager> instance = std::make_unique<ResourceManager>();
		return *instance;
	}

	std::shared_ptr<Texture2D> ResourceManager::loadTexture(const std::string& path, VkFormat format) {
		std::weak_ptr<Texture2D>& weakResource = m_Textures[path];
		std::shared_ptr<Texture2D> resource = weakResource.lock();

		if (!resource) {
			resource = std::make_shared<Texture2D>(path, 4, format);
			weakResource = resource;
		}

		return resource;
	}

	std::shared_ptr<Font> ResourceManager::loadFont(const std::string& path, double fontSize, FontWeight weight) {
		m_Fonts.push_back(std::weak_ptr<Font>());
		std::weak_ptr<Font>& weakResource = m_Fonts[m_Fonts.size() - 1];
		std::shared_ptr<Font> resource = weakResource.lock();

		if (!resource) {
			resource = std::make_shared<Font>(path, fontSize, weight);
			weakResource = resource;
		}

		return resource;
	}

	std::shared_ptr<Font> ResourceManager::findFont(const std::string& family, const FontWeight& weight, bool italic /*= false*/) {
		for (auto& f : m_Fonts) {
			if (auto lockedFont = f.lock()) { // obtain a shared_ptr from weak_ptr
				if (lockedFont->getFamily() == family && lockedFont->getWeight() == weight) { // TODO: Add italic check
					return lockedFont;
				}
			}
		}

		return nullptr;
	}

}
#include "sampler.hpp"

#include <stdexcept>

namespace pw {

	Sampler::Sampler(const SamplerCreateInfo& info, GraphicsDevice_Vulkan& device) : m_Device(device) {
		VkFilter samplerFilter = info.bilinearFiltering ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = samplerFilter;
		samplerInfo.minFilter = samplerFilter;
		samplerInfo.addressModeU = info.addressModeU;
		samplerInfo.addressModeV = info.addressModeV;
		samplerInfo.addressModeW = info.addressModeW;
		samplerInfo.borderColor = info.borderColor;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = info.compareEnable;
		samplerInfo.compareOp = info.compareOp;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR; // TODO: Look into using nearest filtering instead
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = info.maxLOD;

		if (info.anisotropicFiltering) {
			VkPhysicalDeviceProperties properties{};
			vkGetPhysicalDeviceProperties(m_Device.getPhysicalDevice(), &properties);

			samplerInfo.anisotropyEnable = properties.limits.maxSamplerAnisotropy > 0.0f;
			samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		}

		if (vkCreateSampler(m_Device.getDevice(), &samplerInfo, nullptr, &m_Sampler) != VK_SUCCESS) {
			throw std::runtime_error("VULKAN ERROR: Failed to create texture sampler!");
		}
	}

	Sampler::~Sampler() {
		vkDestroySampler(m_Device.getDevice(), m_Sampler, nullptr);
	}

}
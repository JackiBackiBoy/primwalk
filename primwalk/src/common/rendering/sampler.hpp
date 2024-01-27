#pragma once

#include "graphicsDevice_Vulkan.hpp"
#include <vulkan/vulkan.h>

namespace pw {
	struct SamplerCreateInfo {
		float maxLOD = 0.0f;
		bool bilinearFiltering = true;
		bool anisotropicFiltering = true;
		// TODO: Make API agnostic
		VkBorderColor borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
		VkBool32 compareEnable = VK_FALSE;
		VkCompareOp compareOp = VK_COMPARE_OP_ALWAYS;
		VkSamplerAddressMode addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		VkSamplerAddressMode addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		VkSamplerAddressMode addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	};

	class Sampler {
	public:
		Sampler(const SamplerCreateInfo& info, GraphicsDevice_Vulkan& device);
		~Sampler();

		inline VkSampler getVkSampler() const { return m_Sampler; }

	private:
		GraphicsDevice_Vulkan& m_Device;
		VkSampler m_Sampler = VK_NULL_HANDLE;
	};
}
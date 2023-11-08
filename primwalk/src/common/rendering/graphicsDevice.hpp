#pragma once

// primwalk
#include "../../core.hpp"

namespace pw {
	struct PW_API CommandList {
		void* internalState = nullptr;
	};

	class PW_API GraphicsDevice {
	public:
		GraphicsDevice() {};
		virtual ~GraphicsDevice() = default;

		virtual CommandList beginFrame() = 0;
		virtual void endFrame() = 0;
		virtual void waitForGPU() = 0;

	};

	// Global device instance helper
	inline GraphicsDevice*& GetDevice() {
		static GraphicsDevice* device = nullptr;
		return device;
	}
}


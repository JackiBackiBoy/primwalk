#pragma once

// primwalk
#include "primwalk/core.hpp"

namespace pw {
	class PW_API GraphicsDevice {
	public:
		GraphicsDevice() {};
		virtual ~GraphicsDevice() = default;
	};

	// Global device instance helper
	inline GraphicsDevice*& GetDevice() {
		static GraphicsDevice* device = nullptr;
		return device;
	}
}


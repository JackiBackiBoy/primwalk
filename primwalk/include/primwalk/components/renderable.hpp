#pragma once

// primwalk
#include "primwalk/core.hpp"
#include "primwalk/color.hpp"
#include "primwalk/data/model.hpp"

namespace pw {
	struct PW_API Renderable {
		Model* model = nullptr;
		Color color = Color::White;
	};
}
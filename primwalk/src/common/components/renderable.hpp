#pragma once

// primwalk
#include "../../core.hpp"
#include "../color.hpp"
#include "../data/model.hpp"

namespace pw {
	struct PW_API Renderable {
		Model* model = nullptr;
		Color color = Color::White;
	};
}
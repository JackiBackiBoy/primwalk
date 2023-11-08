#pragma once

// primwalk
#include "../../core.hpp"

// std
#include <bitset>
#include <cstdint>

namespace pw {
	typedef uint32_t entity_id;
	const entity_id MAX_ENTITIES = 4096;

	typedef uint8_t component_type;
	const component_type MAX_COMPONENTS = 32;

	typedef std::bitset<MAX_COMPONENTS> component_signature;
}
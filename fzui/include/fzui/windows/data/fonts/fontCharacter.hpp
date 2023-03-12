#pragma once

// FZUI
#include "fzui/core.hpp"

namespace fz {
  struct FZ_API FontCharacter {
	  int width = 0;
	  int height = 0;
	  int bearingX = 0;
	  int bearingY = 0;
	  unsigned int advanceOffset = 0;
  };

	struct FZ_API GlyphData {
		unsigned int width = 0;
		unsigned int height = 0;
		double bearingX = 0;
		double bearingY = 0;
		double advanceX = 0;
		float texLeftX = 0.0f;
		float texTopY = 0.0f;
		float texRightX = 0.0f;
		float texBottomY = 0.0f;
		double pl = 0.0f;
		double pb = 0.0f;
		double pr = 0.0f;
		double pt = 0.0f;
		double il = 0.0f;
		double ib = 0.0f;
		double ir = 0.0f;
		double it = 0.0f;
	};
}

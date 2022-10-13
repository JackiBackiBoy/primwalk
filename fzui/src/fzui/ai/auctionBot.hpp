#pragma once

#include <windows.h>
#include <stdbool.h>
#include "fzui/core/core.hpp"

void FZ_API startAuctionBot(HWND hForzaHandle, const bool active);
void FZ_API stopAuctionBot();

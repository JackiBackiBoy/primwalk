#pragma once

// std
#include <string>

#include <windows.h>
#include <time.h>
#include "fzui/core/core.hpp"
#include "fzui/color.hpp"

void FZ_API sendKey(unsigned int keyCode, unsigned int duration);
void FZ_API auctionSniper(void* data);
void FZ_API listAllWindows(HWND* target);
void FZ_API getColorFromDC(fz::Color* target, HDC hdc, const int x, const int y);
int FZ_API getDeltaTime(const clock_t t1, const clock_t t2);
std::wstring convertToWideString(const std::string& string);

static BOOL CALLBACK listWindowsCallback(HWND hWnd, LPARAM lParam);


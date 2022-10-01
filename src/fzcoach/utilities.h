#pragma once

#include <windows.h>
#include "color.h"

void sendKey(unsigned int keyCode, unsigned int duration);
void auctionSniper(void* data);
void startAuctionBot(HWND hWnd);
void stopAuctionBot(HWND hWnd);
void listAllWindows(HWND* target);
void getColorFromDC(Color* target, HDC hdc, const int x, const int y);
static BOOL CALLBACK listWindowsCallback(HWND hWnd, LPARAM lParam);


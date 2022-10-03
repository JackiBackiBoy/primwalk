#pragma once

#include <windows.h>
#include <time.h>
#include "color.h"

void sendKey(unsigned int keyCode, unsigned int duration);
void auctionSniper(void* data);
void listAllWindows(HWND* target);
void getColorFromDC(Color* target, HDC hdc, const int x, const int y);
int getDeltaTime(const clock_t t1, const clock_t t2);

static BOOL CALLBACK listWindowsCallback(HWND hWnd, LPARAM lParam);


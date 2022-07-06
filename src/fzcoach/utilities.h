#pragma once

#include <windows.h>

struct WindowInfo {
  char title[128];
  unsigned int id;
};

void sendKey(unsigned int keyCode, unsigned int duration);
void auctionSniper(void* data);
void startAuctionBot(HWND hWnd);
void stopAuctionBot(HWND hWnd);
void findActiveWindows();
void listAllWindows(HWND* target);

static BOOL CALLBACK listWindowCallback(HWND hWnd, LPARAM lParam);
static BOOL CALLBACK countWindowsCallback(HWND hWnd, LPARAM lParam);


#pragma once

#include <windows.h>

struct Vector2 {
  float x;
  float y;
};

struct WindowInfo {
  struct Vector2 pos;
};

void setGraphicsParent(HWND parent);
void createGraphicsWindow(void* instance);
void onGraphicsStart();
void onGraphicsUpdate();
LRESULT CALLBACK GraphicsProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

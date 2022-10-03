#pragma once

#include <stdio.h>
#include <windows.h>

#define SMALL_FONT_SIZE 17
#define MEDIUM_FONT_SIZE 24
#define LARGE_FONT_SIZE 32
#define SIDE_MARGIN_WIDTH 30

// Returns 1 if succesful and 0 otherwise
int createWin32Label(HWND window, HWND* targetLabel, const wchar_t* text,
    const int x, const int y, HFONT font);
int createWin32Button(HWND window, HWND* targetButton, const wchar_t* text,
    const int x, const int y, const int width, const int height, HFONT font);
int createWin32CheckBox(HWND window, HWND* targetCheckBox, const wchar_t* text,
    const int x, const int y, HFONT font, const INT_PTR id);
int createWin32HorzLine(HWND window, HWND* targetLine,
    const int x, const int y, HFONT font);
int createWin32SectionTitle(HWND window, HWND* targetTitle, const wchar_t* text,
    const int x, const int y, HFONT font);

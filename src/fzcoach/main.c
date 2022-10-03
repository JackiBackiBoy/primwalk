#define WINVER 0x0A00
#define _WIN32_WINNT  0x0A00

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <conio.h>
#include <process.h>
#include <sdkddkver.h>

#include <windows.h>
#include <commctrl.h>
#include <dwmapi.h>
#include "resource.h"
#include "utilities.h"
#include <tchar.h>
#include <uxtheme.h>
#include <time.h>

#include "ui/win32ui.h"
#include "ai/auctionBot.h"
#include "color.h"

#define MAX_LOADSTRING 100

BOOL CALLBACK SetFont(HWND child, LPARAM font){
  SendMessage(child, WM_SETFONT, font, TRUE);
  return TRUE;
}

HMENU hMenu;
HFONT hSmallFont;
HFONT hMediumFont;
HFONT hLargeFont;

// Labels
HWND hAuctionBotTitle;

// Buttons
HWND hStartButton;
HWND hStopButton;
HWND hSearchWindowButton;
HWND hMatchSearch;
HWND hForzaHandle;

// Checkboxes
HWND hPreviewCheckBox;

// Text
HWND hWindowPreviewLabel;
HWND hTitleHorzLine;
HWND hAuctionBotSettings;
HWND hAuctionBotSettingsHorz;
HWND hAbOverdriveCheckBox;

// List Views
HWND hMatchList = NULL;
LVCOLUMN lvCol;
LVITEM lvItem;

HINSTANCE globalInstance;

void initUI(HWND hWnd);
HBRUSH whiteBrush;

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  LRESULT result = 0;

  if (message == WM_CREATE) {
    // Fonts
    hSmallFont = CreateFont(SMALL_FONT_SIZE, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
    hMediumFont = CreateFont(MEDIUM_FONT_SIZE, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");
    hLargeFont = CreateFont(LARGE_FONT_SIZE, 0, 0, 0, FW_BOLD, 0, 0, 0, 0, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI");

    initUI(hwnd);


    //EnumChildWindows(hwnd, (WNDENUMPROC)SetFont, (LPARAM)hWindowFont);

    RECT rcClient;
    GetWindowRect(hwnd, &rcClient);

    SetWindowPos(hwnd, 
                 NULL, 
                 rcClient.left, rcClient.top,
                 rcClient.right - rcClient.left, rcClient.bottom - rcClient.top,
                 SWP_FRAMECHANGED);

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);
  }
  else {
    bool wasHandled = false;

    switch (message) {
      case WM_CTLCOLORDLG:
      {
        wasHandled = true;
        result = (INT_PTR)whiteBrush;
        break;
      }
      case WM_CTLCOLORSTATIC:
      {
        wasHandled = true;
        result = 0;
        break;
      }
      case WM_CTLCOLORBTN:
      {
        HDC hdc = (HDC)wParam;
        POINT pt;
        HWND hwndCtl;

        SetBkMode(hdc,TRANSPARENT); // Ensure that "static" text doesn't use a solid fill

        pt.x = 0; pt.y = 0;
        MapWindowPoints(hwndCtl, hwnd, &pt, 1);
        SetBrushOrgEx(hdc, -pt.x, -pt.y, NULL);

        wasHandled = true;
        result = (INT_PTR)whiteBrush;
        break;
      }
      case WM_COMMAND:
      {
        switch(HIWORD(wParam)) {
          case BN_CLICKED:
          {
            if ((HWND)lParam == hStartButton) {
              startAuctionBot(hForzaHandle, true);
            }
            else if((HWND)lParam == hStopButton) {
              stopAuctionBot();
            }
            else if ((HWND)lParam == hPreviewCheckBox) {
              BOOL checked = IsDlgButtonChecked(hwnd, IDC_LIVE_PREVIEW);

              if (checked) {
                CheckDlgButton(hwnd, IDC_LIVE_PREVIEW, BST_UNCHECKED);
              }
              else {
                CheckDlgButton(hwnd, IDC_LIVE_PREVIEW, BST_CHECKED);
              }
            }
            break;
          }
        }

        break;
      }
      case WM_DESTROY:
      {
        PostQuitMessage(0);

        wasHandled = true;
        result = 1;
        break;
      }
    }

    if (!wasHandled) {
      result = DefWindowProc(hwnd, message, wParam, lParam);
    }
  }

  return result;
}

HWND graphicsWindow = NULL;

// Main thread (window thread)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  INITCOMMONCONTROLSEX icc;

  // Initialise common controls.
  icc.dwSize = sizeof(icc);
  icc.dwICC = ICC_WIN95_CLASSES;
  InitCommonControlsEx(&icc);

  // Create main window on main thread
  hForzaHandle = FindWindow(NULL, L"Forza Horizon 5");

  if (hForzaHandle != NULL) {
    SetForegroundWindow(hForzaHandle);
  }
  
  HDC windowDC = GetWindowDC(hForzaHandle);
  HDC mem = CreateCompatibleDC(windowDC);

  int windowWidth = 1920;
  int windowHeight = 1080;

  HBITMAP windowImage = CreateCompatibleBitmap(windowDC, windowWidth, windowHeight);
  HBITMAP oldBitmap = SelectObject(mem, windowImage);
  BitBlt(mem, 0, 0, windowWidth, windowHeight, windowDC, 0, 0, SRCCOPY);

  whiteBrush = CreateSolidBrush(RGB(255, 255, 255));
  globalInstance = hInstance;

  TCHAR title[MAX_LOADSTRING];
  HCURSOR cursor = LoadCursor(NULL, IDC_ARROW);
  HICON icon, iconSmall;

  LoadString(hInstance, IDS_APP_TITLE, title, sizeof(TCHAR) * MAX_LOADSTRING);
  icon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON_SMALL));
  iconSmall = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP_ICON_SMALL));

  // Setup window class attributes.
  WNDCLASSEX wcex;
  ZeroMemory(&wcex, sizeof(wcex));

  wcex.cbSize = sizeof(wcex); // WNDCLASSEX size in bytes
  wcex.style = CS_HREDRAW | CS_VREDRAW;   // Window class styles
  wcex.lpszClassName = title; // Window class name
  wcex.hbrBackground = CreateSolidBrush(RGB(255, 255, 255));  // Window background brush color.
  wcex.hCursor = cursor;    // Window cursor
  wcex.lpfnWndProc = WindowProc;    // Window procedure associated to this window class.
  wcex.hInstance = hInstance; // The application instance.
  wcex.hIcon = icon;      // Application icon.
  wcex.hIconSm = iconSmall; // Application small icon.

  // Register window and ensure registration success.
  if (!RegisterClassEx(&wcex)) {
    return 1;
  }

  // Setup window initialization attributes.
  CREATESTRUCT cs;
  ZeroMemory(&cs, sizeof(cs));

  cs.x = CW_USEDEFAULT; // Window X position
  cs.y = CW_USEDEFAULT; // Window Y position
  cs.cx = 840;  // Window width
  cs.cy = 680;  // Window height
  cs.hInstance = hInstance; // Window instance.
  cs.lpszClass = wcex.lpszClassName;    // Window class name
  cs.lpszName = title;  // Window title
  cs.style = WS_OVERLAPPEDWINDOW;   // Window style
  cs.dwExStyle = WS_EX_TOPMOST;

  // Create the window.
  HWND hWnd = CreateWindowEx(
    cs.dwExStyle,
    cs.lpszClass,
    cs.lpszName,
    cs.style,
    cs.x,
    cs.y,
    cs.cx,
    cs.cy,
    cs.hwndParent,
    cs.hMenu,
    cs.hInstance,
    cs.lpCreateParams);

  // Validate window.
  if (!hWnd) { return 1; }

  MSG msg;
  while(GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  UnregisterClass(wcex.lpszClassName, hInstance);

  return (int)msg.wParam;
}

void initUI(HWND hWnd) {
  // Auction bot title header
  createWin32Label(hWnd, &hAuctionBotTitle, L"Forza Coach - Auction Bot", 20, 10, hLargeFont);
  createWin32HorzLine(hWnd, &hTitleHorzLine, 20, 45, hSmallFont);

  // Auction bot start/stop buttons
  createWin32Button(hWnd, &hStartButton, L"Start", 20, 60, 100, 50, hSmallFont);
  createWin32Button(hWnd, &hStopButton, L"Stop", 140, 60, 100, 50, hSmallFont);

  // Auction bot settings
  createWin32Label(hWnd, &hAuctionBotSettings, L"Bot Settings", 20, 140, hMediumFont);
  createWin32HorzLine(hWnd, &hAuctionBotSettingsHorz, 20, 170, hSmallFont);

  createWin32CheckBox(hWnd, &hPreviewCheckBox, L"Live Preview", 20, 180, hSmallFont, IDC_LIVE_PREVIEW);
  createWin32CheckBox(hWnd, &hAbOverdriveCheckBox, L"Overdrive Mode", 180, 180, hSmallFont, IDC_OVERDRIVE_MODE);
}

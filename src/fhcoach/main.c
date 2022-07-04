#include <sdkddkver.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <process.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include "resource.h"
#include "utilities.h"
#include <tchar.h>
#include <strsafe.h>

#define FILE_MENU_OPEN 1
#define FILE_MENU_SAVE 2
#define VIEW_MENU_DEVICES 3
#define VIEW_MENU_SCREEN_CAPTURE 4
#define MAX_LOADSTRING 100

BOOL CALLBACK SetFont(HWND child, LPARAM font){
  SendMessage(child, WM_SETFONT, font, TRUE);
  return TRUE;
}

void OnSize(HWND hwnd, UINT flag, int width, int height)
{
    // Handle resizing
}

HMENU hMenu;

// Buttons
HWND hStartButton;
HWND hStopButton;
HWND hSearchWindowButton;

// Text
HWND hWindowSearchTitle;

HBITMAP hAuctionSniperImage;
HINSTANCE globalInstance;

void addMenu(HWND hWnd);
void addButtons(HWND hWnd);
void addText(HWND hWnd);
void addImages(HWND hWnd);
HBRUSH whiteBrush;

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message) {
    case WM_CREATE:
      addMenu(hwnd);
      addButtons(hwnd);
      addText(hwnd);
      addImages(hwnd);

      EnumChildWindows(hwnd, (WNDENUMPROC)SetFont, (LPARAM)GetStockObject(DEFAULT_GUI_FONT));
      break;
    case WM_COMMAND:
      switch(HIWORD(wParam)) {
        case BN_CLICKED:
          if ((HWND)lParam == hStartButton) {
            startAuctionBot(hwnd);
          }
          else if((HWND)lParam == hStopButton) {
            stopAuctionBot(hwnd);
          }
          break;
      }

      if (wParam == VIEW_MENU_SCREEN_CAPTURE) {
        SendMessage(hwnd, WM_DESTROY, wParam, lParam);
      }
      break;
    case WM_GETMINMAXINFO:
      LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
      lpMMI->ptMinTrackSize.x = 300;
      lpMMI->ptMinTrackSize.y = 300;
      break;

    case WM_CTLCOLORSTATIC:
    {
      DWORD ctrlID = GetDlgCtrlID((HWND)lParam); //Window Control ID
      if (ctrlID == IDT_WINDOW_MATCHES_TITLE) // If desired control
      {
         HDC hdcStatic = (HDC)wParam;
         SetBkColor(hdcStatic, RGB(255, 255, 255));
         return (INT_PTR)whiteBrush;
      }
      break;
    }
    case WM_DESTROY:
      PostQuitMessage(0);
      break;
    default:
      return DefWindowProc(hwnd, message, wParam, lParam);
  }

  return 0;
}

// Main thread (window thread)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
  whiteBrush = CreateSolidBrush(RGB(255, 255, 255));
  globalInstance = hInstance;

  TCHAR title[MAX_LOADSTRING];
  HCURSOR cursor;
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
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);  // Window background brush color.
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
  cs.cx = 640;  // Window width
  cs.cy = 480;  // Window height
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
  if (!hWnd) {
    return 1;
  }


  ShowWindow(hWnd, SW_SHOWDEFAULT);
  UpdateWindow(hWnd);

  MSG msg;
  while(GetMessage(&msg, hWnd, 0, 0 ) > 0) {
    DispatchMessage(&msg);
  }

  UnregisterClass(wcex.lpszClassName, hInstance);

  return (int)msg.wParam;
}

void addMenu(HWND hWnd) {
  // File menu
  HMENU hFileMenu = CreateMenu();
  AppendMenu(hFileMenu, MF_STRING, (UINT_PTR)FILE_MENU_OPEN, L"Open");
  AppendMenu(hFileMenu, MF_STRING, (UINT_PTR)FILE_MENU_SAVE, L"Save");
  AppendMenu(hFileMenu, MF_SEPARATOR, (UINT_PTR)NULL, NULL); 

  // View menju
  HMENU hViewMenu = CreateMenu();
  AppendMenu(hViewMenu, MF_STRING, (UINT_PTR)VIEW_MENU_DEVICES, L"Devices");
  AppendMenu(hViewMenu, MF_STRING, (UINT_PTR)VIEW_MENU_SCREEN_CAPTURE, L"Screen Capture");

  // Main menu
  hMenu = CreateMenu();

  AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, L"&File");
  AppendMenu(hMenu, MF_STRING, (UINT_PTR)NULL, L"&Edit");
  AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hViewMenu, L"&View");
  AppendMenu(hMenu, MF_STRING, (UINT_PTR)NULL, L"&Help");

  SetMenu(hWnd, hMenu);
}

void addButtons(HWND hWnd) {
  // Start button
  hStartButton = CreateWindow(
    L"BUTTON",  // Predefined class; Unicode assumed 
    L"Start",      // Button text 
    WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
    10,         // x position 
    10,         // y position 
    100,        // Button width
    50,        // Button height
    hWnd,     // Parent window
    NULL,       // No menu.
    globalInstance, 
    NULL);      // Pointer not needed.

  // Stop button
  hStopButton = CreateWindow(
    L"BUTTON",  // Predefined class; Unicode assumed 
    L"Stop",      // Button text 
    WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
    120,         // x position 
    10,         // y position 
    100,        // Button width
    50,        // Button height
    hWnd,     // Parent window
    NULL,       // No menu.
    globalInstance, 
    NULL);      // Pointer not needed.

  // Search window button

  hSearchWindowButton = CreateWindow(
    L"BUTTON",  // Predefined class; Unicode assumed 
    L"Search",      // Button text 
    WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
    10,         // x position 
    70,         // y position 
    100,        // Button width
    50,        // Button height
    hWnd,     // Parent window
    NULL,       // No menu.
    globalInstance, 
    NULL);      // Pointer not needed.
}

void addText(HWND hWnd) {
  hWindowSearchTitle = CreateWindow(
    L"STATIC",  // Predefined class; Unicode assumed 
    L"Forza Horizon 5 window matches:",      // Button text 
    WS_VISIBLE | WS_CHILD | SS_CENTER,  // Styles 
    10,         // x position 
    140,         // y position 
    200,        // Button width
    20,        // Button height
    hWnd,     // Parent window
    (HMENU)IDT_WINDOW_MATCHES_TITLE,       // No menu.
    globalInstance, 
    NULL);      // Pointer not needed.
}

void addImages(HWND hWnd) {
}


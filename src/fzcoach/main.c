#include <sdkddkver.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <conio.h>
#include <process.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <commctrl.h>
#include <dwmapi.h>
#include "resource.h"
#include "utilities.h"
#include <tchar.h>
#include <uxtheme.h>

#include "rendering/renderer.h"

#define FILE_MENU_OPEN 1
#define FILE_MENU_SAVE 2
#define VIEW_MENU_DEVICES 3
#define VIEW_MENU_SCREEN_CAPTURE 4
#define MAX_LOADSTRING 100
#define NC_TOP_HEIGHT 30
#define NC_SIDE_WIDTH 5
#define NC_BOTTOM_HEIGHT 5

BOOL CALLBACK SetFont(HWND child, LPARAM font){
  SendMessage(child, WM_SETFONT, font, TRUE);
  return TRUE;
}

HMENU hMenu;

// Resources
HBITMAP hAppLogo;
HBITMAP hAppMin;
HBITMAP hAppMax;
HBITMAP hAppExit;

// Buttons
HWND hStartButton;
HWND hStopButton;
HWND hSearchWindowButton;
HWND hMatchSearch;
HWND hForzaHandle;

// Text
HWND hWindowSearchTitle;

// List Views
HWND hMatchList = NULL;
LVCOLUMN lvCol;
LVITEM lvItem;

HINSTANCE globalInstance;

void addButtons(HWND hWnd);
void addText(HWND hWnd);
LRESULT hitTest(HWND hWnd, WPARAM wParam, LPARAM lParam);
HBRUSH whiteBrush;

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  LRESULT result = 0;

  if (message == WM_CREATE) {
    addButtons(hwnd);
    addText(hwnd);

    EnumChildWindows(hwnd, (WNDENUMPROC)SetFont, (LPARAM)GetStockObject(DEFAULT_GUI_FONT));

    RECT rcClient;
    GetWindowRect(hwnd, &rcClient);

    SetWindowPos(hwnd, 
                 NULL, 
                 rcClient.left, rcClient.top,
                 rcClient.right - rcClient.left, rcClient.bottom - rcClient.top,
                 SWP_FRAMECHANGED);

    SetWindowTheme(hwnd, L"", L"");

    // Load resources
    hAppLogo = (HBITMAP)LoadImage(globalInstance, L"assets/fzcoach_logo.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    hAppMin  = (HBITMAP)LoadImage(globalInstance, L"assets/min.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE); 
    hAppMax  = (HBITMAP)LoadImage(globalInstance, L"assets/max.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE); 
    hAppExit = (HBITMAP)LoadImage(globalInstance, L"assets/exit.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE); 

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);
  }
  else {
    bool wasHandled = false;

    switch (message) {
      case WM_SYSCOMMAND:
      {
        if (wParam == SC_MOUSEMENU ||
            wParam == SC_KEYMENU || wParam == SC_NEXTWINDOW ||
            wParam == SC_PREVWINDOW || wParam == SC_MOVE ||
            wParam == SC_TASKLIST ||
            wParam == SC_MAXIMIZE || wParam == SC_SIZE) {
          return 0;
        }
        break;
      }
      case WM_NCCALCSIZE:
        if (wParam == TRUE) {
          LPNCCALCSIZE_PARAMS ncParams = (LPNCCALCSIZE_PARAMS) lParam;
            ncParams->rgrc[0].top += NC_TOP_HEIGHT;
            ncParams->rgrc[0].left += NC_SIDE_WIDTH;
            ncParams->rgrc[0].bottom -= NC_BOTTOM_HEIGHT;
            ncParams->rgrc[0].right -= NC_SIDE_WIDTH;
            return 0;

          wasHandled = true;
          result = 0;
          break;
        }
      case WM_NCHITTEST:
      {
        result = hitTest(hwnd, wParam, lParam);
        wasHandled = true;
        break;
      }
      case WM_NCACTIVATE:
        //RedrawWindow(hwnd, NULL, NULL, RDW_UPDATENOW);
        wasHandled = true;
        result = 0;
        break;
      case WM_NCPAINT:
      {
        RECT rect;
            GetWindowRect(hwnd, &rect);
            HRGN region = NULL;
            if (wParam == NULLREGION) {
                region = CreateRectRgn(rect.left, rect.top, rect.right, rect.bottom);
            } else {
                HRGN copy = CreateRectRgn(0, 0, 0, 0);
                if (CombineRgn(copy, (HRGN) wParam, NULL, RGN_COPY)) {
                    region = copy;
                } else {
                    DeleteObject(copy);
                }
            }
            HDC dc = GetDCEx(hwnd, region, DCX_WINDOW | DCX_CACHE | DCX_INTERSECTRGN | DCX_LOCKWINDOWUPDATE);
            HDC dcMem = CreateCompatibleDC(dc);

            if (!dc && region) {
                DeleteObject(region);
            }

            // Draw window borders
            HBRUSH ncBrush = CreateSolidBrush(RGB(18, 20, 25));
            HGDIOBJ old = SelectObject(dc, ncBrush);
            int width = rect.right - rect.left;
            int height = rect.bottom - rect.top;
            Rectangle(dc, 0, 0, width, height);
            SelectObject(dc, old);
            DeleteObject(ncBrush);

            // Draw window icon
            BITMAP bm;
            HBITMAP oldBm = (HBITMAP)SelectObject(dcMem, hAppLogo);
            GetObject(hAppLogo, sizeof(bm), &bm);
            BitBlt(
                dc,
                NC_SIDE_WIDTH + 10, NC_TOP_HEIGHT / 2 - bm.bmHeight / 2,
                bm.bmWidth, bm.bmHeight, dcMem, 0, 0, SRCCOPY);

            SelectObject(dcMem, oldBm);
            DeleteDC(dcMem);

            // Draw minimize, maximize and exit buttons
            int offsetX = 1;
            int offsetY = NC_TOP_HEIGHT / 2 - 6;

            dcMem = CreateCompatibleDC(dc);
            oldBm = (HBITMAP)SelectObject(dcMem, hAppExit);
            GetObject(hAppExit, sizeof(bm), &bm);
            offsetX += bm.bmWidth;
            BitBlt(
                dc,
                width - offsetX, offsetY,
                bm.bmWidth, bm.bmHeight, dcMem, 0, 0, SRCCOPY);

            SelectObject(dcMem, oldBm);
            DeleteDC(dcMem);


            dcMem = CreateCompatibleDC(dc);
            oldBm = (HBITMAP)SelectObject(dcMem, hAppMax);
            GetObject(hAppMax, sizeof(bm), &bm);
            offsetX += bm.bmWidth;
            BitBlt(
                dc,
                width - offsetX, offsetY,
                bm.bmWidth, bm.bmHeight, dcMem, 0, 0, SRCCOPY);

            SelectObject(dcMem, oldBm);
            DeleteDC(dcMem);

            dcMem = CreateCompatibleDC(dc);
            oldBm = (HBITMAP)SelectObject(dcMem, hAppMin);
            GetObject(hAppMin, sizeof(bm), &bm);
            offsetX += bm.bmWidth;
            BitBlt(
                dc,
                width - offsetX, offsetY,
                bm.bmWidth, bm.bmHeight, dcMem, 0, 0, SRCCOPY);

            SelectObject(dcMem, oldBm);
            DeleteDC(dcMem);

            ReleaseDC(hwnd, dc);
            return 0;

        wasHandled = true;
        result = 0;
        break;
      }
      case WM_COMMAND:
      {
        switch(HIWORD(wParam)) {
          case BN_CLICKED:
          {
            if ((HWND)lParam == hStartButton) {
              startAuctionBot(hForzaHandle);
            }
            else if((HWND)lParam == hStopButton) {
              stopAuctionBot(hwnd);
            }
            break;
          }
          case CBN_SELCHANGE: 
          {
            int ItemIndex = SendMessage((HWND) lParam, (UINT) CB_GETCURSEL, 
                (WPARAM) 0, (LPARAM) 0);
                TCHAR  ListItem[256];
                SendMessage((HWND) lParam, (UINT) CB_GETLBTEXT, 
                (WPARAM) ItemIndex, (LPARAM) ListItem);
            MessageBox(hwnd, (LPCWSTR) ListItem, TEXT("Item Selected"), MB_OK);                        
          }
        }

        if (wParam == VIEW_MENU_SCREEN_CAPTURE) {
          SendMessage(hwnd, WM_DESTROY, wParam, lParam);
        }

        wasHandled = true;
        result = 0;
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
  // Create main window on main thread
  hForzaHandle = FindWindow(NULL, L"Forza Horizon 5");
  SetForegroundWindow(hForzaHandle);
  
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
  wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;   // Window class styles
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
  if (!hWnd) {
    return 1;
  }

  // Create graphics window on separate thread
  setGraphicsParent(hWnd);
  //HANDLE graphicsThread = (HANDLE)_beginthread(createGraphicsWindow, 0, (void*)&hInstance);



  MSG msg;
  while(GetMessage(&msg, NULL, 0, 0 )) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  UnregisterClass(wcex.lpszClassName, hInstance);

  return (int)msg.wParam;
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
    300,        // Button width
    20,        // Button height
    hWnd,     // Parent window
    (HMENU)IDT_WINDOW_MATCHES_TITLE,       // No menu.
    globalInstance, 
    NULL);      // Pointer not needed.
}

LRESULT hitTest(HWND hWnd, WPARAM wParam, LPARAM lParam) {
    // Get the point coordinates for the hit test.
    POINT ptMouse = { LOWORD(lParam), HIWORD(lParam)};

    // Get the window rectangle.
    RECT rcWindow;
    GetWindowRect(hWnd, &rcWindow);

    // Get the frame rectangle, adjusted for the style without a caption.
    RECT rcFrame = { 0 };
    AdjustWindowRectEx(&rcFrame, WS_OVERLAPPEDWINDOW & ~WS_CAPTION, FALSE, 0);

    // Determine if the hit test is for resizing. Default middle (1,1).
    USHORT uRow = 1;
    USHORT uCol = 1;
    bool fOnResizeBorder = false;

    printf("Mouse Y: %ld\n", ptMouse.y);

    // Determine if the point is at the top or bottom of the window.
    if (ptMouse.y >= rcWindow.top && ptMouse.y < rcWindow.top + NC_TOP_HEIGHT)
    {
        fOnResizeBorder = (ptMouse.y == rcWindow.top);
        uRow = 0;
    }
    else if (ptMouse.y < rcWindow.bottom && ptMouse.y >= rcWindow.bottom - NC_BOTTOM_HEIGHT)
    {
        uRow = 2;
    }

    // Determine if the point is at the left or right of the window.
    if (ptMouse.x >= rcWindow.left && ptMouse.x < rcWindow.left + NC_SIDE_WIDTH)
    {
        uCol = 0; // left side
    }
    else if (ptMouse.x < rcWindow.right && ptMouse.x >= rcWindow.right - NC_SIDE_WIDTH)
    {
        uCol = 2; // right side
    }

    // Hit test (HTTOPLEFT, ... HTBOTTOMRIGHT)
    LRESULT hitTests[3][3] = 
    {
        { HTTOPLEFT,    fOnResizeBorder ? HTTOP : HTCAPTION,    HTTOPRIGHT },
        { HTLEFT,       HTNOWHERE,     HTRIGHT },
        { HTBOTTOMLEFT, HTBOTTOM, HTBOTTOMRIGHT },
    };

    return hitTests[uRow][uCol];
}


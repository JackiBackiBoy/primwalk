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
#include <time.h>

#include "rendering/renderer.h"
#include "color.h"

#define WINDOW_WIDTH 840
#define WINDOW_HEIGHT 680
#define FILE_MENU_OPEN 1
#define FILE_MENU_SAVE 2
#define VIEW_MENU_DEVICES 3
#define VIEW_MENU_SCREEN_CAPTURE 4
#define MAX_LOADSTRING 100
#define NC_TOP_HEIGHT 30
#define NC_SIDE_WIDTH 5
#define NC_BOTTOM_HEIGHT 5
#define NC_MINIMIZE_BUTTON_WIDTH 38
#define NC_MAXIMIZE_BUTTON_WIDTH 40
#define NC_EXIT_BUTTON_WIDTH 41

BOOL CALLBACK SetFont(HWND child, LPARAM font){
  SendMessage(child, WM_SETFONT, font, TRUE);
  return TRUE;
}

// Bot steps
int endStage = 1;
bool stopOptionsTimer = false;
clock_t clickOptionsStart;
clock_t placeBidStart;
clock_t endBotCycleStart;
clock_t confirmStartTime;
clock_t restartStartTime;
bool confirm = false;
bool listingExists = false;
bool placeBidExists = false;
bool endBotCycle = false;
bool botCycleDone = false;
bool restarting = false;
bool botActive = false;
int botPasses = 0b000000; // x   x       x       x        x       x
                          //    shutd   bid     list     conf    bot

HMENU hMenu;
BITMAP bm;

// Resources
HBITMAP hAppLogo;
HBITMAP hAppMin;
HBITMAP hAppMax;
HBITMAP hAppExit;
LRESULT m_HoverState = HTNOWHERE;

// Buttons
HWND hStartButton;
HWND hStopButton;
HWND hSearchWindowButton;
HWND hMatchSearch;
HWND hForzaHandle;

// Text
HWND hWindowPreviewLabel;

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
      case WM_PAINT:
      {
        if (hForzaHandle != NULL) {
          // Screen Capture
          HDC hScreenDC = GetDC(NULL); // CreateDC("DISPLAY",nullptr,nullptr,nullptr);
          HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
          int width = GetDeviceCaps(hScreenDC,HORZRES);
          int height = GetDeviceCaps(hScreenDC,VERTRES);

          // Only capture the game window (if available)
          RECT forzaWindowRect;
          RECT forzaClientRect;

          GetWindowRect(hForzaHandle, &forzaWindowRect);
          GetClientRect(hForzaHandle, &forzaClientRect);

          // Forza window metrics
          int forzaWidth = forzaClientRect.right - forzaClientRect.left;
          int forzaHeight = forzaClientRect.bottom - forzaClientRect.top;
          int forzaBorderThicknessX = GetSystemMetrics(SM_CXSIZEFRAME);
          int forzaBorderThicknessY = GetSystemMetrics(SM_CYSIZEFRAME);

          // Capture screen area
          HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC,width,height);
          HBITMAP hOldBitmap = (HBITMAP)(SelectObject(hMemoryDC,hBitmap));
          BitBlt(
              hMemoryDC,
              0,
              0,
              forzaClientRect.right - forzaClientRect.left,
              forzaClientRect.bottom - forzaClientRect.top,
              hScreenDC,
              forzaBorderThicknessX + forzaWindowRect.left,
              GetSystemMetrics(SM_CYCAPTION) + forzaBorderThicknessY + forzaWindowRect.top,
              SRCCOPY);

          // Bot alignment constans
          const int forzaCenterX = forzaWidth / 2;
          const int forzaCenterY = forzaHeight / 2;
          const int forzaConfirmY = (int)(0.672f * forzaHeight);
          const int forzaConfirmX = forzaCenterX + 100;
          const int forzaListingAreaX = (int)(0.389f * forzaWidth);
          const int forzaListingAreaY = (int)(0.2743f * forzaHeight);
          const int forzaPlaceBidX = (int)(0.330625f * forzaWidth);
          const int forzaPlaceBidY = (int)(0.31f * forzaHeight);
          const int forzaCollectCarX = (int)(0.611328125f * forzaWidth);
          const int forzaCollectCarY = (int)(0.497916667f * forzaHeight);
          const int forzaStartAuctionX = (int)(0.7953125f * forzaWidth);
          const int forzaStartAuctionY = (int)(0.3f * forzaHeight);

          // Bot target colors
          Color confirmColor;
          Color listingColor;
          Color placeBidColor;
          Color collectCarColor;
          Color startAuctionColor;

          // Retrieve colors from screen regions
          getColorFromDC(&confirmColor, hMemoryDC, forzaConfirmX, forzaConfirmY);
          getColorFromDC(&listingColor, hMemoryDC, forzaListingAreaX, forzaListingAreaY);
          getColorFromDC(&placeBidColor, hMemoryDC, forzaPlaceBidX, forzaPlaceBidY);
          getColorFromDC(&collectCarColor, hMemoryDC, forzaCollectCarX, forzaCollectCarY);
          getColorFromDC(&startAuctionColor, hMemoryDC, forzaStartAuctionX, forzaStartAuctionY);

          if (restarting && startAuctionColor.r == 52 && startAuctionColor.g == 23 && startAuctionColor.b == 53) {
            restarting = false;
            confirm = false;
            SendMessage(hForzaHandle, WM_KEYDOWN, VK_RETURN, 0);
          }

          // Bot cycle
          if (!restarting && !endBotCycle && botActive) {
            // Check the RGB-color of the confirm button (if available)
            if (!confirm) {
              SendMessage(hForzaHandle, WM_KEYDOWN, VK_RETURN, 0);
              confirmStartTime = clock();
              confirm = true;
            }
            else if (confirm && !listingExists && (clock() - confirmStartTime) * 1000 / CLOCKS_PER_SEC < 500) {
              SendMessage(hForzaHandle, WM_KEYDOWN, VK_RETURN, 0);
            }

            if (confirm && !listingExists && (clock() - confirmStartTime) * 1000 / CLOCKS_PER_SEC > 1250) {
              SendMessage(hForzaHandle, WM_KEYDOWN, VK_ESCAPE, 0);
              restartStartTime = clock();
              restarting = true;
            }

            if (!restarting) {
              if (confirm && !listingExists && listingColor.r == 247 && listingColor.g == 247 && listingColor.b == 247) {
                SendMessage(hForzaHandle, WM_KEYDOWN, 0x59, 0);

                listingExists = true;
              }
              else if (listingExists && !placeBidExists) {
                SendMessage(hForzaHandle, WM_KEYDOWN, 0x59, 0);
              }

              if (listingExists && !placeBidExists &&
                  placeBidColor.r == 171 && placeBidColor.g == 171 && placeBidColor.b == 171) {
                clickOptionsStart = clock();

                placeBidExists = true;
              }

              if (placeBidExists && !stopOptionsTimer && !endBotCycle) {
                clock_t diff = clock() - clickOptionsStart;
                int mSec = diff * 1000 / CLOCKS_PER_SEC;

                if (mSec >= 300) {
                  SendMessage(hForzaHandle, WM_KEYDOWN, VK_DOWN, 0);
                  SendMessage(hForzaHandle, WM_KEYUP, VK_DOWN, 0);
                  endBotCycleStart = clock();
                  stopOptionsTimer = true;
                }
              }

              if (stopOptionsTimer && collectCarColor.r == 255 && collectCarColor.g == 0 && collectCarColor.b == 134 && !endBotCycle &&
                  (clock() - endBotCycleStart) * 1000 / CLOCKS_PER_SEC >= 1000) {
                endBotCycleStart = clock();
                endBotCycle = true;
              }
              else if (stopOptionsTimer && !endBotCycle) {
                SendMessage(hForzaHandle, WM_KEYDOWN, VK_RETURN, 0);
              }
            }
          }
          else if (endBotCycle) {
            int mSec = (clock() - endBotCycleStart) * 1000 / CLOCKS_PER_SEC;

            if (mSec >= 5000 && endStage == 1) {
              SendMessage(hForzaHandle, WM_KEYDOWN, VK_RETURN, 0);
              endStage = 2;
            }
            else if (mSec >= 5500 && endStage == 2) {
              SendMessage(hForzaHandle, WM_KEYDOWN, VK_ESCAPE, 0);
              endStage = 3;
            }
            else if (mSec >= 6000 && endStage == 3) {
              SendMessage(hForzaHandle, WM_KEYDOWN, VK_ESCAPE, 0);
              endStage = 4;
            }

            if (mSec >= 7000 && endStage == 4) {
              // Bot cycle completed, reset bot flags
              if (startAuctionColor.r == 52 && startAuctionColor.g == 23 && startAuctionColor.b == 53) {
                endBotCycle = false;
                confirm = false;
                listingExists = false;
                placeBidExists = false;
                stopOptionsTimer = false;
                endStage = 1;
              }
              else {
                endBotCycleStart = clock();
                endStage = 1;
              }
            }
          }

          PAINTSTRUCT ps;
          GetObject(hBitmap, sizeof(BITMAP), &bm);

          HDC hdc = BeginPaint(hwnd, &ps);

          float aspectRatio = (float)bm.bmWidth / bm.bmHeight;
          SetStretchBltMode(hdc, STRETCH_HALFTONE);
          StretchBlt(hdc, 10, 170, (int)(360 * aspectRatio), 360,
              hMemoryDC,0,0, forzaClientRect.right - forzaClientRect.left,
              forzaClientRect.bottom - forzaClientRect.top, SRCCOPY);


          EndPaint(hwnd, &ps);
          DeleteDC(hMemoryDC);
          DeleteDC(hScreenDC);
          DeleteObject(hBitmap);
          DeleteObject(hOldBitmap);

          InvalidateRect(hwnd, NULL, FALSE);
        }

        wasHandled = true;
        result = 0;
        break;
      }
      case WM_GETMINMAXINFO:
      {
        LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
        lpMMI->ptMinTrackSize.x = NC_SIDE_WIDTH + 10 + 1 + 160 +
          NC_EXIT_BUTTON_WIDTH + NC_MAXIMIZE_BUTTON_WIDTH + NC_MINIMIZE_BUTTON_WIDTH;
        lpMMI->ptMinTrackSize.y = 50;

        wasHandled = true;
        result = 0;
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
        }
      case WM_NCHITTEST:
      {
        result = hitTest(hwnd, wParam, lParam);
        LRESULT lastResult = m_HoverState;

        // --- Redraw the non-client area when buttons are hovered/unhovered ---
        // Close button
        if ((result == HTCLOSE && lastResult != HTCLOSE) ||
            (result != HTCLOSE && lastResult == HTCLOSE)) {
          RECT wr;
          GetWindowRect(hwnd, &wr);
          int width = wr.right - wr.left;
          int height = wr.bottom - wr.top;

          m_HoverState = result;
          HRGN rgn = CreateRectRgn(wr.left + width - NC_EXIT_BUTTON_WIDTH, wr.top, wr.right, wr.top + NC_TOP_HEIGHT);
          SendMessage(hwnd, WM_NCPAINT, (WPARAM)rgn, 0);
        }
        // Maximize button
        if ((result == HTMAXBUTTON && lastResult != HTMAXBUTTON) ||
            (result != HTMAXBUTTON && lastResult == HTMAXBUTTON)) {
          RECT wr;
          GetWindowRect(hwnd, &wr);
          int width = wr.right - wr.left;
          int height = wr.bottom - wr.top;

          m_HoverState = result;
          HRGN rgn = CreateRectRgn(
              wr.left + width - NC_EXIT_BUTTON_WIDTH - NC_MAXIMIZE_BUTTON_WIDTH,
              wr.top,
              wr.right - NC_EXIT_BUTTON_WIDTH, wr.top + NC_TOP_HEIGHT);
          SendMessage(hwnd, WM_NCPAINT, (WPARAM)rgn, 0);
        }
        // Minimize button
        if ((result == HTMINBUTTON && lastResult != HTMINBUTTON) ||
            (result != HTMINBUTTON && lastResult == HTMINBUTTON)) {
          RECT wr;
          GetWindowRect(hwnd, &wr);
          int width = wr.right - wr.left;
          int height = wr.bottom - wr.top;

          m_HoverState = result;
          HRGN rgn = CreateRectRgn(
              wr.left + width - NC_EXIT_BUTTON_WIDTH - NC_MAXIMIZE_BUTTON_WIDTH - NC_MINIMIZE_BUTTON_WIDTH,
              wr.top,
              wr.right - NC_EXIT_BUTTON_WIDTH - NC_MAXIMIZE_BUTTON_WIDTH, wr.top + NC_TOP_HEIGHT);
          SendMessage(hwnd, WM_NCPAINT, (WPARAM)rgn, 0);
        }

        m_HoverState = result;
        wasHandled = true;
        break;
      }
      case WM_NCACTIVATE:
        wasHandled = true;
        result = 0;
        break;
      case WM_NCLBUTTONDOWN:
      {
        wasHandled = true;
        result = 0;

        if (wParam != HTMINBUTTON && wParam != HTMAXBUTTON && wParam != HTCLOSE) {
          wasHandled = false;
        }

        break;
      }
      case WM_NCLBUTTONUP:
      {
        wasHandled = true;
        result = 0;

        if (wParam == HTMINBUTTON) {
          ShowWindow(hwnd, SW_MINIMIZE);
        }
        else if (wParam == HTMAXBUTTON) {
          WINDOWPLACEMENT wp;
          GetWindowPlacement(hwnd, &wp);

          ShowWindow(hwnd, wp.showCmd == SW_MAXIMIZE ? SW_RESTORE : SW_MAXIMIZE);
        }
        else if (wParam == HTCLOSE) {
          SendMessage(hwnd, WM_DESTROY, 0, 0);
        }
        else {
          wasHandled = false;
        }

        break;
      }
      case WM_NCPAINT:
      {
        RECT rect;
        GetWindowRect(hwnd, &rect);
        HRGN region = NULL;

        if (wParam == NULLREGION) {
            region = CreateRectRgn(rect.left, rect.top, rect.right, rect.bottom);
        }
        else {
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
        TransparentBlt(
            dc,
            NC_SIDE_WIDTH + 10, NC_TOP_HEIGHT / 2 - bm.bmHeight / 2,
            bm.bmWidth, bm.bmHeight, dcMem, 0, 0, bm.bmWidth, bm.bmHeight, RGB(18, 20, 25));

        // Draw minimize, maximize and exit buttons
        int offsetX = 1;
        int offsetY = NC_TOP_HEIGHT / 2 - 6;

        if (m_HoverState == HTCLOSE) {
          HBRUSH hoverBrush = CreateSolidBrush(RGB(255, 0, 0));
          HGDIOBJ old = SelectObject(dc, hoverBrush);
          Rectangle(dc, width - NC_EXIT_BUTTON_WIDTH, 0, width, NC_TOP_HEIGHT);
          SelectObject(dc, old);
          DeleteObject(hoverBrush);
        }
        else if (m_HoverState == HTMAXBUTTON) {
          HBRUSH hoverBrush = CreateSolidBrush(RGB(70, 70, 70));
          HGDIOBJ old = SelectObject(dc, hoverBrush);
          Rectangle(
              dc,
              width - NC_EXIT_BUTTON_WIDTH - NC_MAXIMIZE_BUTTON_WIDTH,
              0,
              width - NC_EXIT_BUTTON_WIDTH,
              NC_TOP_HEIGHT);
          SelectObject(dc, old);
          DeleteObject(hoverBrush);
        }
        else if (m_HoverState == HTMINBUTTON) {
          HBRUSH hoverBrush = CreateSolidBrush(RGB(70, 70, 70));
          HGDIOBJ old = SelectObject(dc, hoverBrush);
          Rectangle(
              dc,
              width - NC_EXIT_BUTTON_WIDTH - NC_MAXIMIZE_BUTTON_WIDTH - NC_MINIMIZE_BUTTON_WIDTH,
              0,
              width - NC_EXIT_BUTTON_WIDTH - NC_MAXIMIZE_BUTTON_WIDTH,
              NC_TOP_HEIGHT);
          SelectObject(dc, old);
          DeleteObject(hoverBrush);
        }

        oldBm = (HBITMAP)SelectObject(dcMem, hAppExit);
        GetObject(hAppExit, sizeof(bm), &bm);
        offsetX += bm.bmWidth;
        TransparentBlt(
            dc,
            width - offsetX, offsetY,
            bm.bmWidth, bm.bmHeight, dcMem, 0, 0, bm.bmWidth, bm.bmHeight, RGB(0, 255, 0));

        oldBm = (HBITMAP)SelectObject(dcMem, hAppMax);
        GetObject(hAppMax, sizeof(bm), &bm);
        offsetX += bm.bmWidth;
        TransparentBlt(
            dc,
            width - offsetX, offsetY,
            bm.bmWidth, bm.bmHeight, dcMem, 0, 0, bm.bmWidth, bm.bmHeight, RGB(0, 255, 0));


        oldBm = (HBITMAP)SelectObject(dcMem, hAppMin);
        GetObject(hAppMin, sizeof(bm), &bm);
        offsetX += bm.bmWidth;
        TransparentBlt(
            dc,
            width - offsetX, offsetY,
            bm.bmWidth, bm.bmHeight, dcMem, 0, 0, bm.bmWidth, bm.bmHeight, RGB(0, 255, 0));

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
              botActive = true;
              botPasses = 0b000001; // right-most bit indicates bot is active
            }
            else if((HWND)lParam == hStopButton) {
              botActive = false;
              botPasses = 0b000000; // reset bot passes
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
  cs.style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;   // Window style
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

  // Live preview checkbox
}

void addText(HWND hWnd) {
  hWindowPreviewLabel = CreateWindow(
    L"STATIC",  // Predefined class; Unicode assumed 
    L"Window preview:",      // Button text 
    WS_VISIBLE | WS_CHILD | SS_CENTER,  // Styles 
    10,         // x position 
    140,         // y position 
    300,        // Button width
    20,        // Button height
    hWnd,     // Parent window
    (HMENU)IDT_WINDOW_PREVIEW_LABEL,       // No menu.
    globalInstance, 
    NULL);      // Pointer not needed.
}

LRESULT hitTest(HWND hWnd, WPARAM wParam, LPARAM lParam) {
    // Get the point coordinates for the hit test.
    POINT ptMouse = { LOWORD(lParam), HIWORD(lParam)};

    // Get the window rectangle.
    RECT rcWindow;
    GetWindowRect(hWnd, &rcWindow);

    // Determine if the hit test is for resizing. Default middle (1,1).
    USHORT uRow = 1;
    USHORT uCol = 1;
    bool fOnResizeBorder = false;

    // Check if cursor is in the top caption area
    if (ptMouse.y >= rcWindow.top && ptMouse.y < rcWindow.top + NC_TOP_HEIGHT)
    {
      fOnResizeBorder = (ptMouse.y == rcWindow.top);
      uRow = 0;
    }
    // Else the cursor must reside in the bottom area of the window
    else if (ptMouse.y < rcWindow.bottom && ptMouse.y >= rcWindow.bottom - NC_BOTTOM_HEIGHT)
    {
      uRow = 2;
    }

    // Determine if the point is at the left or right of the window.
    if (ptMouse.x >= rcWindow.left && ptMouse.x < rcWindow.left + NC_SIDE_WIDTH)
    {

      uCol = 0; // left side
    }
    else if (ptMouse.x < rcWindow.right)
    {
      if (ptMouse.x >= rcWindow.right - NC_SIDE_WIDTH) {
        uCol = 2; // right side
      }
      else if (uRow == 0 && fOnResizeBorder == false &&
               ptMouse.x >= rcWindow.right - 1 - NC_EXIT_BUTTON_WIDTH - NC_MAXIMIZE_BUTTON_WIDTH - NC_MINIMIZE_BUTTON_WIDTH) {
        // On minimize button
        if (ptMouse.x <= rcWindow.right - 1 - NC_EXIT_BUTTON_WIDTH - NC_MAXIMIZE_BUTTON_WIDTH) {
          return HTMINBUTTON;
        }
        else if (ptMouse.x <= rcWindow.right - 1 - NC_EXIT_BUTTON_WIDTH) {
          return HTMAXBUTTON;
        }
        else {
          return HTCLOSE;
        }
      }
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


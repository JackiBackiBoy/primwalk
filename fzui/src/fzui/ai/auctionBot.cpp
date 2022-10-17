#include "fzui/ai/auctionBot.hpp"
#include "fzui/utilities.hpp"
#include <commctrl.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <process.h>
#include <iostream>

static bool botActive = false;
static HANDLE m_BotThread;
static HWND m_ForzaHandle;

unsigned int __stdcall auctionBotThread(void* data) {
  int endStage = 1;
  bool stopOptionsTimer = false;
  clock_t clickOptionsStart;
  clock_t placeBidStart;
  clock_t endBotCycleStart;
  clock_t confirmStartTime;
  clock_t restartStartTime;
  bool endBotCycle = false;
  bool botCycleDone = false;
  bool restarting = false;
  int botStages = 0b000000;
  
  while (botActive && m_ForzaHandle != NULL) {
    // Screen Capture
    HDC hScreenDC = GetDC(NULL);
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
    int width = GetDeviceCaps(hScreenDC,HORZRES);
    int height = GetDeviceCaps(hScreenDC,VERTRES);

    // Only capture the game window (if available)
    RECT forzaWindowRect;
    RECT forzaClientRect;

    GetWindowRect(m_ForzaHandle, &forzaWindowRect);
    GetClientRect(m_ForzaHandle, &forzaClientRect);

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
    fz::Color confirmColor;
    fz::Color listingColor;
    fz::Color placeBidColor;
    fz::Color collectCarColor;
    fz::Color startAuctionColor;

    // Retrieve colors from screen regions
    getColorFromDC(&confirmColor, hMemoryDC, forzaConfirmX, forzaConfirmY);
    getColorFromDC(&listingColor, hMemoryDC, forzaListingAreaX, forzaListingAreaY);
    getColorFromDC(&placeBidColor, hMemoryDC, forzaPlaceBidX, forzaPlaceBidY);
    getColorFromDC(&collectCarColor, hMemoryDC, forzaCollectCarX, forzaCollectCarY);
    getColorFromDC(&startAuctionColor, hMemoryDC, forzaStartAuctionX, forzaStartAuctionY);

    if (restarting && startAuctionColor.r == 52 && startAuctionColor.g == 23 && startAuctionColor.b == 53) {
      restarting = false;
      botStages = 0; // reset bot stage bits
      SendMessage(m_ForzaHandle, WM_KEYDOWN, VK_RETURN, 0);
    }

    // Bot cycle
    if (!restarting && !endBotCycle && botActive) {
      // Check the RGB-color of the confirm button (if available)
      if (botStages == 0b000000) {
        SendMessage(m_ForzaHandle, WM_KEYDOWN, VK_RETURN, 0);
        confirmStartTime = clock();
        botStages |= 1 << 0;
      }
      else if (botStages == 0b000001 && getDeltaTime(clock(), confirmStartTime) < 500) {
        SendMessage(m_ForzaHandle, WM_KEYDOWN, VK_RETURN, 0);
      }

      if (botStages == 0b000001 && getDeltaTime(clock(), confirmStartTime) > 1450) {
        SendMessage(m_ForzaHandle, WM_KEYDOWN, VK_ESCAPE, 0);
        restartStartTime = clock();
        restarting = true;
      }

      if (!restarting) {
        if (botStages == 0b000001 && listingColor.r == 247 && listingColor.g == 247 && listingColor.b == 247) {
          SendMessage(m_ForzaHandle, WM_KEYDOWN, 0x59, 0);
          botStages |= 1 << 1;
        }
        else if (botStages == 0b000011) {
          SendMessage(m_ForzaHandle, WM_KEYDOWN, 0x59, 0);
        }

        if (botStages == 0b000011 &&
            placeBidColor.r == 171 && placeBidColor.g == 171 && placeBidColor.b == 171) {
          clickOptionsStart = clock();
          botStages |= 1 << 2;
        }

        if (botStages == 0b000111 && !stopOptionsTimer && !endBotCycle) {
          if (getDeltaTime(clock(), clickOptionsStart) > 300) {
            SendMessage(m_ForzaHandle, WM_KEYDOWN, VK_DOWN, 0);
            SendMessage(m_ForzaHandle, WM_KEYUP, VK_DOWN, 0);
            endBotCycleStart = clock();
            stopOptionsTimer = true;
          }
        }

        if (stopOptionsTimer && collectCarColor.r == 255 && collectCarColor.g == 0 && collectCarColor.b == 134 && !endBotCycle &&
            getDeltaTime(clock(), endBotCycleStart) > 1000) {
          endBotCycleStart = clock();
          endBotCycle = true;
        }
        else if (stopOptionsTimer && !endBotCycle) {
          SendMessage(m_ForzaHandle, WM_KEYDOWN, VK_RETURN, 0);
        }
      }
    }
    else if (endBotCycle) {
      int mSec = getDeltaTime(clock(), endBotCycleStart);

      if (mSec > 5000 && endStage == 1) {
        SendMessage(m_ForzaHandle, WM_KEYDOWN, VK_RETURN, 0);
        endStage = 2;
      }
      else if (mSec > 5500 && endStage == 2) {
        SendMessage(m_ForzaHandle, WM_KEYDOWN, VK_ESCAPE, 0);
        endStage = 3;
      }
      else if (mSec > 6000 && endStage == 3) {
        SendMessage(m_ForzaHandle, WM_KEYDOWN, VK_ESCAPE, 0);
        endStage = 4;
      }

      if (mSec >= 7000 && endStage == 4) {
        // Bot cycle completed, reset bot flags
        if (startAuctionColor.r == 52 && startAuctionColor.g == 23 && startAuctionColor.b == 53) {
          endBotCycle = false;
          botStages = 0;
          stopOptionsTimer = false;
          endStage = 1;
        }
        else {
          endBotCycleStart = clock();
          endStage = 1;
        }
      }
    }

    //PAINTSTRUCT ps;
    //GetObject(hBitmap, sizeof(BITMAP), &bm);

    //HDC hdc = BeginPaint(hwnd, &ps);

    //float aspectRatio = (float)bm.bmWidth / bm.bmHeight;
    //SetStretchBltMode(hdc, STRETCH_HALFTONE);
    //StretchBlt(hdc, 10, 170, (int)(360 * aspectRatio), 360,
    //    hMemoryDC,0,0, forzaClientRect.right - forzaClientRect.left,
    //    forzaClientRect.bottom - forzaClientRect.top, SRCCOPY);


    //EndPaint(hwnd, &ps);

    DeleteDC(hMemoryDC);
    DeleteDC(hScreenDC);
    DeleteObject(hBitmap);
    DeleteObject(hOldBitmap);
  }

  return 0;
}

void startAuctionBot(HWND hForzaHandle, const bool active) {
  if (!botActive) {
    m_ForzaHandle = hForzaHandle;
    botActive = active;

    // Create separate thread for bot
    m_BotThread = (HANDLE)_beginthreadex((void*)0, 0, &auctionBotThread, (void*)botActive, 0, 0);
  }
}

void stopAuctionBot() {
  if (botActive) {
    botActive = false;
    CloseHandle(m_BotThread);
  }
}

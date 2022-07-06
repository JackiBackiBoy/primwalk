#include "utilities.h"
#include <commctrl.h>
#include <stdio.h>
#include <process.h>
#include "resource.h"

BOOL m_BotActive = FALSE;
HANDLE m_TimingThread = NULL;

void auctionSniper(void* data) { 
  // This thread will terminate itself when condition is not met
  while (*(BOOL*)data == TRUE) {
    printf("Initiating auction sniper...\n");
    Sleep(1000);
    printf("Quick search...\n");
    Sleep(1000);
  }
}

void startAuctionBot(HWND hWnd) {
  // Dispatch timing thread executing the necessary inputs
  if (m_BotActive == FALSE) {
    m_BotActive = TRUE;
    m_TimingThread = (HANDLE)_beginthread(auctionSniper, 0, (void*)&m_BotActive); // last argument = data
  }
}

void stopAuctionBot(HWND hWnd) {
  if (m_BotActive == TRUE) {
    m_BotActive = FALSE;
    printf("Closing thread!");
  }
}

void findActiveWindows() {
  int windowCount = 0;
  EnumWindows(countWindowsCallback, (LPARAM)&windowCount);
  printf("Number of windows: %d\n", windowCount);
}

// Target presumed to be a list view
void listAllWindows(HWND* target) {
  EnumWindows(listWindowCallback, (LPARAM)target);
}

BOOL CALLBACK listWindowCallback(HWND hWnd, LPARAM lParam) {
  LVITEM matchListItem;
  matchListItem.pszText = L"Item";
  matchListItem.mask = LVIF_TEXT | LVIF_STATE;
  matchListItem.stateMask = 0;
  matchListItem.iItem = ;
  matchListItem.iSubItem = 0;
  matchListItem.state = 0;

  int res = ListView_InsertItem(hMatchList, &matchListItem);

  if (res == -1) {
    printf("Failed to add list item!\n");
  }

  ListView_SetItemText(hMatchList, res, 0, L"Item 1");
  ListView_SetItemText(hMatchList, res, 1, L"Item 1");
  ListView_SetItemText(hMatchList, res, 2, L"Item 1");
}

BOOL CALLBACK countWindowsCallback(HWND hWnd, LPARAM lParam) {
  int length = GetWindowTextLength(hWnd);

  if (length <= 0) {
    return TRUE;
  }

  WCHAR* buffer = (WCHAR*)malloc((length + 1) * sizeof(WCHAR));
  GetWindowText(hWnd, buffer, length + 1);


  int* windowCount = (int*)lParam;
  *windowCount += 1;

  DWORD written = 0;
  WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), buffer, length + 1, &written, NULL);
  printf("\n");

  free(buffer); // free the buffer from memory

  return TRUE;
}

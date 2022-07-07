#include "utilities.h"
#include <commctrl.h>
#include <stdio.h>
#include <process.h>
#include "resource.h"

BOOL m_BotActive = FALSE;
HANDLE m_TimingThread = NULL;
static int m_WindowCount = 0;

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

// Target presumed to be a list view
void listAllWindows(HWND* target) {
  EnumWindows(listWindowsCallback, (LPARAM)target);
  printf("Number of windows: %d\n", m_WindowCount);
  m_WindowCount = 0;
}

BOOL CALLBACK listWindowsCallback(HWND hWnd, LPARAM lParam) {
  int length = GetWindowTextLength(hWnd);

  if (length <= 0) {
    return TRUE;
  }

  WCHAR* buffer = (WCHAR*)malloc((length + 1) * sizeof(WCHAR));
  GetWindowText(hWnd, buffer, length + 1);

  // Get window handle ID and truncate it
  WCHAR longWindowId[16];
  swprintf(longWindowId, sizeof(longWindowId), L"%p", hWnd);
  WCHAR windowId[9];
  memcpy(windowId, &longWindowId[8], sizeof(WCHAR) * 8);
  windowId[8] = '\0';

  // Insert list items
  LVITEM matchListItem;
  matchListItem.pszText = L"Item";
  matchListItem.mask = LVIF_TEXT | LVIF_STATE;
  matchListItem.stateMask = 0;
  matchListItem.iItem = m_WindowCount;
  matchListItem.iSubItem = 0;
  matchListItem.state = 0;

  HWND listHandle = *(HWND*)lParam;
  int res = ListView_InsertItem(listHandle, &matchListItem);

  if (res == -1) {
    printf("Failed to add list item!\n");
  }

  ListView_SetItemText(listHandle, res, 0, buffer);
  ListView_SetItemText(listHandle, res, 1, windowId);
  free(buffer); // free the buffer from memory

  m_WindowCount++;

  return TRUE;
}

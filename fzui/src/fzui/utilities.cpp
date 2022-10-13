#include "fzui/utilities.hpp"
#include <commctrl.h>
#include <stdio.h>
#include <process.h>
#include "resource.hpp"

BOOL m_BotActive = FALSE;
static int m_WindowCount = 0;

// Target presumed to be a listview
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
  wchar_t itemText[16] = L"Item";
  LVITEM matchListItem;
  matchListItem.pszText = itemText;
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

void getColorFromDC(fz::Color* target, HDC hdc, const int x, const int y) {
  COLORREF tempColorRef = GetPixel(hdc, x, y);

  target->r = GetRValue(tempColorRef);
  target->g = GetGValue(tempColorRef);
  target->b = GetBValue(tempColorRef);
}


int getDeltaTime(const clock_t t1, const clock_t t2) {
  return (t1 - t2) * 1000 / CLOCKS_PER_SEC;
}

std::wstring convertToWideString(const std::string& string) {
  // Deal with trivial case of empty string
  if(string.empty()) { return std::wstring(); }

  // Determine required length of new string
  size_t reqLength = ::MultiByteToWideChar( CP_UTF8, 0, string.c_str(), (int)string.length(), 0, 0 );

  // Construct new string of required length
  std::wstring ret( reqLength, L'\0' );

  // Convert old string to new string
  ::MultiByteToWideChar( CP_UTF8, 0, string.c_str(), (int)string.length(), &ret[0], (int)ret.length() );

  // Return new string (compiler should optimize this away)
  return ret;
}

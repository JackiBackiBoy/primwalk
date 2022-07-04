#include "utilities.h"
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

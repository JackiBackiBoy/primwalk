#include "renderer.h"
#include <glad/glad.h>
#include <stdio.h>
#include <stdbool.h>
#include <process.h>

#define WGL_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB 0X2092
#define WGL_CONTEXT_FLAGS_ARB 0X2094
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB 0x00000001
#define WGL_CONTEXT_PROFILE_MASK_ARB 0x9126
typedef HGLRC(WINAPI* PFNWGLCREATECONTEXTATTRIBSARBPROC) (HDC hDC, HGLRC hShareContext, const int* attribList);

typedef const char* (WINAPI* PFNWGLGETEXTENSIONSSTRINGEXTPROC)(void);
typedef BOOL (WINAPI* PFNWGLSWAPINTERVALEXTPROC)(int);
typedef int (WINAPI* PFNWGLGETSWAPINTERVALEXTPROC) (void);

static HWND m_ParentWindow = NULL;
HANDLE m_GraphicsThread = NULL;
unsigned int m_VAO;
unsigned int m_VBO;
unsigned int m_EBO;

void setGraphicsParent(HWND parent) {
  m_ParentWindow = parent;
}

void createGraphicsWindow(void* instance) {
  HINSTANCE hInstance = *(HANDLE*)instance;

  WNDCLASSEX wcex;
  wcex.cbSize = sizeof(WNDCLASSEX);
  wcex.style = CS_HREDRAW | CS_VREDRAW; // activates reload on window
  wcex.lpfnWndProc = GraphicsProc;
  wcex.cbClsExtra = 0;
  wcex.cbWndExtra = 0;
  wcex.hInstance = hInstance;
  wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
  wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wcex.lpszMenuName = 0;
  wcex.lpszClassName = L"Forza Coach Renderer";

  if (!RegisterClassEx(&wcex)) {
    printf("Failed to register rendering window!");
  }

  int ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	int ScreenHeight = GetSystemMetrics(SM_CYSCREEN);
	int clientWidth = 800;
	int clientHeight = 600;

	RECT windowRect;
	//https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-setrect
	SetRect(&windowRect, (ScreenWidth / 2) - (clientWidth / 2),
		(ScreenHeight / 2) - (clientHeight / 2),
		(ScreenWidth / 2) + (clientWidth / 2),
		(ScreenHeight / 2) + (clientHeight / 2));


	// https://docs.microsoft.com/en-us/windows/win32/winmsg/window-styles
	DWORD style = (WS_CHILDWINDOW | WS_VISIBLE);
  DWORD exStyle = WS_EX_NOPARENTNOTIFY;

	// https://docs.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-adjustwindowrectex
	AdjustWindowRectEx(&windowRect, style, FALSE, exStyle);

  HWND graphicsWindow = CreateWindowEx(
      exStyle,
      wcex.lpszClassName,
      wcex.lpszClassName,
      style,
      0, 0,
      640, 480,
      m_ParentWindow == NULL ? 0 : m_ParentWindow, 0,
      hInstance, 0);

  HDC hdc = GetDC(graphicsWindow);

  PIXELFORMATDESCRIPTOR pfd;
  memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
  pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 24;
  pfd.cDepthBits = 32;
  pfd.cStencilBits = 8;

  int pixelFormat = ChoosePixelFormat(hdc, &pfd);
  SetPixelFormat(hdc, pixelFormat, &pfd);

  HGLRC tempRC = wglCreateContext(hdc);
  wglMakeCurrent(hdc, tempRC);

  PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;
	wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");

  const int attribList[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
		WGL_CONTEXT_MINOR_VERSION_ARB, 6,
		WGL_CONTEXT_FLAGS_ARB, 0,
		WGL_CONTEXT_PROFILE_MASK_ARB,
		WGL_CONTEXT_CORE_PROFILE_BIT_ARB, 0, 
	};

  HGLRC hglrc = wglCreateContextAttribsARB(hdc, 0, attribList);
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(tempRC);
	wglMakeCurrent(hdc, hglrc);

	if (!gladLoadGL())
	{
		printf("Could not initialize GLAD \n");
	}
	else {
		printf("%i\n", GLVersion.major);
	}

  PFNWGLGETEXTENSIONSSTRINGEXTPROC _wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)wglGetProcAddress("wglGetExtensionsStringEXT");
	bool swapControlSupported = strstr(_wglGetExtensionsStringEXT(), "WGL_EXT_swap_control") != 0;
	//https://www.khronos.org/opengl/wiki/Swap_Interval
	int vsync = 0;

	if (swapControlSupported) {
		PFNWGLSWAPINTERVALEXTPROC wglSwapInternalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
		PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC)wglGetProcAddress("wglGetSwapIntervalEXT");
		if (wglSwapInternalEXT(1))
		{
		  printf("VSync enabled \n");
		}
		else
		{
			printf("Could not enable VSynch");
		}
	}
	else
	{
	  printf("WGL_EXT_swap_control not supported \n");
	}

	// Shows window
	ShowWindow(graphicsWindow, SW_SHOW);
	UpdateWindow(graphicsWindow);

  DWORD lastTick = GetTickCount();
	MSG msg;
	while (true) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT) {
				break;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		DWORD thisTick = GetTickCount();
		float dt = (float)(thisTick - lastTick) * 0.001f;
		lastTick = thisTick;

    RECT clientRect;

    GetClientRect(graphicsWindow, &clientRect);
    clientWidth = clientRect.right - clientRect.left;
    clientHeight = clientRect.bottom - clientRect.top;

    // Update
    glViewport(0, 0, clientWidth, clientHeight);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    float aspect = (float)clientWidth / (float)clientHeight;
    
    SwapBuffers(hdc);
    if (vsync != 0) {
      glFinish();
    }
	}
}

void onGraphicsStart() {

}

void onGraphicsUpdate() {

}

LRESULT GraphicsProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
  switch(message)
	{
    case WM_DESTROY:
		  PostQuitMessage(0);
      break;
	  default:
		  return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

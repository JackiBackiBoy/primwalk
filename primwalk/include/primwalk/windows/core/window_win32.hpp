#pragma once

// primwalk
#include "primwalk/core.hpp"
#include "primwalk/ui/uiEvent.hpp"

// std
#include <atomic>
#include <cstdint>
#include <string>

// Windows
#include <windows.h>

namespace pw {
	class PW_API WindowWin32 : public WindowBase {
	public:
		WindowWin32(const std::string& name, int width, int height);
		~WindowWin32() = default;

		int run();

		void processEvent(const UIEvent& event) override;
		bool isCursorInTitleBar(int x, int y) const override;
		bool isCursorOnBorder(int x, int y) const override;
		inline bool isFullscreen() const { return m_Fullscreen.load(); }
		inline bool isMaximized() const { return m_Maximized.load(); }

		// Getters
		inline HWND getHandle() const { return m_Handle; }

		// Setters
		void setCursor(MouseCursor cursor) override;
		void toggleFullscreen();
		void toggleMaximize();

		void close() override;
		bool shouldClose() override;

	private:
		int init();

		std::wstring stringToWideString(const std::string& string) {
			std::wstring wString(string.size(), L' ');
			wString.resize(std::mbstowcs(&wString[0], string.c_str(), string.size()));
			return wString;
		}

		UIEvent createMouseEvent(unsigned int message, uint64_t wParam, int64_t lParam);
		inline LRESULT hitTest(HWND hWnd, WPARAM wParam, LPARAM lParam) const;
		static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

		// Rendering
		bool m_EnteringWindow = false;
		std::atomic<bool> m_Fullscreen { false };
		std::atomic<bool> m_Maximized { false };

		HINSTANCE m_Instance = NULL;
		HWND m_Handle = NULL;
		HICON m_Icon = NULL;
		HICON m_IconSmall = NULL;
		WINDOWPLACEMENT g_wpPrev = { sizeof(g_wpPrev) };

		bool m_TrackingMouseLeave = false;
		UIEvent m_MouseButtonEvent = { UIEventType::None };
	};

	typedef WindowWin32 Window;
}


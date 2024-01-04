#pragma once

#include "widget.hpp"
#include "../application.hpp"

#include <memory>
#include <vector>

namespace pw {
	struct WidgetEventData {
		MouseCursor cursorRequest = MouseCursor::Default;
	};

	// Acts as the widget manager
	class Editor {
	public:
		static Editor& getInstance();

		// Forbid copy/move semantics
		Editor(const Editor&) = delete;
		Editor(Editor&&) = delete;
		Editor& operator=(const Editor&) = delete;
		Editor& operator=(Editor&&) = delete;

		WidgetEventData processEvent(const UIEvent& event);

		template <typename T>
		T* addWidget() {
			auto widget = std::make_unique<T>();
			T* pWidget = widget.get();

			widget->m_ID = m_Widgets.size();

			m_Widgets.push_back(std::move(widget));

			return pWidget;
		}

		void draw(UIRenderSystem& renderer);
		void destroy(); // TODO: This is a temporary solution due to a separate Editor executable not being in the engine
						// this will be removed in the future.
		inline void setTarget(Application* target) { m_Target = target; }
		
	private:
		Editor();
		~Editor() = default;

		inline static Editor* m_Instance = nullptr;

		Widget* m_CurrentWidget = nullptr;
		std::vector<std::unique_ptr<Widget>> m_Widgets;
		WidgetEventData m_CurrentEventData{};
		Application* m_Target = nullptr;
	};
}
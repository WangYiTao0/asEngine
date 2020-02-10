#pragma once

#include "Core/Core.h"
#include "Events/Event.h"
#include "Core/Window.h"


namespace as
{
	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

	private:
		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
	};

	Application* CreateApplication();
}
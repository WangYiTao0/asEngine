#pragma once

#include "Core/Core.h"
#include "Events/Event.h"
#include "Core/Window.h"
#include "Core\LayerStack.h"
#include "Events\Event.h"
#include "Events\ApplicationEvent.h"
#include "PerfTimer.h"

#include <memory.h>
#include "GUI/ImGuiLayer.h"
#include "High_Level_Interface/MainComponent.h"

int main(int argc, char** argv);

namespace as
{
	class Application	
	{
	public:
		Application();
		virtual ~Application();

		virtual void OnInit() {}
		virtual void OnShutdown() {}
		virtual void OnUpdate(float dt) {}

		virtual void OnEvent(Event& e) {};

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);
		void RenderImGui() {};

		inline Window& GetWindow() { return *m_Window; }
		inline static Application& Get() { return *s_Instance; }
	private:
		void Run();
	private:
		std::unique_ptr<Window> m_Window;
		bool m_Running = true;
		LayerStack m_LayerStack;
		//ImGuiLayer* m_ImGuiLayer;
		PerfTimer timer;
	private:
		static Application* s_Instance;
		friend int ::main(int argc, char** argv);
	};

	Application* CreateApplication();
}
#include "aspch.h"
#include "Application.h"

#include "Events/ApplicationEvent.h"
#include "Core/Log.h"

#include "Graphics/API/asGraphicsDevice.h"
//#include "imgui.h"

namespace as
{
#define BIND_EVENT_FN(x) std::bind(&Application::x, this, std::placeholders::_1)

	Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		s_Instance = this;
		timer.Start();
		m_Window = std::unique_ptr<Window>(Window::Create());
	}

	void Application::Run()
	{
		OnInit();
		MSG msg = {0};

		while (msg.message!=WM_QUIT)
		{ 
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				// TranslateMessage will post auxilliary WM_CHAR messages from key msgs
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				for (Layer* layer : m_LayerStack)
					layer->OnUpdate(0);
			}
		}
		OnShutdown();
	}

	Application::~Application()
	{

	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* layer)
	{
		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}




}


#include "EditorLayer.h"
#include "Platform\Windows\Win32Window.h"

namespace as
{
	EditorLayer::EditorLayer()
	{

		
	}

	void EditorLayer::OnAttach()
	{
		auto& a = as::Application::Get();
		auto hwnd = HWND(a.GetWindow().GetWindow());

		m.SetWindow(hwnd);
		m.Initialize();
	}

	void EditorLayer::OnDetach()
	{
	}

	void EditorLayer::OnUpdate(float dt)
	{
		m.Run();
	}

	void EditorLayer::OnImGuiRender()
	{
	}

	void EditorLayer::OnEvent(as::Event& event)
	{
	}
}
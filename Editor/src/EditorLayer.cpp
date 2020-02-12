#include "EditorLayer.h"
#include "Platform\Windows\Win32Window.h"

EditorLayer::EditorLayer()
{
}

void EditorLayer::OnAttach()
{

	auto hwnd = HWND(as::Application::Get().GetWindow().GetWindow());

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

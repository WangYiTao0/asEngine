#include <asEngine.h>
#include <Core/EntryPoint.h>

#include "EditorLayer.h"


class EditorApp :public as::Application
{
public:
	EditorApp()
	{
		PushLayer(new as::EditorLayer());
	}
	~EditorApp()
	{

	}
};
as::Application* as::CreateApplication()
{
	return new EditorApp();
}

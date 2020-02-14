#include <asEngine.h>
#include <Core/EntryPoint.h>

#include "EditorLayer.h"


class EditorApp :public as::Application
{
public:
	EditorApp()
	{
	}
	~EditorApp()
	{

	}

	virtual void OnInit()override final { PushLayer(new as::EditorLayer()); }

};
as::Application* as::CreateApplication()
{
	return new EditorApp();
}

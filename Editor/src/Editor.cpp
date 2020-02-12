#include <asEngine.h>
#include <Core\EntryPoint.h>

#include "EditorLayer.h"

class Editor : public as::Application
{
public:
	Editor() {
		PushLayer(new EditorLayer());
	}
	~Editor() {

	}
};

as::Application* as::CreateApplication()
{
	return new Editor();
}
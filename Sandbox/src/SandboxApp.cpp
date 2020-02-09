#include <asEngine.h>
#include <Core/EntryPoint.h>

class Sandbox : public as::Application
{
public:
	Sandbox()
	{

	}

	~Sandbox()
	{

	}

};

as::Application* as::CreateApplication()
{
	return new Sandbox();
}
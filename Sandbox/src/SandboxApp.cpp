#include <asEngine.h>
#include <Core/EntryPoint.h>

//#include "Test.h"
class Sandbox : public as::Application
{
public:
	Sandbox()
	{
		//PushLayer(new Test());
	}

	~Sandbox()
	{
		
	}

};

as::Application* as::CreateApplication()
{
	return new Sandbox();
}
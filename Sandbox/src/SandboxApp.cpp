#include <asEngine.h>
#include <Core/EntryPoint.h>

class Sandbox : public asCore::Application
{
public:
	Sandbox()
	{

	}

	~Sandbox()
	{

	}

};

asCore::Application* asCore::CreateApplication()
{
	return new Sandbox();
}
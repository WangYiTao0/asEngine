#include <asEngine.h>
#include <EntryPoint.h>

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
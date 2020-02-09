#pragma once

int main(int argc, char** argv);

namespace as
{
	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

	private:
		static Application* s_Instance;
		friend int ::main(int argc, char** argv);
	};

	Application* CreateApplication();
}
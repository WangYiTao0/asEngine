#pragma once

#ifdef AS_PLATFORM_WINDOWS

extern as::Application* as::CreateApplication();

int main(int argc, char** argv)
{
	asLog::Log::Init();
	AS_CORE_WARN("Initialized Log!");
	int a = 5;
	AS_INFO("Hello! Var={0}", a);

	auto app = as::CreateApplication();

	app->Run();

	delete app;

}

#endif
#pragma once

#include "Core.h"
#include "System\asPlatform.h"
#include "Helpers/asStartupArguments.h"

#ifdef AS_PLATFORM_WINDOWS

extern as::Application* as::CreateApplication();

int main(int argc, char** argv)
{
	asLog::Log::Init();

	AS_CORE_WARN("Initialized Log!");

	auto app = as::CreateApplication();

	app->Run();

	delete app;

}

#endif
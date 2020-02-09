#pragma once

extern asCore::Application* asCore::CreateApplication();

int main(int argc, char** argv)
{
	auto app = asCore::CreateApplication();

	app->Run();

	delete app;

}
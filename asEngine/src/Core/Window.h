#pragma once
#include "aspch.h"

#include "Core/Core.h"
#include "Events/Event.h"


namespace as
{
	struct WindowProps
	{
		std::string Title;
		unsigned int Width;
		unsigned int Height;
		bool isVSync;
		bool isFullScreen;

		WindowProps(const std::string& title = "As Engine",
			unsigned int width = 1280, unsigned int height = 720,
			bool isVSync = true, bool isFullScreen = false)
			: Title(title), Width(width), Height(height),
			isVSync(isVSync), isFullScreen(isFullScreen)
		{
		}
	};

	// Interface representing a desktop system based Window
	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() {}

		virtual void OnUpdate() = 0;

		virtual unsigned int GetWidth() const = 0;
		virtual unsigned int GetHeight() const = 0;

		// Window attributes
		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
		virtual void SetVSync(bool enabled) = 0;
		virtual bool IsVSync() const = 0;

		virtual void* GetWindow() = 0;

		static Window* Create(const WindowProps& props = WindowProps());
	};


}
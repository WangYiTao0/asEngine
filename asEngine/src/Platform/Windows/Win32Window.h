#pragma once

#include <Windows.h>
#include "Core/Window.h"

namespace as
{
	class Win32Window : public Window
	{
	public:
		Win32Window(const WindowProps& props);
		~Win32Window();

		bool Initialize(const WindowProps& props);

		void OnUpdate() override;

		void Shutdown();

		inline unsigned int GetWidth() const override { return m_Data.Width; }
		inline unsigned int GetHeight() const override { return m_Data.Height; }

		static LRESULT CALLBACK HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
		static LRESULT CALLBACK HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
		LRESULT HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
		// Window attributes
		inline void SetEventCallback(const EventCallbackFn& callback) override { m_Data.EventCallback = callback; }
		void SetVSync(bool enabled) override;
		bool IsVSync() const override;

		inline HWND GetWindow()  { return m_Data.m_hWnd; }
	//	inline HWND GetWin32Handle() { return m_Data.m_hWnd; }

	private:

		struct Win32Data
		{
			std::wstring Title;
			unsigned int Width, Height;
			bool isVSync, isFullScreen;

			HINSTANCE m_AppInstance;
			HWND m_hWnd;

			EventCallbackFn EventCallback;
		};

		Win32Data m_Data;
	};

}
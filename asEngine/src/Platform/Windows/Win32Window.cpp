#include "aspch.h"
#include "Win32Window.h"

#include "Core/Log.h"

namespace as
{

	Window* Window::Create(const WindowProps& props)
	{
		return new Win32Window(props);
	}

	Win32Window::Win32Window(const WindowProps& props)
	{
		Initialize(props);
	}

	Win32Window::~Win32Window()
	{
		Shutdown();
	}

	bool Win32Window::Initialize(const WindowProps& props)
	{
		m_Data.Title = std::wstring(props.Title.begin(),props.Title.end());
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;
		m_Data.isVSync = props.isVSync;
		m_Data.isFullScreen = props.isFullScreen;
		m_Data.m_AppInstance = GetModuleHandle(NULL);

		AS_CORE_INFO("Creating window {0} ({1}, {2})", props.Title, props.Width, props.Height);
		

		WNDCLASSEX wndClass;
		memset(&wndClass, 0, sizeof(wndClass));
		wndClass.cbClsExtra = 0;
		wndClass.cbSize = sizeof(wndClass);
		wndClass.cbWndExtra = 0;
		wndClass.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);
		wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wndClass.hIconSm = wndClass.hIcon;
		wndClass.hInstance = m_Data.m_AppInstance;
		wndClass.lpfnWndProc = HandleMsgSetup;
		wndClass.lpszClassName = (LPCWSTR)m_Data.Title.c_str();
		wndClass.lpszMenuName = nullptr;
		wndClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;

		if (!RegisterClassEx(&wndClass))
		{
			MessageBox(NULL, TEXT("RegisterClassEx function failed"), TEXT("Error"), MB_OK | MB_ICONERROR);
			return false;
		}
	
		int screenWidth, screenHeight;
		int posX, posY;
		screenWidth = GetSystemMetrics(SM_CXSCREEN);
		screenHeight = GetSystemMetrics(SM_CYSCREEN);
		if (m_Data.isFullScreen)
		{
			DEVMODE dmScreenSettings;
			memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
			dmScreenSettings.dmSize = sizeof(dmScreenSettings);
			dmScreenSettings.dmPanningWidth = screenWidth;
			dmScreenSettings.dmPanningWidth = screenHeight;
			dmScreenSettings.dmBitsPerPel = 32;
			dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
			ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);

			m_Data.Width = screenWidth;
			m_Data.Height = screenHeight;

			posX = posY = 0;
		}
		else
		{
			posX = (screenWidth - m_Data.Width) / 2;
			posY = (screenHeight - m_Data.Height) / 2;
		}

		m_Data.m_hWnd = CreateWindowEx(
			WS_EX_APPWINDOW,
			wndClass.lpszClassName,
			wndClass.lpszClassName,
			WS_OVERLAPPEDWINDOW,
			posX, posY,
			m_Data.Width, m_Data.Height,
			NULL,
			NULL,
			m_Data.m_AppInstance,
			nullptr);

		if (!m_Data.m_hWnd)
		{
			MessageBox(NULL, TEXT("CreateWindowEx function failed"), TEXT("Error"), MB_OK | MB_ICONERROR);
			return false;
		}

		ShowWindow(m_Data.m_hWnd, SW_SHOW);
		UpdateWindow(m_Data.m_hWnd);
		SetFocus(m_Data.m_hWnd);

		ShowCursor(true);

		return true;
	}

	//std::optional<int> Win32Window::ProcessMessages() 
	//{
	//	MSG msg;
	//	// while queue has messages, remove and dispatch them (but do not block on empty queue)
	//	while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
	//	{
	//		// check for quit because peekmessage does not signal this via return val
	//		if (msg.message == WM_QUIT)
	//		{
	//			// return optional wrapping int (arg to PostQuitMessage is in wparam) signals quit
	//			return (int)msg.wParam;
	//		}

	//		// TranslateMessage will post auxilliary WM_CHAR messages from key msgs
	//		TranslateMessage(&msg);
	//		DispatchMessage(&msg);
	//	}

	//	// return empty optional when not quitting app
	//	return {};
	//}

	void Win32Window::OnUpdate()
	{

	}

	void Win32Window::Shutdown()
	{

		if (m_Data.isFullScreen)
			ChangeDisplaySettings(nullptr, 0);

		DestroyWindow(m_Data.m_hWnd);
		m_Data.m_hWnd = NULL;

		UnregisterClass((LPCTSTR)m_Data.Title.c_str(), m_Data.m_AppInstance);
		m_Data.m_AppInstance = NULL;


	}

	LRESULT CALLBACK Win32Window::HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
	{
		// use create parameter passed in from CreateWindow() to store window class pointer at WinAPI side
		if (msg == WM_NCCREATE)
		{
			// extract ptr to window class from creation data
			const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
			Win32Window* const pWnd = static_cast<Win32Window*>(pCreate->lpCreateParams);
			// set WinAPI-managed user data to store ptr to window class
			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
			// set message proc to normal (non-setup) handler now that setup is finished
			SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Win32Window::HandleMsgThunk));
			// forward message to window class handler
			return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
		}
		// if we get a message before the WM_NCCREATE message, handle with default handler
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	LRESULT CALLBACK Win32Window::HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
	{
		// retrieve ptr to window class
		Win32Window* const pWnd = reinterpret_cast<Win32Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
		// forward message to window class handler
		return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
	}

	LRESULT Win32Window::HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
	{
		switch (msg)
		{
		case WM_CLOSE:
			PostQuitMessage(0);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		}
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}

	void Win32Window::SetVSync(bool enabled)
	{

	}

	bool Win32Window::IsVSync() const
	{

		return m_Data.isVSync;
	}


}



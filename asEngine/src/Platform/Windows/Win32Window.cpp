#include "aspch.h"
#include "Win32Window.h"
#include "Helpers\asHelper.h"
#include "Tools/asBackLog.h"
#include "Core/Log.h"
#include "Helpers/asStartupArguments.h"
#include "../resource.h"
#include "Graphics/asRenderer.h"
#include "GUI\asWidget.h"
#include "input/asRawInput.h"


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
		LPWSTR szCmdLine;
		szCmdLine = GetCommandLineW();
		as::asStartupArguments::Parse(szCmdLine);

		//AS_CORE_INFO("CmdLineInfo--{0}", szCmdLine);

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

		HACCEL hAccelTable = LoadAccelerators(m_Data.m_AppInstance, MAKEINTRESOURCE(IDC_ASENGINEGAME));

		ShowWindow(m_Data.m_hWnd, SW_SHOW);
		UpdateWindow(m_Data.m_hWnd);
		SetFocus(m_Data.m_hWnd);
		ShowCursor(true);



		return true;
	}

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

	INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
	{
		UNREFERENCED_PARAMETER(lParam);
		switch (message)
		{
		case WM_INITDIALOG:
			return (INT_PTR)TRUE;

		case WM_COMMAND:
			if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
			{
				EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
			}
			break;
		}
		return (INT_PTR)FALSE;
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

	//extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	LRESULT Win32Window::HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept
	{
		//if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		//	return true;
		switch (msg)
		{
		case WM_COMMAND:
		{
			int wmId = LOWORD(wParam);
			// Parse the menu selections:
			switch (wmId)
			{
			case IDM_ABOUT:
				//DialogBox(m_Data.m_AppInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
				break;
			case IDM_EXIT:
				DestroyWindow(hWnd);
				break;
			default:
				return DefWindowProc(hWnd, msg, wParam, lParam);
			}
		}
		break;
		case WM_SIZE:
		{
			if (asRenderer::GetDevice() != nullptr)
			{
				int width = LOWORD(lParam);
				int height = HIWORD(lParam);

				asRenderer::GetDevice()->SetResolution(width, height);
				asRenderer::GetCamera().CreatePerspective((float)asRenderer::GetInternalResolution().x, (float)asRenderer::GetInternalResolution().y, 0.1f, 800);
			}
		}
		break;
		case WM_KEYDOWN:
			switch (wParam)
			{
			case VK_HOME:
				asBackLog::Toggle();
				break;
			case VK_UP:
				if (asBackLog::isActive())
					asBackLog::historyPrev();
				break;
			case VK_DOWN:
				if (asBackLog::isActive())
					asBackLog::historyNext();
				break;
			case VK_NEXT:
				if (asBackLog::isActive())
					asBackLog::Scroll(10);
				break;
			case VK_PRIOR:
				if (asBackLog::isActive())
					asBackLog::Scroll(-10);
				break;
			default:
				break;
			}
			break;
		case WM_HOTKEY:
			switch (wParam)
			{
			case PRINTSCREEN:
			{
				asHelper::screenshot();
			}
			break;
			default:
				break;
			}
			break;
		case WM_CHAR:
			switch (wParam)
			{
			case VK_BACK:
				if (asBackLog::isActive())
					asBackLog::deletefromInput();
				asTextInputField::DeleteFromInput();
				break;
			case VK_RETURN:
				if (asBackLog::isActive())
					asBackLog::acceptInput();
				break;
			default:
			{
				const char c = (const char)(TCHAR)wParam;
				if (asBackLog::isActive())
				{
					asBackLog::input(c);
				}
				asTextInputField::AddInput(c);
			}
			break;
			}
			break;
		case WM_INPUT:
			asRawInput::ParseMessage((void*)lParam);
			break;
		case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			// TODO: Add any drawing code that uses hdc here...
			EndPaint(hWnd, &ps);
		}
		break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, msg, wParam, lParam);
		}
		return 0;
	}

	void Win32Window::SetVSync(bool enabled)
	{

	}

	bool Win32Window::IsVSync() const
	{

		return m_Data.isVSync;
	}

	void* Win32Window::GetWindow() const
	{
		return m_Data.m_hWnd;
	}

}



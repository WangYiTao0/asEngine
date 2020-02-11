#include "aspch.h"
#include "CommonInclude.h"
#include "ImGuiLayer.h"
#include "Graphics\asRenderer.h"
#include "Core\Application.h"
#include "Platform\Windows\Win32Window.h"

#include <imgui.h>
#include <examples\imgui_impl_dx11.h>
#include <examples\imgui_impl_win32.h>

namespace as
{

	ImGuiLayer::ImGuiLayer()
		:Layer("ImGuiLayer")
	{


	}

	ImGuiLayer::~ImGuiLayer()
	{

	}

	void ImGuiLayer::OnAttach()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;
		ImGui::StyleColorsDark();

		Application& app = Application::Get();
		HWND hwnd = w32Window*>(app.GetWindow()).GetWindow();
		ImGui_ImplWin32_Init(hwnd);
		ImGui_ImplDX11_Init(asRenderer::GetDevice(), devicecontext);
	}

	void ImGuiLayer::OnDetach()
	{
		ImGui_ImplWin32_Shutdown();
		ImGui_ImplDX11_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiLayer::Begin()
	{
		ImGui_ImplWin32_NewFrame();
		ImGui_ImplDX11_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiLayer::End()
	{

		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();
		io.DisplaySize = ImVec2(app.GetWindow().GetWidth(), app.GetWindow().GetHeight());

		// Rendering
		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			//GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			//glfwMakeContextCurrent(backup_current_context);
		}
	}

}



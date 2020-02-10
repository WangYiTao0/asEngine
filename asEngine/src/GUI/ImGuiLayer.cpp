#include "aspch.h"
//#include "ImGuiLayer.h"
//#include "Graphics\asRenderer.h"
//
//#include <imgui.h>
//#include <examples\imgui_impl_dx11.h>
//
//namespace as
//{
//
//	ImGuiLayer::ImGuiLayer()
//		:Layer("ImGuiLayer")
//	{
//		IMGUI_CHECKVERSION();
//		ImGui::CreateContext();
//		ImGui::StyleColorsDark();
//		ImGuiIO& io = ImGui::GetIO(); (void)io;
//		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
//		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
//		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
//		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
//		//io.ConfigViewportsNoAutoMerge = true;
//		//io.ConfigViewportsNoTaskBarIcon = true;
//
//		// Setup Dear ImGui style
//		ImGui::StyleColorsDark();
//		//ImGui::StyleColorsClassic();
//
//			// Setup Dear ImGui style
//		ImGui::StyleColorsDark();
//		//ImGui::StyleColorsClassic();
//
//		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
//		ImGuiStyle& style = ImGui::GetStyle();
//		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
//		{
//			style.WindowRounding = 0.0f;
//			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
//		}
//		//ImGui_ImplDX11_Init(asRenderer::GetDevice(), asRenderer::GetContext);
//
//
//	}
//
//	ImGuiLayer::~ImGuiLayer()
//	{
//
//		ImGui::DestroyContext();
//	}
//
//	void ImGuiLayer::OnAttach()
//	{
//
//	}
//
//	void ImGuiLayer::OnDetach()
//	{
//
//	}
//
//	void ImGuiLayer::Begin()
//	{
//		ImGui::NewFrame();
//	}
//
//	void ImGuiLayer::End()
//	{
//		ImGui::End();
//	}
//
//}
//
//

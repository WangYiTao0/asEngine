#include "aspch.h"
#include "MainComponent.h"
#include "RenderPath.h"
#include "Graphics/asRenderer.h"
#include "Helpers/asHelper.h"
#include "Helpers/asTimer.h"
#include "Input/asInput.h"
#include "Tools/asBackLog.h"
//#include "MainComponent_BindLua.h"
//#include "wiVersion.h"
#include "Graphics/asEnums.h"
#include "Graphics/asTextureHelper.h"
#include "Tools/asProfiler.h"
#include "System/asInitializer.h"
#include "Helpers/asStartupArguments.h"
#include "Graphics/asFont.h"
#include "Graphics/asImage.h"

#include "Graphics/API/asGraphicsDevice_DX11.h"
#include "Graphics/API/asGraphicsDevice_DX12.h"
#include "Graphics/API/asGraphicsDevice_Vulkan.h"

using namespace std;

namespace as
{
	using namespace asGraphics;

	void MainComponent::Initialize()
	{
		if (initialized)
		{
			return;
		}
		initialized = true;

		asHelper::GetOriginalWorkingDirectory();

		// User can also create a graphics device if custom logic is desired, but he must do before this function!
		if (asRenderer::GetDevice() == nullptr)
		{
			auto window = asPlatform::GetWindow();

			bool debugdevice = asStartupArguments::HasArgument("debugdevice");

			if (asStartupArguments::HasArgument("vulkan"))
			{
#ifdef ASENGINE_BUILD_VULKAN
				//asRenderer::SetShaderPath(asRenderer::GetShaderPath() + "spirv/");
				//asRenderer::SetDevice(std::make_shared<GraphicsDevice_Vulkan>(window, fullscreen, debugdevice));
#else
				asHelper::messageBox("Vulkan SDK not found during building the application! Vulkan API disabled!", "Error");
#endif
			}
			else if (asStartupArguments::HasArgument("dx12"))
			{
				if (asStartupArguments::HasArgument("hlsl6"))
				{
					asRenderer::SetShaderPath(asRenderer::GetShaderPath() + "hlsl6/");
				}
				asRenderer::SetDevice(std::make_shared<GraphicsDevice_DX12>(window, fullscreen, debugdevice));
			}

			// default graphics device:
			if (asRenderer::GetDevice() == nullptr)
			{
				asRenderer::SetDevice(std::make_shared<GraphicsDevice_DX11>(window, fullscreen, debugdevice));
			}
		}
		asInitializer::InitializeComponentsAsync();
	}

	void MainComponent::ActivatePath(RenderPath* component, float fadeSeconds, asColor fadeColor)
	{
		if (component == nullptr)
		{
			return;
		}
		// Fade manager will activate on fadeout
		fadeManager.Clear();
		fadeManager.Start(fadeSeconds, fadeColor, [this, component]() {

			if (activePath != nullptr)
			{
				activePath->Stop();
			}

			component->Start();
			activePath = component;
			});

		fadeManager.Update(0); // If user calls ActivatePath without fadeout, it will be instant
	}

	void MainComponent::Run()
	{
		if (!initialized)
		{
			// Initialize in a lazy way, so the user application doesn't have to call this explicitly
			Initialize();
			initialized = true;
		}
		if (!asInitializer::IsInitializeFinished())
		{
			// Until engine is not loaded, present initialization screen...
			CommandList cmd = asRenderer::GetDevice()->BeginCommandList();
			asRenderer::GetDevice()->PresentBegin(cmd);
			asFont(asBackLog::getText(), asFontParams(4, 4, infoDisplay.size)).Draw(cmd);
			asRenderer::GetDevice()->PresentEnd(cmd);
			return;
		}

		//static bool startup_script = false;
		//if (!startup_script)
		//{
		//	startup_script = true;
		//	//wiLua::GetGlobal()->RegisterObject(MainComponent_BindLua::className, "main", new MainComponent_BindLua(this));
		//	//wiLua::GetGlobal()->RunFile("startup.lua");
		//}

		asProfiler::BeginFrame();

		deltaTime = float(std::max(0.0, timer.elapsed() / 1000.0));
		timer.record();

		if (asPlatform::IsWindowActive())
		{
			// If the application is active, run Update loops:

			const float dt = framerate_lock ? (1.0f / targetFrameRate) : deltaTime;

			fadeManager.Update(dt);

			// Fixed time update:
			auto range = asProfiler::BeginRangeCPU("Fixed Update");
			{
				if (frameskip)
				{
					deltaTimeAccumulator += dt;
					if (deltaTimeAccumulator > 10)
					{
						// application probably lost control, fixed update would take too long
						deltaTimeAccumulator = 0;
					}

					const float targetFrameRateInv = 1.0f / targetFrameRate;
					while (deltaTimeAccumulator >= targetFrameRateInv)
					{
						FixedUpdate();
						deltaTimeAccumulator -= targetFrameRateInv;
					}
				}
				else
				{
					FixedUpdate();
				}
			}
			asProfiler::EndRange(range); // Fixed Update

			// Variable-timed update:
			Update(dt);

			asInput::Update();

			Render();
		}
		else
		{
			// If the application is not active, disable Update loops:
			deltaTimeAccumulator = 0;
		}

		CommandList cmd = asRenderer::GetDevice()->BeginCommandList();
		asRenderer::GetDevice()->PresentBegin(cmd);
		{
			Compose(cmd);
			asProfiler::EndFrame(cmd); // End before Present() so that GPU queries are properly recorded
		}
		asRenderer::GetDevice()->PresentEnd(cmd);

		asRenderer::EndFrame();
	}

	void MainComponent::Update(float dt)
	{
		auto range = asProfiler::BeginRangeCPU("Update");

		//wiLua::GetGlobal()->SetDeltaTime(double(dt));
		//wiLua::GetGlobal()->Update();

		if (GetActivePath() != nullptr)
		{
			GetActivePath()->Update(dt);
		}

		asProfiler::EndRange(range); // Update
	}

	void MainComponent::FixedUpdate()
	{
		asBackLog::Update();
		//wiLua::GetGlobal()->FixedUpdate();

		if (GetActivePath() != nullptr)
		{
			GetActivePath()->FixedUpdate();
		}
	}

	void MainComponent::Render()
	{
		auto range = asProfiler::BeginRangeCPU("Render");

		//wiLua::GetGlobal()->Render();

		if (GetActivePath() != nullptr)
		{
			GetActivePath()->Render();
		}

		asProfiler::EndRange(range); // Render
	}

	void MainComponent::Compose(CommandList cmd)
	{
		auto range = asProfiler::BeginRangeCPU("Compose");

		if (GetActivePath() != nullptr)
		{
			GetActivePath()->Compose(cmd);
		}

		if (fadeManager.IsActive())
		{
			// display fade rect
			static asImageParams fx;
			fx.siz.x = (float)asRenderer::GetDevice()->GetScreenWidth();
			fx.siz.y = (float)asRenderer::GetDevice()->GetScreenHeight();
			fx.opacity = fadeManager.opacity;
			asImage::Draw(asTextureHelper::getColor(fadeManager.color), fx, cmd);
		}

		// Draw the information display
		if (infoDisplay.active)
		{
			stringstream ss("");
			if (infoDisplay.watermark)
			{
				ss << "As Engine " << "1.0" << " ";

#if defined(_ARM)
				ss << "[ARM]";
#elif defined(_WIN64)
				ss << "[64-bit]";
#elif defined(_WIN32)
				ss << "[32-bit]";
#endif

				if (dynamic_cast<GraphicsDevice_DX11*>(asRenderer::GetDevice()))
				{
					ss << "[DX11]";
				}
				else if (dynamic_cast<GraphicsDevice_DX12*>(asRenderer::GetDevice()))
				{
					ss << "[DX12]";
				}
#ifdef ASENGINE_BUILD_VULKAN
				else if (dynamic_cast<GraphicsDevice_Vulkan*>(asRenderer::GetDevice()))
				{
					ss << "[Vulkan]";
				}
#endif

#ifdef _DEBUG
				ss << "[DEBUG]";
#endif
				if (asRenderer::GetDevice()->IsDebugDevice())
				{
					ss << "[debugdevice]";
				}
				ss << endl;
			}
			if (infoDisplay.resolution)
			{
				ss << "Resolution: " << asRenderer::GetDevice()->GetScreenWidth() << " x " << asRenderer::GetDevice()->GetScreenHeight() << endl;
			}
			if (infoDisplay.fpsinfo)
			{
				ss.precision(2);
				ss << fixed << 1.0f / deltaTime << " FPS" << endl;
#ifdef _DEBUG
				ss << "Warning: This is a [DEBUG] build, performance will be slow!" << endl;
#endif
				if (asRenderer::GetDevice()->IsDebugDevice())
				{
					ss << "Warning: Graphics is in [debugdevice] mode, performance will be slow!" << endl;
				}
			}
			ss.precision(2);
			asFont(ss.str(), asFontParams(4, 4, infoDisplay.size, ASFALIGN_LEFT, ASFALIGN_TOP, 0, 0, asColor(255, 255, 255, 255), asColor(0, 0, 0, 255))).Draw(cmd);
		}

		asProfiler::DrawData(4, 120, cmd);

		asBackLog::Draw(cmd);

		asProfiler::EndRange(range); // Compose
	}

	void MainComponent::SetWindow(asPlatform::window_type window)
	{
		asPlatform::GetWindow() = window;
	}
}




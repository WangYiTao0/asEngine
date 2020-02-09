#include "aspch.h"
#include "LoadingScreen.h"
#include "MainComponent.h"

namespace as
{

	bool LoadingScreen::isActive()
	{
		return asJobSystem::IsBusy(ctx_main) || asJobSystem::IsBusy(ctx_finish);
	}

	void LoadingScreen::addLoadingFunction(std::function<void()> loadingFunction)
	{
		if (loadingFunction != nullptr)
		{
			tasks.push_back(loadingFunction);
		}
	}

	void LoadingScreen::addLoadingComponent(RenderPath* component, MainComponent* main, float fadeSeconds, asColor fadeColor)
	{
		addLoadingFunction([=] {
			component->Load();
			});
		onFinished([=] {
			main->ActivatePath(component, fadeSeconds, fadeColor);
			});
	}

	void LoadingScreen::onFinished(std::function<void()> finishFunction)
	{
		if (finishFunction != nullptr)
			finish = finishFunction;
	}

	void LoadingScreen::Unload()
	{
		RenderPath2D::Unload();
	}

	void LoadingScreen::Start()
	{
		for (auto& x : tasks)
		{
			asJobSystem::Execute(ctx_main, x);
		}
		asJobSystem::Execute(ctx_finish, [this] {
			asJobSystem::Wait(ctx_main);
			finish();
			});

		RenderPath2D::Start();
	}

	void LoadingScreen::Stop()
	{
		tasks.clear();
		finish = nullptr;

		RenderPath2D::Stop();
	}

}





#pragma once
#include "RenderPath2D.h"
#include "Helpers/asColors.h"
#include "System/asJobSystem.h"

#include <functional>

namespace as
{
	class MainComponent;

	class LoadingScreen :
		public RenderPath2D
	{
	private:
		asJobSystem::context ctx_main;
		asJobSystem::context ctx_finish;
		std::vector<std::function<void()>> tasks;
		std::function<void()> finish;
	public:

		//Add a loading task which should be executed
		//use std::bind( YourFunctionPointer )
		void addLoadingFunction(std::function<void()> loadingFunction);
		//Helper for loading a whole renderable component
		void addLoadingComponent(RenderPath* component, MainComponent* main, float fadeSeconds = 0, asColor fadeColor = asColor(0, 0, 0, 255));
		//Set a function that should be called when the loading finishes
		//use std::bind( YourFunctionPointer )
		void onFinished(std::function<void()> finishFunction);
		//Get percentage of finished loading tasks (values 0-100)
		int getPercentageComplete();
		//See if the loading is currently running
		bool isActive();

		//Start Executing the tasks and mark the loading as active
		virtual void Start() override;
		//Clear all tasks
		virtual void Stop() override;

		virtual void Unload() override;
	};
}

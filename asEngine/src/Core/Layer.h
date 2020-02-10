#pragma once

#include "Core/Core.h"
#include "Events\Event.h"
#include "Core/PerfTimer.h"


namespace as
{
	class Layer
	{
	public:
		Layer(const std::string& debugName = "Layer");
		virtual ~Layer() = default;

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate(float dt) {}
		virtual void OnImGuiRender() {}
		virtual void OnEvent(Event & event) {}

		inline const std::string& GetName()const { return m_DebugName; }
	private:
		std::string m_DebugName;
	};

}
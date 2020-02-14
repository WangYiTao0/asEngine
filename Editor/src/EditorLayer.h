#pragma once
#include <asEngine.h>
#include "EditorComponent.h"

namespace as
{
	class EditorLayer :public as::Layer
	{
	public:
		EditorLayer();

		virtual void OnAttach()override;
		virtual void OnDetach() override;
		virtual void OnUpdate(float dt) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(as::Event& event) override;
	private:
		Editor m;
	};
}
#pragma once
#include "CommonInclude.h"
#include "Graphics/API/asGraphicsDevice.h"
#include "System/asScene.h"

#include <list>

namespace as
{

	class asHashString;

	class asWidget;

	class asGUI : public asScene::TransformComponent
	{
		friend class asWidget;
	private:
		std::list<asWidget*> widgets;
		asWidget* activeWidget;
		bool focus;
		bool visible;

		XMFLOAT2 pointerpos;
	public:
		asGUI();
		~asGUI();

		void Update(float dt);
		void Render(asGraphics::CommandList cmd) const;

		void AddWidget(asWidget* widget);
		void RemoveWidget(asWidget* widget);
		asWidget* GetWidget(const asHashString& name);

		void ActivateWidget(asWidget* widget);
		void DeactivateWidget(asWidget* widget);
		const asWidget* GetActiveWidget() const;
		// true if another widget is currently active
		bool IsWidgetDisabled(asWidget* widget);

		// returns true if any gui element has the focus
		bool HasFocus();

		void SetVisible(bool value) { visible = value; }
		bool IsVisible() { return visible; }

		void ResetScissor(asGraphics::CommandList cmd) const;


		const XMFLOAT2& GetPointerPos() const
		{
			return pointerpos;
		}
	};
}

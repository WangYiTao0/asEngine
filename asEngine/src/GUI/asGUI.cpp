#include "aspch.h"
#include "asGUI.h"

#include "asGUI.h"
#include "asWidget.h"
#include "Helpers/asHashString.h"
#include "Graphics/asRenderer.h"
#include "Input/asInput.h"

using namespace std;

namespace as
{
	using namespace asGraphics;

	asGUI::asGUI() : activeWidget(nullptr), focus(false), visible(true), pointerpos(XMFLOAT2(0, 0))
	{
		SetDirty();
		scale_local.x = (float)asRenderer::GetDevice()->GetScreenWidth();
		scale_local.y = (float)asRenderer::GetDevice()->GetScreenHeight();
		UpdateTransform();
	}


	asGUI::~asGUI()
	{
	}


	void asGUI::Update(float dt)
	{
		if (!visible)
		{
			return;
		}

		if (asRenderer::GetDevice()->ResolutionChanged())
		{
			SetDirty();
			scale_local.x = (float)asRenderer::GetDevice()->GetScreenWidth();
			scale_local.y = (float)asRenderer::GetDevice()->GetScreenHeight();
			UpdateTransform();
		}

		XMFLOAT4 _p = asInput::GetPointer();
		pointerpos.x = _p.x;
		pointerpos.y = _p.y;

		if (activeWidget != nullptr)
		{
			if (!activeWidget->IsEnabled() || !activeWidget->IsVisible())
			{
				// deactivate active widget if it became invisible or disabled
				DeactivateWidget(activeWidget);
			}
		}

		focus = false;
		for (auto& widget : widgets)
		{
			if (widget->parent == this)
			{
				// the contained child widgets will be updated by the containers
				widget->Update(this, dt);
			}

			if (widget->IsEnabled() && widget->IsVisible() && widget->GetState() > asWidget::ASWIDGETSTATE::IDLE)
			{
				focus = true;
			}
		}
	}

	void asGUI::Render(CommandList cmd) const
	{
		if (!visible)
		{
			return;
		}

		asRenderer::GetDevice()->EventBegin("GUI", cmd);
		for (auto& x : widgets)
		{
			if (x->parent == this && x != activeWidget)
			{
				// the contained child widgets will be rendered by the containers
				ResetScissor(cmd);
				x->Render(this, cmd);
			}
		}
		if (activeWidget != nullptr)
		{
			// render the active widget on top of everything
			ResetScissor(cmd);
			activeWidget->Render(this, cmd);
		}

		for (auto& x : widgets)
		{
			x->RenderTooltip(this, cmd);
		}

		ResetScissor(cmd);
		asRenderer::GetDevice()->EventEnd(cmd);
	}

	void asGUI::ResetScissor(CommandList cmd) const
	{
		asGraphics::Rect scissor[1];
		scissor[0].bottom = (int32_t)(asRenderer::GetDevice()->GetScreenHeight());
		scissor[0].left = (int32_t)(0);
		scissor[0].right = (int32_t)(asRenderer::GetDevice()->GetScreenWidth());
		scissor[0].top = (int32_t)(0);
		asRenderer::GetDevice()->BindScissorRects(1, scissor, cmd);
	}

	void asGUI::AddWidget(asWidget* widget)
	{
		widget->AttachTo(this);
		widgets.push_back(widget);
	}

	void asGUI::RemoveWidget(asWidget* widget)
	{
		widget->Detach();
		widgets.remove(widget);
	}

	asWidget* asGUI::GetWidget(const asHashString& name)
	{
		for (auto& x : widgets)
		{
			if (x->GetName() == name)
			{
				return x;
			}
		}
		return nullptr;
	}

	void asGUI::ActivateWidget(asWidget* widget)
	{
		activeWidget = widget;
		activeWidget->Activate();
	}
	void asGUI::DeactivateWidget(asWidget* widget)
	{
		widget->Deactivate();
		if (activeWidget == widget)
		{
			activeWidget = nullptr;
		}
	}
	const asWidget* asGUI::GetActiveWidget() const
	{
		return activeWidget;
	}
	bool asGUI::IsWidgetDisabled(asWidget* widget)
	{
		return (activeWidget != nullptr && activeWidget != widget);
	}
	bool asGUI::HasFocus()
	{
		if (!visible)
		{
			return false;
		}

		return focus;
	}

}

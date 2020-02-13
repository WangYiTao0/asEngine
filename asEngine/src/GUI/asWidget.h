#pragma once
#include "CommonInclude.h"
#include "Helpers/asHashString.h"
#include "Helpers/asColors.h"
#include "Graphics/API/asGraphicsDevice.h"
#include "Helpers/asIntersect.h"
#include "System/asScene.h"

#include <string>
#include <list>
#include <functional>


namespace as
{
	class asGUI;

	struct asEventArgs
	{
		XMFLOAT2 clickPos;
		XMFLOAT2 startPos;
		XMFLOAT2 deltaPos;
		XMFLOAT2 endPos;
		float fValue;
		bool bValue;
		int iValue;
		asColor color;
		std::string sValue;
	};

	class asWidget : public asScene::TransformComponent
	{
		friend class asGUI;
	public:
		enum ASWIDGETSTATE
		{
			IDLE,			// widget is doing nothing
			FOCUS,			// widget got pointer dragged on or selected
			ACTIVE,			// widget is interacted with right now
			DEACTIVATING,	// widget has last been active but no more interactions are occuring
			ASWIDGETSTATE_COUNT,
		};
	private:
		int tooltipTimer = 0;
	protected:
		asHashString fastName;
		std::string text;
		std::string tooltip;
		std::string scriptTip;
		bool enabled = true;
		bool visible = true;

		ASWIDGETSTATE state = IDLE;
		void Activate();
		void Deactivate();
		asGraphics::Rect scissorRect;

		asColor colors[ASWIDGETSTATE_COUNT] = {
			asColor::Booger(),
			asColor::Gray(),
			asColor::White(),
			asColor::Gray(),
		};
		static_assert(arraysize(colors) == ASWIDGETSTATE_COUNT, "Every ASWIDGETSTATE needs a default color!");

		asColor textColor = asColor(255, 255, 255, 255);
		asColor textShadowColor = asColor(0, 0, 0, 255);
	public:
		const asHashString& GetName() const;
		void SetName(const std::string& value);
		const std::string& GetText() const;
		void SetText(const std::string& value);
		void SetTooltip(const std::string& value);
		void SetScriptTip(const std::string& value);
		void SetPos(const XMFLOAT2& value);
		void SetSize(const XMFLOAT2& value);
		ASWIDGETSTATE GetState() const;
		virtual void SetEnabled(bool val);
		bool IsEnabled() const;
		virtual void SetVisible(bool val);
		bool IsVisible() const;
		// last param default: set color for all states
		void SetColor(asColor color, ASWIDGETSTATE state = ASWIDGETSTATE_COUNT);
		asColor GetColor() const;
		void SetScissorRect(const asGraphics::Rect& rect);
		void SetTextColor(asColor value) { textColor = value; }
		void SetTextShadowColor(asColor value) { textShadowColor = value; }

		virtual void Update(asGUI* gui, float dt);
		virtual void Render(const asGUI* gui, asGraphics::CommandList cmd) const = 0;
		void RenderTooltip(const asGUI* gui, asGraphics::CommandList cmd) const;

		XMFLOAT3 translation = XMFLOAT3(0, 0, 0);
		XMFLOAT3 scale = XMFLOAT3(1, 1, 1);

		Hitbox2D hitBox;

		asScene::TransformComponent* parent = nullptr;
		XMFLOAT4X4 world_parent_bind = IDENTITYMATRIX;
		void AttachTo(asScene::TransformComponent* parent);
		void Detach();

		static void LoadShaders();
	};

	// Clickable, draggable box
	class asButton : public asWidget
	{
	protected:
		std::function<void(asEventArgs args)> onClick;
		std::function<void(asEventArgs args)> onDragStart;
		std::function<void(asEventArgs args)> onDrag;
		std::function<void(asEventArgs args)> onDragEnd;
		XMFLOAT2 dragStart = XMFLOAT2(0, 0);
		XMFLOAT2 prevPos = XMFLOAT2(0, 0);
	public:
		asButton(const std::string& name = "");
		virtual ~asButton();

		virtual void Update(asGUI* gui, float dt) override;
		virtual void Render(const asGUI* gui, asGraphics::CommandList cmd) const override;

		void OnClick(std::function<void(asEventArgs args)> func);
		void OnDragStart(std::function<void(asEventArgs args)> func);
		void OnDrag(std::function<void(asEventArgs args)> func);
		void OnDragEnd(std::function<void(asEventArgs args)> func);
	};

	// Static box that holds text
	class asLabel : public asWidget
	{
	protected:
	public:
		asLabel(const std::string& name = "");
		virtual ~asLabel();

		virtual void Update(asGUI* gui, float dt) override;
		virtual void Render(const asGUI* gui, asGraphics::CommandList cmd) const override;
	};

	// Text input box
	class asTextInputField : public asWidget
	{
	protected:
		std::function<void(asEventArgs args)> onInputAccepted;

		std::string value;
		static std::string value_new;
	public:
		asTextInputField(const std::string& name = "");
		virtual ~asTextInputField();

		void SetValue(const std::string& newValue);
		void SetValue(int newValue);
		void SetValue(float newValue);
		const std::string& GetValue();

		// There can only be ONE active text input field, so these methods modify the active one
		static void AddInput(const char inputChar);
		static void DeleteFromInput();

		virtual void Update(asGUI* gui, float dt) override;
		virtual void Render(const asGUI* gui, asGraphics::CommandList cmd) const override;

		void OnInputAccepted(std::function<void(asEventArgs args)> func);
	};

	// Define an interval and slide the control along it
	class asSlider : public asWidget
	{
	protected:
		std::function<void(asEventArgs args)> onSlide;
		float start = 0, end = 1;
		float step = 1000;
		float value = 0;

		asTextInputField* valueInputField;
	public:
		// start : slider minimum value
		// end : slider maximum value
		// defaultValue : slider default Value
		// step : slider step size
		asSlider(float start = 0.0f, float end = 1.0f, float defaultValue = 0.5f, float step = 1000.0f, const std::string& name = "");
		virtual ~asSlider();

		void SetValue(float value);
		float GetValue();
		void SetRange(float start, float end);

		virtual void Update(asGUI* gui, float dt) override;
		virtual void Render(const asGUI* gui, asGraphics::CommandList cmd) const override;

		void OnSlide(std::function<void(asEventArgs args)> func);
	};

	// Two-state clickable box
	class asCheckBox :public asWidget
	{
	protected:
		std::function<void(asEventArgs args)> onClick;
		bool checked = false;
	public:
		asCheckBox(const std::string& name = "");
		virtual ~asCheckBox();

		void SetCheck(bool value);
		bool GetCheck() const;

		virtual void Update(asGUI* gui, float dt) override;
		virtual void Render(const asGUI* gui, asGraphics::CommandList cmd) const override;

		void OnClick(std::function<void(asEventArgs args)> func);
	};

	// Drop-down list
	class asComboBox :public asWidget
	{
	protected:
		std::function<void(asEventArgs args)> onSelect;
		int selected = -1;
		int maxVisibleItemCount = 8;
		int firstItemVisible = 0;

		// While the widget is active (rolled down) these are the inner states that control behaviour
		enum COMBOSTATE
		{
			COMBOSTATE_INACTIVE,	// When the list is just being dropped down, or the widget is not active
			COMBOSTATE_HOVER,		// The widget is in drop-down state with the last item hovered highlited
			COMBOSTATE_SELECTING,	// The hovered item is clicked
			COMBOSTATE_SCROLLBAR_HOVER,		// scrollbar is to be selected
			COMBOSTATE_SCROLLBAR_GRABBED,	// scrollbar is moved
			COMBOSTATE_COUNT,
		} combostate = COMBOSTATE_INACTIVE;
		int hovered = -1;

		float scrollbar_delta = 0;

		std::vector<std::string> items;

		float GetItemOffset(int index) const;
	public:
		asComboBox(const std::string& name = "");
		virtual ~asComboBox();

		void AddItem(const std::string& item);
		void RemoveItem(int index);
		void ClearItems();
		void SetMaxVisibleItemCount(int value);
		bool HasScrollbar() const;

		void SetSelected(int index);
		int GetSelected();
		std::string GetItemText(int index);

		virtual void Update(asGUI* gui, float dt) override;
		virtual void Render(const asGUI* gui, asGraphics::CommandList cmd) const override;

		void OnSelect(std::function<void(asEventArgs args)> func);
	};

	// Widget container
	class asWindow :public asWidget
	{
	protected:
		asGUI* gui = nullptr;
		asButton* closeButton = nullptr;
		asButton* minimizeButton = nullptr;
		asButton* resizeDragger_UpperLeft = nullptr;
		asButton* resizeDragger_BottomRight = nullptr;
		asButton* moveDragger = nullptr;
		std::list<asWidget*> childrenWidgets;
		bool minimized = false;
	public:
		asWindow(asGUI* gui, const std::string& name = "");
		virtual ~asWindow();

		void AddWidget(asWidget* widget);
		void RemoveWidget(asWidget* widget);
		void RemoveWidgets(bool alsoDelete = false);

		virtual void Update(asGUI* gui, float dt) override;
		virtual void Render(const asGUI* gui, asGraphics::CommandList cmd) const override;

		virtual void SetVisible(bool value) override;
		virtual void SetEnabled(bool value) override;
		void SetMinimized(bool value);
		bool IsMinimized() const;
	};

	// HSV-Color Picker
	class asColorPicker : public asWindow
	{
	protected:
		std::function<void(asEventArgs args)> onColorChanged;
		bool huefocus = false; // whether the hue picker is in focus or the saturation
		float hue = 0.0f;			// [0, 360] degrees
		float saturation = 0.0f;	// [0, 1]
		float luminance = 1.0f;		// [0, 1]
	public:
		asColorPicker(asGUI* gui, const std::string& name = "");

		virtual void Update(asGUI* gui, float dt) override;
		virtual void Render(const asGUI* gui, asGraphics::CommandList cmd) const override;

		asColor GetPickColor() const;
		void SetPickColor(asColor value);

		void OnColorChanged(std::function<void(asEventArgs args)> func);
	};

}

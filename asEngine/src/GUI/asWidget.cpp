#include "aspch.h"
#include "asWidget.h"


#include "asWidget.h"
#include "asGUI.h"
#include "Graphics/asImage.h"
#include "Graphics/asTextureHelper.h"
#include "Graphics/asFont.h"
#include "Helpers/asMath.h"
#include "Helpers/asHelper.h"
#include "Input/asInput.h"
#include "Graphics/asRenderer.h"
#include "Graphics/GPUMapping/ShaderInterop_Renderer.h"

#include <DirectXCollision.h>

#include <sstream>

using namespace std;

namespace as
{
	using namespace asGraphics;
	using namespace asScene;


	static asGraphics::PipelineState PSO_colorpicker;

	void asWidget::Update(asGUI* gui, float dt)
	{
		assert(gui != nullptr && "Ivalid GUI!");

		Hitbox2D pointerHitbox = Hitbox2D(gui->GetPointerPos(), XMFLOAT2(1, 1));
		hitBox = Hitbox2D(XMFLOAT2(translation.x, translation.y), XMFLOAT2(scale.x, scale.y));

		if (GetState() != ASWIDGETSTATE::ACTIVE && !tooltip.empty() && pointerHitbox.intersects(hitBox))
		{
			tooltipTimer++;
		}
		else
		{
			tooltipTimer = 0;
		}

		UpdateTransform();

		if (parent != nullptr)
		{
			this->UpdateTransform_Parented(*parent, world_parent_bind);
		}

		XMVECTOR S, R, T;
		XMMatrixDecompose(&S, &R, &T, XMLoadFloat4x4(&world));
		XMStoreFloat3(&translation, T);
		XMStoreFloat3(&scale, S);

		scissorRect.bottom = (int32_t)(translation.y + scale.y);
		scissorRect.left = (int32_t)(translation.x);
		scissorRect.right = (int32_t)(translation.x + scale.x);
		scissorRect.top = (int32_t)(translation.y);
	}
	void asWidget::AttachTo(TransformComponent* parent)
	{
		this->parent = parent;

		this->parent->UpdateTransform();
		XMStoreFloat4x4(&world_parent_bind, XMMatrixInverse(nullptr, XMLoadFloat4x4(&parent->world)));
	}
	void asWidget::Detach()
	{
		this->parent = nullptr;
		ApplyTransform();
	}
	void asWidget::RenderTooltip(const asGUI* gui, CommandList cmd) const
	{
		if (!IsVisible())
		{
			return;
		}

		assert(gui != nullptr && "Ivalid GUI!");

		if (tooltipTimer > 25)
		{
			gui->ResetScissor(cmd);

			XMFLOAT2 tooltipPos = XMFLOAT2(gui->pointerpos.x, gui->pointerpos.y);
			if (tooltipPos.y > asRenderer::GetDevice()->GetScreenHeight() * 0.8f)
			{
				tooltipPos.y -= 30;
			}
			else
			{
				tooltipPos.y += 40;
			}
			asFontParams fontProps = asFontParams((int)tooltipPos.x, (int)tooltipPos.y, ASFONTSIZE_DEFAULT, WIFALIGN_LEFT, WIFALIGN_TOP);
			fontProps.color = asColor(25, 25, 25, 255);
			asFont tooltipFont = asFont(tooltip, fontProps);
			if (!scriptTip.empty())
			{
				tooltipFont.SetText(tooltip + "\n" + scriptTip);
			}

			int textWidth = tooltipFont.textWidth();
			if (tooltipPos.x > asRenderer::GetDevice()->GetScreenWidth() - textWidth)
			{
				tooltipPos.x -= textWidth + 10;
				tooltipFont.params.posX = (int)tooltipPos.x;
			}

			static const float _border = 2;
			float fontWidth = (float)tooltipFont.textWidth() + _border * 2;
			float fontHeight = (float)tooltipFont.textHeight() + _border * 2;
			asImage::Draw(asTextureHelper::getWhite(), asImageParams(tooltipPos.x - _border, tooltipPos.y - _border, fontWidth, fontHeight, asColor(255, 234, 165).toFloat4()), cmd);
			tooltipFont.SetText(tooltip);
			tooltipFont.Draw(cmd);
			if (!scriptTip.empty())
			{
				tooltipFont.SetText(scriptTip);
				tooltipFont.params.posY += (int)(fontHeight / 2);
				tooltipFont.params.color = asColor(25, 25, 25, 110);
				tooltipFont.Draw(cmd);
			}
		}
	}
	const asHashString& asWidget::GetName() const
	{
		return fastName;
	}
	void asWidget::SetName(const std::string& value)
	{
		if (value.length() <= 0)
		{
			static unsigned long widgetID = 0;
			stringstream ss("");
			ss << "widget_" << widgetID++;
			fastName = asHashString(ss.str());
		}
		else
		{
			fastName = asHashString(value);
		}

	}
	const string& asWidget::GetText() const
	{
		return text;
	}
	void asWidget::SetText(const std::string& value)
	{
		text = value;
	}
	void asWidget::SetTooltip(const std::string& value)
	{
		tooltip = value;
	}
	void asWidget::SetScriptTip(const std::string& value)
	{
		scriptTip = value;
	}
	void asWidget::SetPos(const XMFLOAT2& value)
	{
		SetDirty();
		translation_local.x = value.x;
		translation_local.y = value.y;
		UpdateTransform();

		translation = translation_local;
	}
	void asWidget::SetSize(const XMFLOAT2& value)
	{
		SetDirty();
		scale_local.x = value.x;
		scale_local.y = value.y;
		UpdateTransform();

		scale = scale_local;
	}
	asWidget::ASWIDGETSTATE asWidget::GetState() const
	{
		return state;
	}
	void asWidget::SetEnabled(bool val)
	{
		enabled = val;
	}
	bool asWidget::IsEnabled() const
	{
		return enabled && visible;
	}
	void asWidget::SetVisible(bool val)
	{
		visible = val;
	}
	bool asWidget::IsVisible() const
	{
		return visible;
	}
	void asWidget::Activate()
	{
		state = ACTIVE;
	}
	void asWidget::Deactivate()
	{
		state = DEACTIVATING;
	}
	void asWidget::SetColor(asColor color, ASWIDGETSTATE state)
	{
		if (state == ASWIDGETSTATE_COUNT)
		{
			for (int i = 0; i < ASWIDGETSTATE_COUNT; ++i)
			{
				colors[i] = color;
			}
		}
		else
		{
			colors[state] = color;
		}
	}
	asColor asWidget::GetColor() const
	{
		asColor retVal = colors[GetState()];
		if (!IsEnabled()) {
			retVal = asColor::lerp(asColor::Transparent(), retVal, 0.5f);
		}
		return retVal;
	}
	void asWidget::SetScissorRect(const asGraphics::Rect& rect)
	{
		scissorRect = rect;
		if (scissorRect.bottom > 0)
			scissorRect.bottom -= 1;
		if (scissorRect.left > 0)
			scissorRect.left += 1;
		if (scissorRect.right > 0)
			scissorRect.right -= 1;
		if (scissorRect.top > 0)
			scissorRect.top += 1;
	}
	void asWidget::LoadShaders()
	{
		PipelineStateDesc desc;
		desc.vs = asRenderer::GetVertexShader(VSTYPE_LINE);
		desc.ps = asRenderer::GetPixelShader(PSTYPE_LINE);
		desc.il = asRenderer::GetVertexLayout(VLTYPE_LINE);
		desc.dss = asRenderer::GetDepthStencilState(DSSTYPE_XRAY);
		desc.bs = asRenderer::GetBlendState(BSTYPE_TRANSPARENT);
		desc.rs = asRenderer::GetRasterizerState(RSTYPE_DOUBLESIDED);
		desc.pt = TRIANGLESTRIP;
		asRenderer::GetDevice()->CreatePipelineState(&desc, &PSO_colorpicker);
	}


	asButton::asButton(const std::string& name) :asWidget()
	{
		SetName(name);
		SetText(fastName.GetString());
		OnClick([](asEventArgs args) {});
		OnDragStart([](asEventArgs args) {});
		OnDrag([](asEventArgs args) {});
		OnDragEnd([](asEventArgs args) {});
		SetSize(XMFLOAT2(100, 30));
	}
	asButton::~asButton()
	{

	}
	void asButton::Update(asGUI* gui, float dt)
	{
		asWidget::Update(gui, dt);

		if (!IsEnabled())
		{
			return;
		}

		if (gui->IsWidgetDisabled(this))
		{
			return;
		}

		hitBox.pos.x = translation.x;
		hitBox.pos.y = translation.y;
		hitBox.siz.x = scale.x;
		hitBox.siz.y = scale.y;

		Hitbox2D pointerHitbox = Hitbox2D(gui->GetPointerPos(), XMFLOAT2(1, 1));

		if (state == FOCUS)
		{
			state = IDLE;
		}
		if (state == DEACTIVATING)
		{
			asEventArgs args;
			args.clickPos = pointerHitbox.pos;
			onDragEnd(args);

			if (pointerHitbox.intersects(hitBox))
			{
				// Click occurs when the button is released within the bounds
				onClick(args);
			}

			state = IDLE;
		}
		if (state == ACTIVE)
		{
			gui->DeactivateWidget(this);
		}

		bool clicked = false;
		// hover the button
		if (pointerHitbox.intersects(hitBox))
		{
			if (state == IDLE)
			{
				state = FOCUS;
			}
		}

		if (asInput::Press(asInput::MOUSE_BUTTON_LEFT))
		{
			if (state == FOCUS)
			{
				// activate
				clicked = true;
			}
		}

		if (asInput::Down(asInput::MOUSE_BUTTON_LEFT))
		{
			if (state == DEACTIVATING)
			{
				// Keep pressed until mouse is released
				gui->ActivateWidget(this);

				asEventArgs args;
				args.clickPos = pointerHitbox.pos;
				XMFLOAT3 posDelta;
				posDelta.x = pointerHitbox.pos.x - prevPos.x;
				posDelta.y = pointerHitbox.pos.y - prevPos.y;
				posDelta.z = 0;
				args.deltaPos = XMFLOAT2(posDelta.x, posDelta.y);
				onDrag(args);
			}
		}

		if (clicked)
		{
			asEventArgs args;
			args.clickPos = pointerHitbox.pos;
			dragStart = args.clickPos;
			args.startPos = dragStart;
			onDragStart(args);
			gui->ActivateWidget(this);
		}

		prevPos.x = pointerHitbox.pos.x;
		prevPos.y = pointerHitbox.pos.y;
	}
	void asButton::Render(const asGUI* gui, CommandList cmd) const
	{
		assert(gui != nullptr && "Ivalid GUI!");

		if (!IsVisible())
		{
			return;
		}

		asColor color = GetColor();

		gui->ResetScissor(cmd);

		asImage::Draw(asTextureHelper::getWhite()
			, asImageParams(translation.x, translation.y, scale.x, scale.y, color.toFloat4()), cmd);


		asRenderer::GetDevice()->BindScissorRects(1, &scissorRect, cmd);
		asFont(text, asFontParams((int)(translation.x + scale.x * 0.5f), (int)(translation.y + scale.y * 0.5f), ASFONTSIZE_DEFAULT, WIFALIGN_CENTER, WIFALIGN_CENTER, 0, 0,
			textColor, textShadowColor)).Draw(cmd);

	}
	void asButton::OnClick(function<void(asEventArgs args)> func)
	{
		onClick = move(func);
	}
	void asButton::OnDragStart(function<void(asEventArgs args)> func)
	{
		onDragStart = move(func);
	}
	void asButton::OnDrag(function<void(asEventArgs args)> func)
	{
		onDrag = move(func);
	}
	void asButton::OnDragEnd(function<void(asEventArgs args)> func)
	{
		onDragEnd = move(func);
	}




	asLabel::asLabel(const std::string& name)
	{
		SetName(name);
		SetText(fastName.GetString());
		SetSize(XMFLOAT2(100, 20));
	}
	asLabel::~asLabel()
	{

	}
	void asLabel::Update(asGUI* gui, float dt)
	{
		asWidget::Update(gui, dt);

		if (!IsEnabled())
		{
			return;
		}

		if (gui->IsWidgetDisabled(this))
		{
			return;
		}
	}
	void asLabel::Render(const asGUI* gui, CommandList cmd) const
	{
		assert(gui != nullptr && "Ivalid GUI!");

		if (!IsVisible())
		{
			return;
		}

		asColor color = GetColor();

		gui->ResetScissor(cmd);

		asImage::Draw(asTextureHelper::getWhite()
			, asImageParams(translation.x, translation.y, scale.x, scale.y, color.toFloat4()), cmd);


		asRenderer::GetDevice()->BindScissorRects(1, &scissorRect, cmd);
		asFont(text, asFontParams((int)translation.x + 2, (int)translation.y + 2, ASFONTSIZE_DEFAULT, WIFALIGN_LEFT, WIFALIGN_TOP, 0, 0,
			textColor, textShadowColor)).Draw(cmd);

	}




	string asTextInputField::value_new = "";
	asTextInputField::asTextInputField(const std::string& name)
	{
		SetName(name);
		SetText(fastName.GetString());
		OnInputAccepted([](asEventArgs args) {});
		SetSize(XMFLOAT2(100, 30));
	}
	asTextInputField::~asTextInputField()
	{

	}
	void asTextInputField::SetValue(const std::string& newValue)
	{
		value = newValue;
	}
	void asTextInputField::SetValue(int newValue)
	{
		stringstream ss("");
		ss << newValue;
		value = ss.str();
	}
	void asTextInputField::SetValue(float newValue)
	{
		stringstream ss("");
		ss << newValue;
		value = ss.str();
	}
	const std::string& asTextInputField::GetValue()
	{
		return value;
	}
	void asTextInputField::Update(asGUI* gui, float dt)
	{
		asWidget::Update(gui, dt);

		if (!IsEnabled())
		{
			return;
		}

		if (gui->IsWidgetDisabled(this))
		{
			return;
		}

		hitBox.pos.x = translation.x;
		hitBox.pos.y = translation.y;
		hitBox.siz.x = scale.x;
		hitBox.siz.y = scale.y;

		Hitbox2D pointerHitbox = Hitbox2D(gui->GetPointerPos(), XMFLOAT2(1, 1));
		bool intersectsPointer = pointerHitbox.intersects(hitBox);

		if (state == FOCUS)
		{
			state = IDLE;
		}
		if (state == DEACTIVATING)
		{
			state = IDLE;
		}

		bool clicked = false;
		// hover the button
		if (intersectsPointer)
		{
			if (state == IDLE)
			{
				state = FOCUS;
			}
		}

		if (asInput::Press(asInput::MOUSE_BUTTON_LEFT))
		{
			if (state == FOCUS)
			{
				// activate
				clicked = true;
			}
		}

		if (asInput::Down(asInput::MOUSE_BUTTON_LEFT))
		{
			if (state == DEACTIVATING)
			{
				// Keep pressed until mouse is released
				gui->ActivateWidget(this);
			}
		}

		if (clicked)
		{
			gui->ActivateWidget(this);

			value_new = value;
		}

		if (state == ACTIVE)
		{
			if (asInput::Press(asInput::KEYBOARD_BUTTON_ENTER))
			{
				// accept input...

				value = value_new;
				value_new.clear();

				asEventArgs args;
				args.sValue = value;
				args.iValue = atoi(value.c_str());
				args.fValue = (float)atof(value.c_str());
				onInputAccepted(args);

				gui->DeactivateWidget(this);
			}
			else if ((asInput::Press(asInput::MOUSE_BUTTON_LEFT) && !intersectsPointer) ||
				asInput::Press(asInput::KEYBOARD_BUTTON_ESCAPE))
			{
				// cancel input 
				value_new.clear();
				gui->DeactivateWidget(this);
			}

		}
	}
	void asTextInputField::Render(const asGUI* gui, CommandList cmd) const
	{
		assert(gui != nullptr && "Ivalid GUI!");

		if (!IsVisible())
		{
			return;
		}

		asColor color = GetColor();

		gui->ResetScissor(cmd);

		asImage::Draw(asTextureHelper::getWhite()
			, asImageParams(translation.x, translation.y, scale.x, scale.y, color.toFloat4()), cmd);



		asRenderer::GetDevice()->BindScissorRects(1, &scissorRect, cmd);

		string activeText = text;
		if (state == ACTIVE)
		{
			activeText = value_new;
		}
		else if (!value.empty())
		{
			activeText = value;
		}
		asFont(activeText, asFontParams((int)(translation.x + 2), (int)(translation.y + scale.y * 0.5f), ASFONTSIZE_DEFAULT, WIFALIGN_LEFT, WIFALIGN_CENTER, 0, 0,
			textColor, textShadowColor)).Draw(cmd);

	}
	void asTextInputField::OnInputAccepted(function<void(asEventArgs args)> func)
	{
		onInputAccepted = move(func);
	}
	void asTextInputField::AddInput(const char inputChar)
	{
		value_new.push_back(inputChar);
	}
	void asTextInputField::DeleteFromInput()
	{
		if (!value_new.empty())
		{
			value_new.pop_back();
		}
	}





	asSlider::asSlider(float start, float end, float defaultValue, float step, const std::string& name) : start(start), end(end), value(defaultValue), step(std::max(step, 1.0f))
	{
		SetName(name);
		SetText(fastName.GetString());
		OnSlide([](asEventArgs args) {});
		SetSize(XMFLOAT2(200, 40));

		valueInputField = new asTextInputField(name + "_endInputField");
		valueInputField->SetSize(XMFLOAT2(scale.y * 2, scale.y));
		valueInputField->SetPos(XMFLOAT2(scale.x + 20, 0));
		valueInputField->SetValue(end);
		valueInputField->OnInputAccepted([&](asEventArgs args) {
			this->value = args.fValue;
			this->start = std::min(this->start, args.fValue);
			this->end = std::max(this->end, args.fValue);
			onSlide(args);
			});
		valueInputField->parent = this;
		valueInputField->AttachTo(this);
	}
	asSlider::~asSlider()
	{
		SAFE_DELETE(valueInputField);
	}
	void asSlider::SetValue(float value)
	{
		this->value = value;
	}
	float asSlider::GetValue()
	{
		return value;
	}
	void asSlider::SetRange(float start, float end)
	{
		this->start = start;
		this->end = end;
		this->value = asMath::Clamp(this->value, start, end);
	}
	void asSlider::Update(asGUI* gui, float dt)
	{
		asWidget::Update(gui, dt);

		for (int i = 0; i < ASWIDGETSTATE_COUNT; ++i)
		{
			valueInputField->SetColor(this->colors[i], (ASWIDGETSTATE)i);
		}
		valueInputField->SetTextColor(this->textColor);
		valueInputField->SetTextShadowColor(this->textShadowColor);
		valueInputField->SetEnabled(IsEnabled());
		valueInputField->Update(gui, dt);

		if (!IsEnabled())
		{
			return;
		}

		if (gui->IsWidgetDisabled(this))
		{
			return;
		}

		bool dragged = false;

		if (state == FOCUS)
		{
			state = IDLE;
		}
		if (state == DEACTIVATING)
		{
			state = IDLE;
		}
		if (state == ACTIVE)
		{
			if (asInput::Down(asInput::MOUSE_BUTTON_LEFT))
			{
				if (state == ACTIVE)
				{
					// continue drag if already grabbed wheter it is intersecting or not
					dragged = true;
				}
			}
			else
			{
				gui->DeactivateWidget(this);
			}
		}

		float headWidth = scale.x * 0.05f;

		hitBox.pos.x = translation.x - headWidth * 0.5f;
		hitBox.pos.y = translation.y;
		hitBox.siz.x = scale.x + headWidth;
		hitBox.siz.y = scale.y;

		Hitbox2D pointerHitbox = Hitbox2D(gui->GetPointerPos(), XMFLOAT2(1, 1));


		if (pointerHitbox.intersects(hitBox))
		{
			// hover the slider
			if (state == IDLE)
			{
				state = FOCUS;
			}
		}

		if (asInput::Press(asInput::MOUSE_BUTTON_LEFT))
		{
			if (state == FOCUS)
			{
				// activate
				dragged = true;
			}
		}


		if (dragged)
		{
			asEventArgs args;
			args.clickPos = pointerHitbox.pos;
			value = asMath::InverseLerp(translation.x, translation.x + scale.x, args.clickPos.x);
			value = asMath::Clamp(value, 0, 1);
			value *= step;
			value = floorf(value);
			value /= step;
			value = asMath::Lerp(start, end, value);
			args.fValue = value;
			args.iValue = (int)value;
			onSlide(args);
			gui->ActivateWidget(this);
		}

		valueInputField->SetValue(value);
	}
	void asSlider::Render(const asGUI* gui, CommandList cmd) const
	{
		assert(gui != nullptr && "Ivalid GUI!");

		if (!IsVisible())
		{
			return;
		}

		asColor color = GetColor();

		float headWidth = scale.x * 0.05f;

		gui->ResetScissor(cmd);

		// trail
		asImage::Draw(asTextureHelper::getWhite()
			, asImageParams(translation.x - headWidth * 0.5f, translation.y + scale.y * 0.5f - scale.y * 0.1f, scale.x + headWidth, scale.y * 0.2f, color.toFloat4()), cmd);
		// head
		float headPosX = asMath::Lerp(translation.x, translation.x + scale.x, asMath::Clamp(asMath::InverseLerp(start, end, value), 0, 1));
		asImage::Draw(asTextureHelper::getWhite()
			, asImageParams(headPosX - headWidth * 0.5f, translation.y, headWidth, scale.y, color.toFloat4()), cmd);

		if (parent != gui)
		{
			asRenderer::GetDevice()->BindScissorRects(1, &scissorRect, cmd);
		}
		// text
		asFont(text, asFontParams((int)(translation.x - headWidth * 0.5f), (int)(translation.y + scale.y * 0.5f), ASFONTSIZE_DEFAULT, WIFALIGN_RIGHT, WIFALIGN_CENTER, 0, 0,
			textColor, textShadowColor)).Draw(cmd);

		//// value
		//stringstream ss("");
		//ss << value;
		//asFont(ss.str(), asFontParams((int)(translation.x + scale.x + headWidth), (int)(translation.y + scale.y*0.5f), -1, WIFALIGN_LEFT, WIFALIGN_CENTER, 0, 0,
		//	textColor, textShadowColor )).Draw(gui->GetGraphicsThread(), parent != nullptr);



		valueInputField->Render(gui, cmd);
	}
	void asSlider::OnSlide(function<void(asEventArgs args)> func)
	{
		onSlide = move(func);
	}





	asCheckBox::asCheckBox(const std::string& name)
	{
		SetName(name);
		SetText(fastName.GetString());
		OnClick([](asEventArgs args) {});
		SetSize(XMFLOAT2(20, 20));
	}
	asCheckBox::~asCheckBox()
	{

	}
	void asCheckBox::Update(asGUI* gui, float dt)
	{
		asWidget::Update(gui, dt);

		if (!IsEnabled())
		{
			return;
		}

		if (gui->IsWidgetDisabled(this))
		{
			return;
		}

		if (state == FOCUS)
		{
			state = IDLE;
		}
		if (state == DEACTIVATING)
		{
			state = IDLE;
		}
		if (state == ACTIVE)
		{
			gui->DeactivateWidget(this);
		}

		hitBox.pos.x = translation.x;
		hitBox.pos.y = translation.y;
		hitBox.siz.x = scale.x;
		hitBox.siz.y = scale.y;

		Hitbox2D pointerHitbox = Hitbox2D(gui->GetPointerPos(), XMFLOAT2(1, 1));

		bool clicked = false;
		// hover the button
		if (pointerHitbox.intersects(hitBox))
		{
			if (state == IDLE)
			{
				state = FOCUS;
			}
		}

		if (asInput::Press(asInput::MOUSE_BUTTON_LEFT))
		{
			if (state == FOCUS)
			{
				// activate
				clicked = true;
			}
		}

		if (asInput::Down(asInput::MOUSE_BUTTON_LEFT))
		{
			if (state == DEACTIVATING)
			{
				// Keep pressed until mouse is released
				gui->ActivateWidget(this);
			}
		}

		if (clicked)
		{
			SetCheck(!GetCheck());
			asEventArgs args;
			args.clickPos = pointerHitbox.pos;
			args.bValue = GetCheck();
			onClick(args);
			gui->ActivateWidget(this);
		}

	}
	void asCheckBox::Render(const asGUI* gui, CommandList cmd) const
	{
		assert(gui != nullptr && "Ivalid GUI!");

		if (!IsVisible())
		{
			return;
		}

		asColor color = GetColor();

		gui->ResetScissor(cmd);

		// control
		asImage::Draw(asTextureHelper::getWhite()
			, asImageParams(translation.x, translation.y, scale.x, scale.y, color.toFloat4()), cmd);

		// check
		if (GetCheck())
		{
			asImage::Draw(asTextureHelper::getWhite()
				, asImageParams(translation.x + scale.x * 0.25f, translation.y + scale.y * 0.25f, scale.x * 0.5f, scale.y * 0.5f, asColor::lerp(color, asColor::White(), 0.8f).toFloat4())
				, cmd);
		}

		if (parent != gui)
		{
			asRenderer::GetDevice()->BindScissorRects(1, &scissorRect, cmd);
		}
		asFont(text, asFontParams((int)(translation.x), (int)(translation.y + scale.y * 0.5f), ASFONTSIZE_DEFAULT, WIFALIGN_RIGHT, WIFALIGN_CENTER, 0, 0,
			textColor, textShadowColor)).Draw(cmd);

	}
	void asCheckBox::OnClick(function<void(asEventArgs args)> func)
	{
		onClick = move(func);
	}
	void asCheckBox::SetCheck(bool value)
	{
		checked = value;
	}
	bool asCheckBox::GetCheck() const
	{
		return checked;
	}





	asComboBox::asComboBox(const std::string& name)
	{
		SetName(name);
		SetText(fastName.GetString());
		OnSelect([](asEventArgs args) {});
		SetSize(XMFLOAT2(100, 20));
	}
	asComboBox::~asComboBox()
	{

	}
	float asComboBox::GetItemOffset(int index) const
	{
		index = std::max(firstItemVisible, index) - firstItemVisible;
		return scale.y * (index + 1) + 1;
	}
	bool asComboBox::HasScrollbar() const
	{
		return maxVisibleItemCount < (int)items.size();
	}
	void asComboBox::Update(asGUI* gui, float dt)
	{
		asWidget::Update(gui, dt);

		if (!IsEnabled())
		{
			return;
		}

		if (gui->IsWidgetDisabled(this))
		{
			return;
		}

		if (state == FOCUS)
		{
			state = IDLE;
		}
		if (state == DEACTIVATING)
		{
			state = IDLE;
		}
		if (state == ACTIVE && combostate == COMBOSTATE_SELECTING)
		{
			hovered = -1;
			gui->DeactivateWidget(this);
		}
		if (state == IDLE && combostate == COMBOSTATE_SELECTING)
		{
			combostate = COMBOSTATE_INACTIVE;
		}

		hitBox.pos.x = translation.x;
		hitBox.pos.y = translation.y;
		hitBox.siz.x = scale.x + scale.y + 1; // + drop-down indicator arrow + little offset
		hitBox.siz.y = scale.y;

		Hitbox2D pointerHitbox = Hitbox2D(gui->GetPointerPos(), XMFLOAT2(1, 1));

		bool clicked = false;
		// hover the button
		if (pointerHitbox.intersects(hitBox))
		{
			if (state == IDLE)
			{
				state = FOCUS;
			}
		}

		if (asInput::Press(asInput::MOUSE_BUTTON_LEFT))
		{
			// activate
			clicked = true;
		}

		bool click_down = false;
		if (asInput::Down(asInput::MOUSE_BUTTON_LEFT))
		{
			click_down = true;
			if (state == DEACTIVATING)
			{
				// Keep pressed until mouse is released
				gui->ActivateWidget(this);
			}
		}


		if (clicked && state == FOCUS)
		{
			gui->ActivateWidget(this);
		}

		const float scrollbar_begin = translation.y + scale.y + 1 + scale.y * 0.5f;
		const float scrollbar_end = scrollbar_begin + std::max(0.0f, (float)std::min(maxVisibleItemCount, (int)items.size()) - 1) * scale.y;

		if (state == ACTIVE)
		{
			if (HasScrollbar())
			{
				if (combostate != COMBOSTATE_SELECTING && combostate != COMBOSTATE_INACTIVE)
				{
					if (combostate == COMBOSTATE_SCROLLBAR_GRABBED || pointerHitbox.intersects(Hitbox2D(XMFLOAT2(translation.x + scale.x + 1, translation.y + scale.y + 1), XMFLOAT2(scale.y, (float)std::min(maxVisibleItemCount, (int)items.size()) * scale.y))))
					{
						if (click_down)
						{
							combostate = COMBOSTATE_SCROLLBAR_GRABBED;
							scrollbar_delta = asMath::Clamp(pointerHitbox.pos.y, scrollbar_begin, scrollbar_end) - scrollbar_begin;
							const float scrollbar_value = asMath::InverseLerp(scrollbar_begin, scrollbar_end, scrollbar_begin + scrollbar_delta);
							firstItemVisible = int(float(std::max(0, (int)items.size() - maxVisibleItemCount)) * scrollbar_value + 0.5f);
							firstItemVisible = std::max(0, std::min((int)items.size() - maxVisibleItemCount, firstItemVisible));
						}
						else
						{
							combostate = COMBOSTATE_SCROLLBAR_HOVER;
						}
					}
					else if (!click_down)
					{
						combostate = COMBOSTATE_HOVER;
					}
				}
			}

			if (combostate == COMBOSTATE_INACTIVE)
			{
				combostate = COMBOSTATE_HOVER;
			}
			else if (combostate == COMBOSTATE_SELECTING)
			{
				gui->DeactivateWidget(this);
				combostate = COMBOSTATE_INACTIVE;
			}
			else if (combostate == COMBOSTATE_HOVER)
			{
				int scroll = (int)asInput::GetPointer().z;
				firstItemVisible -= scroll;
				firstItemVisible = std::max(0, std::min((int)items.size() - maxVisibleItemCount, firstItemVisible));
				if (scroll)
				{
					const float scrollbar_value = asMath::InverseLerp(0, float(std::max(0, (int)items.size() - maxVisibleItemCount)), float(firstItemVisible));
					scrollbar_delta = asMath::Lerp(scrollbar_begin, scrollbar_end, scrollbar_value) - scrollbar_begin;
				}

				hovered = -1;
				for (int i = firstItemVisible; i < (firstItemVisible + std::min(maxVisibleItemCount, (int)items.size())); ++i)
				{
					Hitbox2D itembox;
					itembox.pos.x = translation.x;
					itembox.pos.y = translation.y + GetItemOffset(i);
					itembox.siz.x = scale.x;
					itembox.siz.y = scale.y;
					if (pointerHitbox.intersects(itembox))
					{
						hovered = i;
						break;
					}
				}

				if (clicked)
				{
					combostate = COMBOSTATE_SELECTING;
					if (hovered >= 0)
					{
						SetSelected(hovered);
					}
				}
			}
		}

	}
	void asComboBox::Render(const asGUI* gui, CommandList cmd) const
	{
		assert(gui != nullptr && "Ivalid GUI!");

		if (!IsVisible())
		{
			return;
		}

		asColor color = GetColor();
		if (combostate != COMBOSTATE_INACTIVE)
		{
			color = colors[FOCUS];
		}

		gui->ResetScissor(cmd);

		// control-base
		asImage::Draw(asTextureHelper::getWhite()
			, asImageParams(translation.x, translation.y, scale.x, scale.y, color.toFloat4()), cmd);

		// control-arrow
		asImage::Draw(asTextureHelper::getWhite()
			, asImageParams(translation.x + scale.x + 1, translation.y, scale.y, scale.y, color.toFloat4()), cmd);
		asFont("V", asFontParams((int)(translation.x + scale.x + scale.y * 0.5f), (int)(translation.y + scale.y * 0.5f), ASFONTSIZE_DEFAULT, WIFALIGN_CENTER, WIFALIGN_CENTER, 0, 0,
			textColor, textShadowColor)).Draw(cmd);

		if (parent != gui)
		{
			asRenderer::GetDevice()->BindScissorRects(1, &scissorRect, cmd);
		}
		asFont(text, asFontParams((int)(translation.x), (int)(translation.y + scale.y * 0.5f), ASFONTSIZE_DEFAULT, WIFALIGN_RIGHT, WIFALIGN_CENTER, 0, 0,
			textColor, textShadowColor)).Draw(cmd);

		if (selected >= 0)
		{
			asFont(items[selected], asFontParams((int)(translation.x + scale.x * 0.5f), (int)(translation.y + scale.y * 0.5f), ASFONTSIZE_DEFAULT, WIFALIGN_CENTER, WIFALIGN_CENTER, 0, 0,
				textColor, textShadowColor)).Draw(cmd);
		}

		// drop-down
		if (state == ACTIVE)
		{
			if (HasScrollbar())
			{
				// control-scrollbar-base
				{
					asColor col = colors[IDLE];
					col.setA(col.getA() / 2);
					asImage::Draw(asTextureHelper::getWhite()
						, asImageParams(translation.x + scale.x + 1, translation.y + scale.y + 1, scale.y, scale.y * maxVisibleItemCount, col.toFloat4()), cmd);
				}

				// control-scrollbar-grab
				{
					asColor col = colors[IDLE];
					if (combostate == COMBOSTATE_SCROLLBAR_HOVER)
					{
						col = colors[FOCUS];
					}
					else if (combostate == COMBOSTATE_SCROLLBAR_GRABBED)
					{
						col = colors[ACTIVE];
					}
					asImage::Draw(asTextureHelper::getWhite()
						, asImageParams(translation.x + scale.x + 1, translation.y + scale.y + 1 + scrollbar_delta, scale.y, scale.y, col.toFloat4()), cmd);
				}
			}

			gui->ResetScissor(cmd);

			// control-list
			for (int i = firstItemVisible; i < (firstItemVisible + std::min(maxVisibleItemCount, (int)items.size())); ++i)
			{
				asColor col = colors[IDLE];
				if (hovered == i)
				{
					if (combostate == COMBOSTATE_HOVER)
					{
						col = colors[FOCUS];
					}
					else if (combostate == COMBOSTATE_SELECTING)
					{
						col = colors[ACTIVE];
					}
				}
				asImage::Draw(asTextureHelper::getWhite()
					, asImageParams(translation.x, translation.y + GetItemOffset(i), scale.x, scale.y, col.toFloat4()), cmd);
				asFont(items[i], asFontParams((int)(translation.x + scale.x * 0.5f), (int)(translation.y + scale.y * 0.5f + GetItemOffset(i)), ASFONTSIZE_DEFAULT, WIFALIGN_CENTER, WIFALIGN_CENTER, 0, 0,
					textColor, textShadowColor)).Draw(cmd);
			}
		}
	}
	void asComboBox::OnSelect(function<void(asEventArgs args)> func)
	{
		onSelect = move(func);
	}
	void asComboBox::AddItem(const std::string& item)
	{
		items.push_back(item);

		if (selected < 0)
		{
			selected = 0;
		}
	}
	void asComboBox::RemoveItem(int index)
	{
		std::vector<string> newItems(0);
		newItems.reserve(items.size());
		for (size_t i = 0; i < items.size(); ++i)
		{
			if (i != index)
			{
				newItems.push_back(items[i]);
			}
		}
		items = newItems;

		if (items.empty())
		{
			selected = -1;
		}
		else if (selected > index)
		{
			selected--;
		}
	}
	void asComboBox::ClearItems()
	{
		items.clear();

		selected = -1;
	}
	void asComboBox::SetMaxVisibleItemCount(int value)
	{
		maxVisibleItemCount = value;
	}
	void asComboBox::SetSelected(int index)
	{
		selected = index;

		asEventArgs args;
		args.iValue = selected;
		args.sValue = GetItemText(selected);
		onSelect(args);
	}
	string asComboBox::GetItemText(int index)
	{
		if (index >= 0)
		{
			return items[index];
		}
		return "";
	}
	int asComboBox::GetSelected()
	{
		return selected;
	}






	static const float windowcontrolSize = 20.0f;
	asWindow::asWindow(asGUI* gui, const std::string& name) : gui(gui)
	{
		assert(gui != nullptr && "Ivalid GUI!");

		SetName(name);
		SetText(fastName.GetString());
		SetSize(XMFLOAT2(640, 480));

		// Add controls

		// Add a grabber onto the title bar
		moveDragger = new asButton(name + "_move_dragger");
		moveDragger->SetText("");
		moveDragger->SetSize(XMFLOAT2(scale.x - windowcontrolSize * 3, windowcontrolSize));
		moveDragger->SetPos(XMFLOAT2(windowcontrolSize, 0));
		moveDragger->OnDrag([this, gui](asEventArgs args) {
			this->Translate(XMFLOAT3(args.deltaPos.x, args.deltaPos.y, 0));
			this->asWidget::Update(gui, 0);
			for (auto& x : this->childrenWidgets)
			{
				x->asWidget::Update(gui, 0);
			}
			});
		AddWidget(moveDragger);

		// Add close button to the top right corner
		closeButton = new asButton(name + "_close_button");
		closeButton->SetText("x");
		closeButton->SetSize(XMFLOAT2(windowcontrolSize, windowcontrolSize));
		closeButton->SetPos(XMFLOAT2(translation.x + scale.x - windowcontrolSize, translation.y));
		closeButton->OnClick([this](asEventArgs args) {
			this->SetVisible(false);
			});
		closeButton->SetTooltip("Close window");
		AddWidget(closeButton);

		// Add minimize button to the top right corner
		minimizeButton = new asButton(name + "_minimize_button");
		minimizeButton->SetText("-");
		minimizeButton->SetSize(XMFLOAT2(windowcontrolSize, windowcontrolSize));
		minimizeButton->SetPos(XMFLOAT2(translation.x + scale.x - windowcontrolSize * 2, translation.y));
		minimizeButton->OnClick([this](asEventArgs args) {
			this->SetMinimized(!this->IsMinimized());
			});
		minimizeButton->SetTooltip("Minimize window");
		AddWidget(minimizeButton);

		// Add a resizer control to the upperleft corner
		resizeDragger_UpperLeft = new asButton(name + "_resize_dragger_upper_left");
		resizeDragger_UpperLeft->SetText("");
		resizeDragger_UpperLeft->SetSize(XMFLOAT2(windowcontrolSize, windowcontrolSize));
		resizeDragger_UpperLeft->SetPos(XMFLOAT2(0, 0));
		resizeDragger_UpperLeft->OnDrag([this, gui](asEventArgs args) {
			XMFLOAT2 scaleDiff;
			scaleDiff.x = (scale.x - args.deltaPos.x) / scale.x;
			scaleDiff.y = (scale.y - args.deltaPos.y) / scale.y;
			this->Translate(XMFLOAT3(args.deltaPos.x, args.deltaPos.y, 0));
			this->Scale(XMFLOAT3(scaleDiff.x, scaleDiff.y, 1));
			this->asWidget::Update(gui, 0);
			for (auto& x : this->childrenWidgets)
			{
				x->asWidget::Update(gui, 0);
			}
			});
		AddWidget(resizeDragger_UpperLeft);

		// Add a resizer control to the bottom right corner
		resizeDragger_BottomRight = new asButton(name + "_resize_dragger_bottom_right");
		resizeDragger_BottomRight->SetText("");
		resizeDragger_BottomRight->SetSize(XMFLOAT2(windowcontrolSize, windowcontrolSize));
		resizeDragger_BottomRight->SetPos(XMFLOAT2(translation.x + scale.x - windowcontrolSize, translation.y + scale.y - windowcontrolSize));
		resizeDragger_BottomRight->OnDrag([this, gui](asEventArgs args) {
			XMFLOAT2 scaleDiff;
			scaleDiff.x = (scale.x + args.deltaPos.x) / scale.x;
			scaleDiff.y = (scale.y + args.deltaPos.y) / scale.y;
			this->Scale(XMFLOAT3(scaleDiff.x, scaleDiff.y, 1));
			this->asWidget::Update(gui, 0);
			for (auto& x : this->childrenWidgets)
			{
				x->asWidget::Update(gui, 0);
			}
			});
		AddWidget(resizeDragger_BottomRight);


		SetEnabled(true);
		SetVisible(true);
		SetMinimized(false);
	}
	asWindow::~asWindow()
	{
		RemoveWidgets(true);
	}
	void asWindow::AddWidget(asWidget* widget)
	{
		assert(gui != nullptr && "Ivalid GUI!");

		widget->SetEnabled(this->IsEnabled());
		widget->SetVisible(this->IsVisible());
		gui->AddWidget(widget);
		widget->AttachTo(this);

		childrenWidgets.push_back(widget);
	}
	void asWindow::RemoveWidget(asWidget* widget)
	{
		assert(gui != nullptr && "Ivalid GUI!");

		gui->RemoveWidget(widget);
		//widget->detach();

		childrenWidgets.remove(widget);
	}
	void asWindow::RemoveWidgets(bool alsoDelete)
	{
		assert(gui != nullptr && "Ivalid GUI!");

		for (auto& x : childrenWidgets)
		{
			//x->detach();
			gui->RemoveWidget(x);
			if (alsoDelete)
			{
				SAFE_DELETE(x);
			}
		}

		childrenWidgets.clear();
	}
	void asWindow::Update(asGUI* gui, float dt)
	{
		asWidget::Update(gui, dt);

		//// TODO: Override window controls for nicer alignment:
		//if (moveDragger != nullptr)
		//{
		//	moveDragger->scale.y = windowcontrolSize;
		//}
		//if (resizeDragger_UpperLeft != nullptr)
		//{
		//	resizeDragger_UpperLeft->scale.x = windowcontrolSize;
		//	resizeDragger_UpperLeft->scale.y = windowcontrolSize;
		//}
		//if (resizeDragger_BottomRight != nullptr)
		//{
		//	resizeDragger_BottomRight->scale.x = windowcontrolSize;
		//	resizeDragger_BottomRight->scale.y = windowcontrolSize;
		//}
		//if (closeButton != nullptr)
		//{
		//	closeButton->scale.x = windowcontrolSize;
		//	closeButton->scale.y = windowcontrolSize;
		//}
		//if (minimizeButton != nullptr)
		//{
		//	minimizeButton->scale.x = windowcontrolSize;
		//	minimizeButton->scale.y = windowcontrolSize;
		//}

		for (auto& x : childrenWidgets)
		{
			x->Update(gui, dt);
			x->SetScissorRect(scissorRect);
		}

		if (!IsEnabled())
		{
			return;
		}

		if (gui->IsWidgetDisabled(this))
		{
			return;
		}
	}
	void asWindow::Render(const asGUI* gui, CommandList cmd) const
	{
		assert(gui != nullptr && "Ivalid GUI!");

		if (!IsVisible())
		{
			return;
		}

		asColor color = GetColor();

		gui->ResetScissor(cmd);

		// body
		if (!IsMinimized())
		{
			asImage::Draw(asTextureHelper::getWhite()
				, asImageParams(translation.x, translation.y, scale.x, scale.y, color.toFloat4()), cmd);
		}

		for (auto& x : childrenWidgets)
		{
			if (x != gui->GetActiveWidget())
			{
				// the gui will render the active on on top of everything!
				x->Render(gui, cmd);
			}
		}

		asRenderer::GetDevice()->BindScissorRects(1, &scissorRect, cmd);
		asFont(text, asFontParams((int)(translation.x + resizeDragger_UpperLeft->scale.x + 2), (int)(translation.y + resizeDragger_UpperLeft->scale.y * 0.5f), ASFONTSIZE_DEFAULT, WIFALIGN_LEFT, WIFALIGN_CENTER, 0, 0,
			textColor, textShadowColor)).Draw(cmd);

	}
	void asWindow::SetVisible(bool value)
	{
		asWidget::SetVisible(value);
		SetMinimized(!value);
		for (auto& x : childrenWidgets)
		{
			x->SetVisible(value);
		}
	}
	void asWindow::SetEnabled(bool value)
	{
		asWidget::SetEnabled(value);
		for (auto& x : childrenWidgets)
		{
			if (x == moveDragger)
				continue;
			if (x == minimizeButton)
				continue;
			if (x == closeButton)
				continue;
			if (x == resizeDragger_UpperLeft)
				continue;
			x->SetEnabled(value);
		}
	}
	void asWindow::SetMinimized(bool value)
	{
		minimized = value;

		if (resizeDragger_BottomRight != nullptr)
		{
			resizeDragger_BottomRight->SetVisible(!value);
		}
		for (auto& x : childrenWidgets)
		{
			if (x == moveDragger)
				continue;
			if (x == minimizeButton)
				continue;
			if (x == closeButton)
				continue;
			if (x == resizeDragger_UpperLeft)
				continue;
			x->SetVisible(!value);
		}
	}
	bool asWindow::IsMinimized() const
	{
		return minimized;
	}




	struct rgb {
		float r;       // a fraction between 0 and 1
		float g;       // a fraction between 0 and 1
		float b;       // a fraction between 0 and 1
	};

	struct hsv {
		float h;       // angle in degrees
		float s;       // a fraction between 0 and 1
		float v;       // a fraction between 0 and 1
	};

	hsv rgb2hsv(rgb in)
	{
		hsv         out;
		float		min, max, delta;

		min = in.r < in.g ? in.r : in.g;
		min = min < in.b ? min : in.b;

		max = in.r > in.g ? in.r : in.g;
		max = max > in.b ? max : in.b;

		out.v = max;                                // v
		delta = max - min;
		if (delta < 0.00001f)
		{
			out.s = 0;
			out.h = 0; // undefined, maybe nan?
			return out;
		}
		if (max > 0.0f) { // NOTE: if Max is == 0, this divide would cause a crash
			out.s = (delta / max);                  // s
		}
		else {
			// if max is 0, then r = g = b = 0              
			// s = 0, h is undefined
			out.s = 0.0f;
			out.h = NAN;                            // its now undefined
			return out;
		}
		if (in.r >= max)                           // > is bogus, just keeps compilor happy
			out.h = (in.g - in.b) / delta;        // between yellow & magenta
		else
			if (in.g >= max)
				out.h = 2.0f + (in.b - in.r) / delta;  // between cyan & yellow
			else
				out.h = 4.0f + (in.r - in.g) / delta;  // between magenta & cyan

		out.h *= 60.0f;                              // degrees

		if (out.h < 0.0f)
			out.h += 360.0f;

		return out;
	}


	rgb hsv2rgb(hsv in)
	{
		float		hh, p, q, t, ff;
		long        i;
		rgb         out;

		if (in.s <= 0.0f) {       // < is bogus, just shuts up warnings
			out.r = in.v;
			out.g = in.v;
			out.b = in.v;
			return out;
		}
		hh = in.h;
		if (hh >= 360.0f) hh = 0.0f;
		hh /= 60.0f;
		i = (long)hh;
		ff = hh - i;
		p = in.v * (1.0f - in.s);
		q = in.v * (1.0f - (in.s * ff));
		t = in.v * (1.0f - (in.s * (1.0f - ff)));

		switch (i) {
		case 0:
			out.r = in.v;
			out.g = t;
			out.b = p;
			break;
		case 1:
			out.r = q;
			out.g = in.v;
			out.b = p;
			break;
		case 2:
			out.r = p;
			out.g = in.v;
			out.b = t;
			break;

		case 3:
			out.r = p;
			out.g = q;
			out.b = in.v;
			break;
		case 4:
			out.r = t;
			out.g = p;
			out.b = in.v;
			break;
		case 5:
		default:
			out.r = in.v;
			out.g = p;
			out.b = q;
			break;
		}
		return out;
	}

	asColorPicker::asColorPicker(asGUI* gui, const std::string& name) :asWindow(gui, name)
	{
		SetSize(XMFLOAT2(300, 260));
		SetColor(asColor::Ghost());
		RemoveWidget(resizeDragger_BottomRight);
		RemoveWidget(resizeDragger_UpperLeft);
	}
	static const float colorpicker_center = 120;
	static const float colorpicker_radius_triangle = 68;
	static const float colorpicker_radius = 75;
	static const float colorpicker_width = 22;
	void asColorPicker::Update(asGUI* gui, float dt)
	{
		asWindow::Update(gui, dt);

		if (!IsEnabled())
		{
			return;
		}

		//if (!gui->IsWidgetDisabled(this))
		//{
		//	return;
		//}

		if (state == DEACTIVATING)
		{
			state = IDLE;
		}

		XMFLOAT2 center = XMFLOAT2(translation.x + colorpicker_center, translation.y + colorpicker_center);
		XMFLOAT2 pointer = gui->GetPointerPos();
		float distance = asMath::Distance(center, pointer);
		bool hover_hue = (distance > colorpicker_radius) && (distance < colorpicker_radius + colorpicker_width);

		float distTri = 0;
		XMFLOAT4 A, B, C;
		asMath::ConstructTriangleEquilateral(colorpicker_radius_triangle, A, B, C);
		XMVECTOR _A = XMLoadFloat4(&A);
		XMVECTOR _B = XMLoadFloat4(&B);
		XMVECTOR _C = XMLoadFloat4(&C);
		XMMATRIX _triTransform = XMMatrixRotationZ(-hue / 360.0f * XM_2PI) * XMMatrixTranslation(center.x, center.y, 0);
		_A = XMVector4Transform(_A, _triTransform);
		_B = XMVector4Transform(_B, _triTransform);
		_C = XMVector4Transform(_C, _triTransform);
		XMVECTOR O = XMVectorSet(pointer.x, pointer.y, 0, 0);
		XMVECTOR D = XMVectorSet(0, 0, 1, 0);
		bool hover_saturation = TriangleTests::Intersects(O, D, _A, _B, _C, distTri);

		if (hover_hue && state == IDLE)
		{
			state = FOCUS;
			huefocus = true;
		}
		else if (hover_saturation && state == IDLE)
		{
			state = FOCUS;
			huefocus = false;
		}
		else if (state == IDLE)
		{
			huefocus = false;
		}

		bool dragged = false;

		if (asInput::Press(asInput::MOUSE_BUTTON_LEFT))
		{
			if (state == FOCUS)
			{
				// activate
				dragged = true;
			}
		}

		if (asInput::Down(asInput::MOUSE_BUTTON_LEFT))
		{
			if (state == ACTIVE)
			{
				// continue drag if already grabbed whether it is intersecting or not
				dragged = true;
			}
		}

		dragged = dragged && !gui->IsWidgetDisabled(this);
		if (huefocus && dragged)
		{
			//hue pick
			const float angle = asMath::GetAngle(XMFLOAT2(pointer.x - center.x, pointer.y - center.y), XMFLOAT2(colorpicker_radius, 0));
			hue = angle / XM_2PI * 360.0f;
			gui->ActivateWidget(this);
		}
		else if (!huefocus && dragged)
		{
			// saturation pick
			float u, v, w;
			asMath::GetBarycentric(O, _A, _B, _C, u, v, w, true);
			// u = saturated corner (color)
			// v = desaturated corner (white)
			// w = no luminosity corner (black)

			hsv source;
			source.h = hue;
			source.s = 1;
			source.v = 1;
			rgb result = hsv2rgb(source);

			XMVECTOR color_corner = XMVectorSet(result.r, result.g, result.b, 1);
			XMVECTOR white_corner = XMVectorSet(1, 1, 1, 1);
			XMVECTOR black_corner = XMVectorSet(0, 0, 0, 1);
			XMVECTOR inner_point = u * color_corner + v * white_corner + w * black_corner;

			result.r = XMVectorGetX(inner_point);
			result.g = XMVectorGetY(inner_point);
			result.b = XMVectorGetZ(inner_point);
			source = rgb2hsv(result);

			saturation = source.s;
			luminance = source.v;

			gui->ActivateWidget(this);
		}
		else if (state != IDLE)
		{
			gui->DeactivateWidget(this);
		}

		if (dragged)
		{
			asEventArgs args;
			args.clickPos = pointer;
			args.color = GetPickColor();
			onColorChanged(args);
		}
	}
	void asColorPicker::Render(const asGUI* gui, CommandList cmd) const
	{
		asWindow::Render(gui, cmd);


		if (!IsVisible() || IsMinimized())
		{
			return;
		}

		struct Vertex
		{
			XMFLOAT4 pos;
			XMFLOAT4 col;
		};
		static asGraphics::GPUBuffer vb_saturation;
		static asGraphics::GPUBuffer vb_hue;
		static asGraphics::GPUBuffer vb_picker_saturation;
		static asGraphics::GPUBuffer vb_picker_hue;
		static asGraphics::GPUBuffer vb_preview;

		static std::vector<Vertex> vertices_saturation;

		static bool buffersComplete = false;
		if (!buffersComplete)
		{
			buffersComplete = true;

			// saturation
			{
				vertices_saturation.push_back({ XMFLOAT4(0,0,0,0),XMFLOAT4(1,0,0,1) });	// hue
				vertices_saturation.push_back({ XMFLOAT4(0,0,0,0),XMFLOAT4(1,1,1,1) });	// white
				vertices_saturation.push_back({ XMFLOAT4(0,0,0,0),XMFLOAT4(0,0,0,1) });	// black
				asMath::ConstructTriangleEquilateral(colorpicker_radius_triangle, vertices_saturation[0].pos, vertices_saturation[1].pos, vertices_saturation[2].pos);

				// create alpha blended edge:
				vertices_saturation.push_back(vertices_saturation[0]); // outer
				vertices_saturation.push_back(vertices_saturation[0]); // inner
				vertices_saturation.push_back(vertices_saturation[1]); // outer
				vertices_saturation.push_back(vertices_saturation[1]); // inner
				vertices_saturation.push_back(vertices_saturation[2]); // outer
				vertices_saturation.push_back(vertices_saturation[2]); // inner
				vertices_saturation.push_back(vertices_saturation[0]); // outer
				vertices_saturation.push_back(vertices_saturation[0]); // inner
				asMath::ConstructTriangleEquilateral(colorpicker_radius_triangle + 4, vertices_saturation[3].pos, vertices_saturation[5].pos, vertices_saturation[7].pos); // extrude outer
				vertices_saturation[9].pos = vertices_saturation[3].pos; // last outer

				GPUBufferDesc desc;
				desc.BindFlags = BIND_VERTEX_BUFFER;
				desc.ByteWidth = (uint32_t)(vertices_saturation.size() * sizeof(Vertex));
				desc.CPUAccessFlags = CPU_ACCESS_WRITE;
				desc.MiscFlags = 0;
				desc.StructureByteStride = 0;
				desc.Usage = USAGE_DYNAMIC;
				SubresourceData data;
				data.pSysMem = vertices_saturation.data();
				asRenderer::GetDevice()->CreateBuffer(&desc, &data, &vb_saturation);
			}
			// hue
			{
				const float edge = 2.0f;
				std::vector<Vertex> vertices;
				uint32_t segment_count = 100;
				// inner alpha blended edge
				for (uint32_t i = 0; i <= segment_count; ++i)
				{
					float p = float(i) / segment_count;
					float t = p * XM_2PI;
					float x = cos(t);
					float y = -sin(t);
					hsv source;
					source.h = p * 360.0f;
					source.s = 1;
					source.v = 1;
					rgb result = hsv2rgb(source);
					XMFLOAT4 color = XMFLOAT4(result.r, result.g, result.b, 1);
					XMFLOAT4 coloralpha = XMFLOAT4(result.r, result.g, result.b, 0);
					vertices.push_back({ XMFLOAT4((colorpicker_radius - edge) * x, (colorpicker_radius - edge) * y, 0, 1), coloralpha });
					vertices.push_back({ XMFLOAT4(colorpicker_radius * x, colorpicker_radius * y, 0, 1), color });
				}
				// middle hue
				for (uint32_t i = 0; i <= segment_count; ++i)
				{
					float p = float(i) / segment_count;
					float t = p * XM_2PI;
					float x = cos(t);
					float y = -sin(t);
					hsv source;
					source.h = p * 360.0f;
					source.s = 1;
					source.v = 1;
					rgb result = hsv2rgb(source);
					XMFLOAT4 color = XMFLOAT4(result.r, result.g, result.b, 1);
					vertices.push_back({ XMFLOAT4(colorpicker_radius * x, colorpicker_radius * y, 0, 1), color });
					vertices.push_back({ XMFLOAT4((colorpicker_radius + colorpicker_width) * x, (colorpicker_radius + colorpicker_width) * y, 0, 1), color });
				}
				// outer alpha blended edge
				for (uint32_t i = 0; i <= segment_count; ++i)
				{
					float p = float(i) / segment_count;
					float t = p * XM_2PI;
					float x = cos(t);
					float y = -sin(t);
					hsv source;
					source.h = p * 360.0f;
					source.s = 1;
					source.v = 1;
					rgb result = hsv2rgb(source);
					XMFLOAT4 color = XMFLOAT4(result.r, result.g, result.b, 1);
					XMFLOAT4 coloralpha = XMFLOAT4(result.r, result.g, result.b, 0);
					vertices.push_back({ XMFLOAT4((colorpicker_radius + colorpicker_width) * x, (colorpicker_radius + colorpicker_width) * y, 0, 1), color });
					vertices.push_back({ XMFLOAT4((colorpicker_radius + colorpicker_width + edge) * x, (colorpicker_radius + colorpicker_width + edge) * y, 0, 1), coloralpha });
				}

				GPUBufferDesc desc;
				desc.BindFlags = BIND_VERTEX_BUFFER;
				desc.ByteWidth = (uint32_t)(vertices.size() * sizeof(Vertex));
				desc.CPUAccessFlags = 0;
				desc.MiscFlags = 0;
				desc.StructureByteStride = 0;
				desc.Usage = USAGE_IMMUTABLE;
				SubresourceData data;
				data.pSysMem = vertices.data();
				asRenderer::GetDevice()->CreateBuffer(&desc, &data, &vb_hue);
			}
			// saturation picker (small circle)
			{
				float _radius = 3;
				float _width = 3;
				std::vector<Vertex> vertices;
				uint32_t segment_count = 100;
				for (uint32_t i = 0; i <= segment_count; ++i)
				{
					float p = float(i) / 100;
					float t = p * XM_2PI;
					float x = cos(t);
					float y = -sin(t);
					vertices.push_back({ XMFLOAT4(_radius * x, _radius * y, 0, 1), XMFLOAT4(1,1,1,1) });
					vertices.push_back({ XMFLOAT4((_radius + _width) * x, (_radius + _width) * y, 0, 1), XMFLOAT4(1,1,1,1) });
				}

				GPUBufferDesc desc;
				desc.BindFlags = BIND_VERTEX_BUFFER;
				desc.ByteWidth = (uint32_t)(vertices.size() * sizeof(Vertex));
				desc.CPUAccessFlags = 0;
				desc.MiscFlags = 0;
				desc.StructureByteStride = 0;
				desc.Usage = USAGE_IMMUTABLE;
				SubresourceData data;
				data.pSysMem = vertices.data();
				asRenderer::GetDevice()->CreateBuffer(&desc, &data, &vb_picker_saturation);
			}
			// hue picker (rectangle)
			{
				float boldness = 4.0f;
				float halfheight = 8.0f;
				Vertex vertices[] = {
					// left side:
					{ XMFLOAT4(colorpicker_radius - boldness, -halfheight, 0, 1),XMFLOAT4(1,1,1,1) },
					{ XMFLOAT4(colorpicker_radius, -halfheight, 0, 1),XMFLOAT4(1,1,1,1) },
					{ XMFLOAT4(colorpicker_radius - boldness, halfheight, 0, 1),XMFLOAT4(1,1,1,1) },
					{ XMFLOAT4(colorpicker_radius, halfheight, 0, 1),XMFLOAT4(1,1,1,1) },

					// bottom side:
					{ XMFLOAT4(colorpicker_radius - boldness, halfheight, 0, 1),XMFLOAT4(1,1,1,1) },
					{ XMFLOAT4(colorpicker_radius - boldness, halfheight - boldness, 0, 1),XMFLOAT4(1,1,1,1) },
					{ XMFLOAT4(colorpicker_radius + colorpicker_width + boldness, halfheight, 0, 1),XMFLOAT4(1,1,1,1) },
					{ XMFLOAT4(colorpicker_radius + colorpicker_width + boldness, halfheight - boldness, 0, 1),XMFLOAT4(1,1,1,1) },

					// right side:
					{ XMFLOAT4(colorpicker_radius + colorpicker_width + boldness, halfheight, 0, 1),XMFLOAT4(1,1,1,1) },
					{ XMFLOAT4(colorpicker_radius + colorpicker_width, halfheight, 0, 1),XMFLOAT4(1,1,1,1) },
					{ XMFLOAT4(colorpicker_radius + colorpicker_width + boldness, -halfheight, 0, 1),XMFLOAT4(1,1,1,1) },
					{ XMFLOAT4(colorpicker_radius + colorpicker_width, -halfheight, 0, 1),XMFLOAT4(1,1,1,1) },

					// top side:
					{ XMFLOAT4(colorpicker_radius + colorpicker_width + boldness, -halfheight, 0, 1),XMFLOAT4(1,1,1,1) },
					{ XMFLOAT4(colorpicker_radius + colorpicker_width + boldness, -halfheight + boldness, 0, 1),XMFLOAT4(1,1,1,1) },
					{ XMFLOAT4(colorpicker_radius - boldness, -halfheight, 0, 1),XMFLOAT4(1,1,1,1) },
					{ XMFLOAT4(colorpicker_radius - boldness, -halfheight + boldness, 0, 1),XMFLOAT4(1,1,1,1) },
				};

				GPUBufferDesc desc;
				desc.BindFlags = BIND_VERTEX_BUFFER;
				desc.ByteWidth = (uint32_t)sizeof(vertices);
				desc.CPUAccessFlags = 0;
				desc.MiscFlags = 0;
				desc.StructureByteStride = 0;
				desc.Usage = USAGE_IMMUTABLE;
				SubresourceData data;
				data.pSysMem = vertices;
				asRenderer::GetDevice()->CreateBuffer(&desc, &data, &vb_picker_hue);
			}
			// preview
			{
				float _width = 20;
				Vertex vertices[] = {
					{ XMFLOAT4(-_width, -_width, 0, 1),XMFLOAT4(1,1,1,1) },
					{ XMFLOAT4(-_width, _width, 0, 1),XMFLOAT4(1,1,1,1) },
					{ XMFLOAT4(_width, _width, 0, 1),XMFLOAT4(1,1,1,1) },
					{ XMFLOAT4(-_width, -_width, 0, 1),XMFLOAT4(1,1,1,1) },
					{ XMFLOAT4(_width, _width, 0, 1),XMFLOAT4(1,1,1,1) },
					{ XMFLOAT4(_width, -_width, 0, 1),XMFLOAT4(1,1,1,1) },
				};

				GPUBufferDesc desc;
				desc.BindFlags = BIND_VERTEX_BUFFER;
				desc.ByteWidth = (uint32_t)sizeof(vertices);
				desc.CPUAccessFlags = 0;
				desc.MiscFlags = 0;
				desc.StructureByteStride = 0;
				desc.Usage = USAGE_IMMUTABLE;
				SubresourceData data;
				data.pSysMem = vertices;
				asRenderer::GetDevice()->CreateBuffer(&desc, &data, &vb_preview);
			}

		}

		const asColor final_color = GetPickColor();
		const float angle = hue / 360.0f * XM_2PI;

		const XMMATRIX Projection = asRenderer::GetDevice()->GetScreenProjection();

		asRenderer::GetDevice()->BindConstantBuffer(VS, asRenderer::GetConstantBuffer(CBTYPE_MISC), CBSLOT_RENDERER_MISC, cmd);
		asRenderer::GetDevice()->BindPipelineState(&PSO_colorpicker, cmd);

		MiscCB cb;

		// render saturation triangle
		{
			if (vb_saturation.IsValid() && !vertices_saturation.empty())
			{
				hsv source;
				source.h = hue;
				source.s = 1;
				source.v = 1;
				rgb result = hsv2rgb(source);
				vertices_saturation[0].col = XMFLOAT4(result.r, result.g, result.b, 1);

				vertices_saturation[3].col = vertices_saturation[0].col; vertices_saturation[3].col.w = 0;
				vertices_saturation[4].col = vertices_saturation[0].col;
				vertices_saturation[5].col = vertices_saturation[1].col; vertices_saturation[5].col.w = 0;
				vertices_saturation[6].col = vertices_saturation[1].col;
				vertices_saturation[7].col = vertices_saturation[2].col; vertices_saturation[7].col.w = 0;
				vertices_saturation[8].col = vertices_saturation[2].col;
				vertices_saturation[9].col = vertices_saturation[0].col; vertices_saturation[9].col.w = 0;
				vertices_saturation[10].col = vertices_saturation[0].col;

				asRenderer::GetDevice()->UpdateBuffer(&vb_saturation, vertices_saturation.data(), cmd, vb_saturation.GetDesc().ByteWidth);
			}

			XMStoreFloat4x4(&cb.g_xTransform,
				XMMatrixRotationZ(-angle) *
				XMMatrixTranslation(translation.x + colorpicker_center, translation.y + colorpicker_center, 0) *
				Projection
			);
			cb.g_xColor = IsEnabled() ? float4(1, 1, 1, 1) : float4(0.5f, 0.5f, 0.5f, 1);
			asRenderer::GetDevice()->UpdateBuffer(asRenderer::GetConstantBuffer(CBTYPE_MISC), &cb, cmd);
			const GPUBuffer* vbs[] = {
				&vb_saturation,
			};
			const uint32_t strides[] = {
				sizeof(Vertex),
			};
			asRenderer::GetDevice()->BindVertexBuffers(vbs, 0, arraysize(vbs), strides, nullptr, cmd);
			asRenderer::GetDevice()->Draw(vb_saturation.GetDesc().ByteWidth / sizeof(Vertex), 0, cmd);
		}

		// render hue circle
		{
			XMStoreFloat4x4(&cb.g_xTransform,
				XMMatrixTranslation(translation.x + colorpicker_center, translation.y + colorpicker_center, 0) *
				Projection
			);
			cb.g_xColor = IsEnabled() ? float4(1, 1, 1, 1) : float4(0.5f, 0.5f, 0.5f, 1);
			asRenderer::GetDevice()->UpdateBuffer(asRenderer::GetConstantBuffer(CBTYPE_MISC), &cb, cmd);
			const GPUBuffer* vbs[] = {
				&vb_hue,
			};
			const uint32_t strides[] = {
				sizeof(Vertex),
			};
			asRenderer::GetDevice()->BindVertexBuffers(vbs, 0, arraysize(vbs), strides, nullptr, cmd);
			asRenderer::GetDevice()->Draw(vb_hue.GetDesc().ByteWidth / sizeof(Vertex), 0, cmd);
		}

		// render hue picker
		if (IsEnabled())
		{
			XMStoreFloat4x4(&cb.g_xTransform,
				XMMatrixRotationZ(-hue / 360.0f * XM_2PI) *
				XMMatrixTranslation(translation.x + colorpicker_center, translation.y + colorpicker_center, 0) *
				Projection
			);

			hsv source;
			source.h = hue;
			source.s = 1;
			source.v = 1;
			rgb result = hsv2rgb(source);
			cb.g_xColor = float4(1 - result.r, 1 - result.g, 1 - result.b, 1);

			asRenderer::GetDevice()->UpdateBuffer(asRenderer::GetConstantBuffer(CBTYPE_MISC), &cb, cmd);
			const GPUBuffer* vbs[] = {
				&vb_picker_hue,
			};
			const uint32_t strides[] = {
				sizeof(Vertex),
			};
			asRenderer::GetDevice()->BindVertexBuffers(vbs, 0, arraysize(vbs), strides, nullptr, cmd);
			asRenderer::GetDevice()->Draw(vb_picker_hue.GetDesc().ByteWidth / sizeof(Vertex), 0, cmd);
		}

		// render saturation picker
		if (IsEnabled())
		{
			XMFLOAT2 center = XMFLOAT2(translation.x + colorpicker_center, translation.y + colorpicker_center);
			XMFLOAT4 A, B, C;
			asMath::ConstructTriangleEquilateral(colorpicker_radius_triangle, A, B, C);
			XMVECTOR _A = XMLoadFloat4(&A);
			XMVECTOR _B = XMLoadFloat4(&B);
			XMVECTOR _C = XMLoadFloat4(&C);
			XMMATRIX _triTransform = XMMatrixRotationZ(-hue / 360.0f * XM_2PI) * XMMatrixTranslation(center.x, center.y, 0);
			_A = XMVector4Transform(_A, _triTransform);
			_B = XMVector4Transform(_B, _triTransform);
			_C = XMVector4Transform(_C, _triTransform);

			hsv source;
			source.h = hue;
			source.s = 1;
			source.v = 1;
			rgb result = hsv2rgb(source);

			XMVECTOR color_corner = XMVectorSet(result.r, result.g, result.b, 1);
			XMVECTOR white_corner = XMVectorSet(1, 1, 1, 1);
			XMVECTOR black_corner = XMVectorSet(0, 0, 0, 1);

			source.h = hue;
			source.s = saturation;
			source.v = luminance;
			result = hsv2rgb(source);
			XMVECTOR inner_point = XMVectorSet(result.r, result.g, result.b, 1);

			float u, v, w;
			asMath::GetBarycentric(inner_point, color_corner, white_corner, black_corner, u, v, w, true);

			XMVECTOR picker_center = u * _A + v * _B + w * _C;

			XMStoreFloat4x4(&cb.g_xTransform,
				XMMatrixTranslationFromVector(picker_center) *
				Projection
			);
			cb.g_xColor = float4(1 - final_color.toFloat3().x, 1 - final_color.toFloat3().y, 1 - final_color.toFloat3().z, 1);
			asRenderer::GetDevice()->UpdateBuffer(asRenderer::GetConstantBuffer(CBTYPE_MISC), &cb, cmd);
			const GPUBuffer* vbs[] = {
				&vb_picker_saturation,
			};
			const uint32_t strides[] = {
				sizeof(Vertex),
			};
			asRenderer::GetDevice()->BindVertexBuffers(vbs, 0, arraysize(vbs), strides, nullptr, cmd);
			asRenderer::GetDevice()->Draw(vb_picker_saturation.GetDesc().ByteWidth / sizeof(Vertex), 0, cmd);
		}

		// render preview
		{
			XMStoreFloat4x4(&cb.g_xTransform,
				XMMatrixTranslation(translation.x + 260, translation.y + 40, 0) *
				Projection
			);
			cb.g_xColor = final_color.toFloat4();
			asRenderer::GetDevice()->UpdateBuffer(asRenderer::GetConstantBuffer(CBTYPE_MISC), &cb, cmd);
			const GPUBuffer* vbs[] = {
				&vb_preview,
			};
			const uint32_t strides[] = {
				sizeof(Vertex),
			};
			asRenderer::GetDevice()->BindVertexBuffers(vbs, 0, arraysize(vbs), strides, nullptr, cmd);
			asRenderer::GetDevice()->Draw(vb_preview.GetDesc().ByteWidth / sizeof(Vertex), 0, cmd);
		}

		// RGB values:
		stringstream ss("");
		ss << "R: " << int(final_color.getR()) << endl;
		ss << "G: " << int(final_color.getG()) << endl;
		ss << "B: " << int(final_color.getB()) << endl;
		ss << endl;
		ss << "H: " << int(hue) << endl;
		ss << "S: " << int(saturation * 100) << endl;
		ss << "V: " << int(luminance * 100) << endl;
		asFont(ss.str(), asFontParams((int)(translation.x + 240), (int)(translation.y + 80), ASFONTSIZE_DEFAULT, WIFALIGN_LEFT, WIFALIGN_TOP, 0, 0,
			textColor, textShadowColor)).Draw(cmd);

	}
	asColor asColorPicker::GetPickColor() const
	{
		hsv source;
		source.h = hue;
		source.s = saturation;
		source.v = luminance;
		rgb result = hsv2rgb(source);
		return asColor::fromFloat3(XMFLOAT3(result.r, result.g, result.b));
	}
	void asColorPicker::SetPickColor(asColor value)
	{
		rgb source;
		source.r = value.toFloat3().x;
		source.g = value.toFloat3().y;
		source.b = value.toFloat3().z;
		hsv result = rgb2hsv(source);
		if (
			(value.getR() < 255 || value.getG() < 255 || value.getB() < 255) &&
			(value.getR() > 0 || value.getG() > 0 || value.getB() > 0)
			)
		{
			hue = result.h; // only change the hue when not pure black or white because those don't retain the saturation value
		}
		saturation = result.s;
		luminance = result.v;
	}
	void asColorPicker::OnColorChanged(function<void(asEventArgs args)> func)
	{
		onColorChanged = move(func);
	}

}
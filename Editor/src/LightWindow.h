#pragma once
namespace as
{


	class asGUI;
	class asWindow;
	class asLabel;
	class asCheckBox;
	class asSlider;
	class asButton;
	class asColorPicker;
	class asComboBox;

	class LightWindow
	{
	public:
		LightWindow(asGUI* gui);
		~LightWindow();

		asGUI* GUI;

		asECS::Entity entity;
		void SetEntity(asECS::Entity entity);

		void SetLightType(asScene::LightComponent::LightType type);

		asWindow* lightWindow;
		asSlider* energySlider;
		asSlider* rangeSlider;
		asSlider* radiusSlider;
		asSlider* widthSlider;
		asSlider* heightSlider;
		asSlider* fovSlider;
		asSlider* biasSlider;
		asCheckBox* shadowCheckBox;
		asCheckBox* haloCheckBox;
		asCheckBox* volumetricsCheckBox;
		asCheckBox* staticCheckBox;
		asButton* addLightButton;
		asColorPicker* colorPicker;
		asComboBox* typeSelectorComboBox;

		asLabel* lensflare_Label;
		asButton* lensflare_Button[7];
	};
}


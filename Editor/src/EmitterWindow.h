#pragma once

namespace as
{

	class asGUI;
	class asWindow;
	class asLabel;
	class asCheckBox;
	class asSlider;
	class asComboBox;
	class asColorPicker;
	class asButton;

	class MaterialWindow;

	class EmitterWindow
	{
	public:
		EmitterWindow(asGUI* gui);
		~EmitterWindow();

		asECS::Entity entity;
		void SetEntity(asECS::Entity entity);

		void UpdateData();

		asScene::asEmittedParticle* GetEmitter();

		asGUI* GUI;

		asWindow* emitterWindow;

		asTextInputField* emitterNameField;
		asButton* addButton;
		asButton* restartButton;
		asComboBox* meshComboBox;
		asComboBox* shaderTypeComboBox;
		asLabel* infoLabel;
		asSlider* maxParticlesSlider;
		asCheckBox* sortCheckBox;
		asCheckBox* depthCollisionsCheckBox;
		asCheckBox* sphCheckBox;
		asCheckBox* pauseCheckBox;
		asCheckBox* debugCheckBox;
		asSlider* emitCountSlider;
		asSlider* emitSizeSlider;
		asSlider* emitRotationSlider;
		asSlider* emitNormalSlider;
		asSlider* emitScalingSlider;
		asSlider* emitLifeSlider;
		asSlider* emitRandomnessSlider;
		asSlider* emitLifeRandomnessSlider;
		asSlider* emitMotionBlurSlider;
		asSlider* emitMassSlider;
		asSlider* timestepSlider;
		asSlider* sph_h_Slider;
		asSlider* sph_K_Slider;
		asSlider* sph_p0_Slider;
		asSlider* sph_e_Slider;

	};

}

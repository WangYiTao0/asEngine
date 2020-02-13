#pragma once

namespace as
{

	class asGUI;
	class asWindow;
	class asLabel;
	class asCheckBox;
	class asSlider;
	class asColorPicker;
	class asButton;
	class asComboBox;
	class asTextInputField;

	class SoundWindow
	{
	public:
		SoundWindow(asGUI* gui);
		~SoundWindow();

		asECS::Entity entity = asECS::INVALID_ENTITY;
		void SetEntity(asECS::Entity entity);

		asGUI* GUI;

		asWindow* soundWindow;
		asComboBox* reverbComboBox;
		asButton* addButton;
		asLabel* filenameLabel;
		asTextInputField* nameField;
		asButton* playstopButton;
		asCheckBox* loopedCheckbox;
		asSlider* volumeSlider;
	};
}
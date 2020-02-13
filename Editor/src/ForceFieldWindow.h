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

	class ForceFieldWindow
	{
	public:
		ForceFieldWindow(asGUI* gui);
		~ForceFieldWindow();

		asECS::Entity entity;
		void SetEntity(asECS::Entity entity);

		asGUI* GUI;

		asWindow* forceFieldWindow;

		asComboBox* typeComboBox;
		asSlider* gravitySlider;
		asSlider* rangeSlider;
		asButton* addButton;
	};

}
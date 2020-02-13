#pragma once
namespace as
{
	class asGUI;
	class asWindow;
	class asLabel;
	class asCheckBox;
	class asSlider;

	class DecalWindow
	{
	public:
		DecalWindow(asGUI* gui);
		~DecalWindow();

		asGUI* GUI;

		asECS::Entity entity;
		void SetEntity(asECS::Entity entity);

		asTextInputField* decalNameField;

		asWindow* decalWindow;
	};

}

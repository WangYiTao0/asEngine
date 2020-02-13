#pragma once

namespace as
{

	class asGUI;
	class asWindow;
	class asLabel;
	class asCheckBox;
	class asSlider;

	class EnvProbeWindow
	{
	public:
		EnvProbeWindow(asGUI* gui);
		~EnvProbeWindow();

		asGUI* GUI;

		asECS::Entity entity;
		void SetEntity(asECS::Entity entity);

		asWindow* envProbeWindow;

		asCheckBox* realTimeCheckBox;
		asButton* generateButton;
		asButton* refreshButton;
		asButton* refreshAllButton;
	};

}
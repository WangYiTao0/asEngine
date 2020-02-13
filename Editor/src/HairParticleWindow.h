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

	class HairParticleWindow
	{
	public:
		HairParticleWindow(asGUI* gui);
		~HairParticleWindow();

		asECS::Entity entity;
		void SetEntity(asECS::Entity entity);

		void UpdateData();

		asScene::asHairParticle* GetHair();

		asGUI* GUI;

		asWindow* hairWindow;

		asButton* addButton;
		asComboBox* meshComboBox;
		asSlider* lengthSlider;
		asSlider* stiffnessSlider;
		asSlider* randomnessSlider;
		asSlider* countSlider;
		asSlider* segmentcountSlider;
		asSlider* randomSeedSlider;
		asSlider* viewDistanceSlider;

	};
}

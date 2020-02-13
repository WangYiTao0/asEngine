#pragma once

namespace as
{
	class EditorComponent;

	class asGUI;
	class asWindow;
	class asLabel;
	class asCheckBox;
	class asSlider;
	class asComboBox;
	class asColorPicker;

	class ObjectWindow
	{
	public:
		ObjectWindow(EditorComponent* editor);
		~ObjectWindow();

		EditorComponent* editor;
		asECS::Entity entity;
		void SetEntity(asECS::Entity entity);

		asGUI* GUI;

		asWindow* objectWindow;

		asLabel* nameLabel;
		asCheckBox* renderableCheckBox;
		asSlider* ditherSlider;
		asSlider* cascadeMaskSlider;
		asColorPicker* colorPicker;

		asLabel* physicsLabel;
		asCheckBox* rigidBodyCheckBox;
		asCheckBox* disabledeactivationCheckBox;
		asCheckBox* kinematicCheckBox;
		asComboBox* collisionShapeComboBox;

		asSlider* lightmapResolutionSlider;
		asComboBox* lightmapSourceUVSetComboBox;
		asButton* generateLightmapButton;
		asButton* stopLightmapGenButton;
		asButton* clearLightmapButton;
	};
}


#pragma once

namespace as
{

	class asGUI;
	class asWindow;
	class asLabel;
	class asCheckBox;
	class asSlider;
	class asButton;

	class MeshWindow
	{
	public:
		MeshWindow(asGUI* gui);
		~MeshWindow();

		asGUI* GUI;

		asECS::Entity entity;
		void SetEntity(asECS::Entity entity);

		asWindow* meshWindow;
		asLabel* meshInfoLabel;
		asCheckBox* doubleSidedCheckBox;
		asCheckBox* softbodyCheckBox;
		asSlider* massSlider;
		asSlider* frictionSlider;
		asButton* impostorCreateButton;
		asSlider* impostorDistanceSlider;
		asSlider* tessellationFactorSlider;
		asButton* flipCullingButton;
		asButton* flipNormalsButton;
		asButton* computeNormalsSmoothButton;
		asButton* computeNormalsHardButton;
		asButton* recenterButton;
		asButton* recenterToBottomButton;
	};
}

#pragma once
namespace as
{
	class asGUI;
	class asWindow;
	class asLabel;
	class asCheckBox;
	class asSlider;
	class asTextInputField;

	class CameraWindow
	{
	public:
		CameraWindow(asGUI* gui);
		~CameraWindow();

		void ResetCam();

		asECS::Entity proxy = asECS::INVALID_ENTITY;
		void SetEntity(asECS::Entity entity);


		asScene::TransformComponent camera_transform;
		asScene::TransformComponent camera_target;

		asGUI* GUI;

		asWindow* cameraWindow;
		asSlider* farPlaneSlider;
		asSlider* nearPlaneSlider;
		asSlider* fovSlider;
		asSlider* movespeedSlider;
		asSlider* rotationspeedSlider;
		asButton* resetButton;
		asCheckBox* fpsCheckBox;

		asButton* proxyButton;
		asTextInputField* proxyNameField;
		asCheckBox* followCheckBox;
		asSlider* followSlider;
	};

}
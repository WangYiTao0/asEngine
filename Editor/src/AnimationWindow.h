#pragma once
#include <asEngine.h>
namespace as
{
	class asGUI;
	class asWindow;
	class asLabel;
	class asCheckBox;
	class asSlider;
	class asComboBox;
	class asButton;

	class AnimationWindow
	{
	public:
		AnimationWindow(asGUI* gui);
		~AnimationWindow();

		asGUI* GUI;

		asECS::Entity entity = asECS::INVALID_ENTITY;

		asWindow* animWindow;
		asComboBox* animationsComboBox;
		asCheckBox* loopedCheckBox;
		asButton* playButton;
		asButton* stopButton;
		asSlider* timerSlider;

		void Update();
	};
}
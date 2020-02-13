#pragma once

namespace as
{

	class RenderPath3D;

	class asGUI;
	class asWindow;
	class asLabel;
	class asCheckBox;
	class asSlider;
	class asButton;

	class PostprocessWindow
	{
	public:
		PostprocessWindow(asGUI* gui, RenderPath3D* component);
		~PostprocessWindow();

		asGUI* GUI;
		RenderPath3D* component;

		asWindow* ppWindow;
		asSlider* exposureSlider;
		asCheckBox* lensFlareCheckBox;
		asCheckBox* lightShaftsCheckBox;
		asCheckBox* ssaoCheckBox;
		asSlider* ssaoRangeSlider;
		asSlider* ssaoSampleCountSlider;
		asSlider* ssaoPowerSlider;
		asCheckBox* ssrCheckBox;
		asCheckBox* sssCheckBox;
		asCheckBox* eyeAdaptionCheckBox;
		asCheckBox* motionBlurCheckBox;
		asSlider* motionBlurStrengthSlider;
		asCheckBox* depthOfFieldCheckBox;
		asSlider* depthOfFieldFocusSlider;
		asSlider* depthOfFieldScaleSlider;
		asSlider* depthOfFieldAspectSlider;
		asCheckBox* bloomCheckBox;
		asSlider* bloomStrengthSlider;
		asCheckBox* fxaaCheckBox;
		asCheckBox* colorGradingCheckBox;
		asButton* colorGradingButton;
		asCheckBox* sharpenFilterCheckBox;
		asSlider* sharpenFilterAmountSlider;
		asCheckBox* outlineCheckBox;
		asSlider* outlineThresholdSlider;
		asSlider* outlineThicknessSlider;
		asCheckBox* chromaticaberrationCheckBox;
		asSlider* chromaticaberrationSlider;


	};

}

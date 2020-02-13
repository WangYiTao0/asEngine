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

	class WeatherWindow
	{
	public:
		WeatherWindow(asGUI* gui);
		~WeatherWindow();

		void Update();

		asScene::WeatherComponent& GetWeather() const;
		void InvalidateProbes() const;

		asGUI* GUI;

		asWindow* weatherWindow;
		asSlider* fogStartSlider;
		asSlider* fogEndSlider;
		asSlider* fogHeightSlider;
		asSlider* cloudinessSlider;
		asSlider* cloudScaleSlider;
		asSlider* cloudSpeedSlider;
		asSlider* windSpeedSlider;
		asSlider* windDirectionSlider;
		asButton* skyButton;
		asColorPicker* ambientColorPicker;
		asColorPicker* horizonColorPicker;
		asColorPicker* zenithColorPicker;

		// ocean params:
		asCheckBox* ocean_enabledCheckBox;
		asSlider* ocean_patchSizeSlider;
		asSlider* ocean_waveAmplitudeSlider;
		asSlider* ocean_choppyScaleSlider;
		asSlider* ocean_windDependencySlider;
		asSlider* ocean_timeScaleSlider;
		asSlider* ocean_heightSlider;
		asSlider* ocean_detailSlider;
		asSlider* ocean_toleranceSlider;
		asColorPicker* ocean_colorPicker;
		asButton* ocean_resetButton;
	};
}


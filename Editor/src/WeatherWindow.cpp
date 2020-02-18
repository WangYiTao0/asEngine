#include <asEngine.h>
#include "WeatherWindow.h"

#include <thread>

using namespace std;
namespace as
{
	using namespace asECS;
	using namespace asScene;
	using namespace asGraphics;

	WeatherWindow::WeatherWindow(asGUI* gui) : GUI(gui)
	{
		assert(GUI && "Invalid GUI!");

		XMFLOAT2 option_size = XMFLOAT2(100, 20);
		XMFLOAT2 slider_size = XMFLOAT2(100, 20);

		float screenW = (float)asRenderer::GetDevice()->GetScreenWidth();
		float screenH = (float)asRenderer::GetDevice()->GetScreenHeight();


		weatherWindow = new asWindow(GUI, "Weather Window");
		weatherWindow->SetSize(XMFLOAT2(1000, 820));
		GUI->AddWidget(weatherWindow);

		float x = 200;
		float y = 20;
		float step = 32;

		fogStartSlider = new asSlider(0, 5000, 0, 100000, "Fog Start: ");
		fogStartSlider->SetSize(slider_size);
		fogStartSlider->SetPos(XMFLOAT2(x, y += step));
		fogStartSlider->OnSlide([&](asEventArgs args) {
			GetWeather().fogStart = args.fValue;
			});
		weatherWindow->AddWidget(fogStartSlider);

		fogEndSlider = new asSlider(1, 5000, 1000, 10000, "Fog End: ");
		fogEndSlider->SetSize(slider_size);
		fogEndSlider->SetPos(XMFLOAT2(x, y += step));
		fogEndSlider->OnSlide([&](asEventArgs args) {
			GetWeather().fogEnd = args.fValue;
			});
		weatherWindow->AddWidget(fogEndSlider);

		fogHeightSlider = new asSlider(0, 1, 0, 10000, "Fog Height: ");
		fogHeightSlider->SetSize(slider_size);
		fogHeightSlider->SetPos(XMFLOAT2(x, y += step));
		fogHeightSlider->OnSlide([&](asEventArgs args) {
			GetWeather().fogHeight = args.fValue;
			});
		weatherWindow->AddWidget(fogHeightSlider);

		cloudinessSlider = new asSlider(0, 1, 0.0f, 10000, "Cloudiness: ");
		cloudinessSlider->SetSize(slider_size);
		cloudinessSlider->SetPos(XMFLOAT2(x, y += step));
		cloudinessSlider->OnSlide([&](asEventArgs args) {
			GetWeather().cloudiness = args.fValue;
			});
		weatherWindow->AddWidget(cloudinessSlider);

		cloudScaleSlider = new asSlider(0.00005f, 0.001f, 0.0005f, 10000, "Cloud Scale: ");
		cloudScaleSlider->SetSize(slider_size);
		cloudScaleSlider->SetPos(XMFLOAT2(x, y += step));
		cloudScaleSlider->OnSlide([&](asEventArgs args) {
			GetWeather().cloudScale = args.fValue;
			});
		weatherWindow->AddWidget(cloudScaleSlider);

		cloudSpeedSlider = new asSlider(0.001f, 0.2f, 0.1f, 10000, "Cloud Speed: ");
		cloudSpeedSlider->SetSize(slider_size);
		cloudSpeedSlider->SetPos(XMFLOAT2(x, y += step));
		cloudSpeedSlider->OnSlide([&](asEventArgs args) {
			GetWeather().cloudSpeed = args.fValue;
			});
		weatherWindow->AddWidget(cloudSpeedSlider);

		windSpeedSlider = new asSlider(0.001f, 0.2f, 0.1f, 10000, "Wind Speed: ");
		windSpeedSlider->SetSize(slider_size);
		windSpeedSlider->SetPos(XMFLOAT2(x, y += step));
		weatherWindow->AddWidget(windSpeedSlider);

		windDirectionSlider = new asSlider(0, 1, 0, 10000, "Wind Direction: ");
		windDirectionSlider->SetSize(slider_size);
		windDirectionSlider->SetPos(XMFLOAT2(x, y += step));
		windDirectionSlider->OnSlide([&](asEventArgs args) {
			XMMATRIX rot = XMMatrixRotationY(args.fValue * XM_PI * 2);
			XMVECTOR dir = XMVectorSet(1, 0, 0, 0);
			dir = XMVector3TransformNormal(dir, rot);
			dir *= windSpeedSlider->GetValue();
			XMStoreFloat3(&GetWeather().windDirection, dir);
			});
		weatherWindow->AddWidget(windDirectionSlider);


		skyButton = new asButton("Load Sky");
		skyButton->SetTooltip("Load a skybox cubemap texture...");
		skyButton->SetSize(XMFLOAT2(240, 30));
		skyButton->SetPos(XMFLOAT2(x - 100, y += step));
		skyButton->OnClick([=](asEventArgs args) {
			auto& weather = GetWeather();

			if (weather.skyMap == nullptr)
			{
				asHelper::FileDialogParams params;
				asHelper::FileDialogResult result;
				params.type = asHelper::FileDialogParams::OPEN;
				params.description = "Cubemap texture";
				params.extensions.push_back("dds");
				asHelper::FileDialog(params, result);

				if (result.ok) {
					string fileName = result.filenames.front();
					weather.skyMapName = fileName;
					weather.skyMap = asResourceManager::Load(fileName);
					skyButton->SetText(fileName);
				}
			}
			else
			{
				weather.skyMap.reset();
				weather.skyMapName.clear();
				skyButton->SetText("Load Sky");
			}

			// Also, we invalidate all environment probes to reflect the sky changes.
			InvalidateProbes();

			});
		weatherWindow->AddWidget(skyButton);

		asButton* preset0Button = new asButton("WeatherPreset - Default");
		preset0Button->SetTooltip("Apply this weather preset to the world.");
		preset0Button->SetSize(XMFLOAT2(240, 30));
		preset0Button->SetPos(XMFLOAT2(x - 100, y += step * 2));
		preset0Button->OnClick([=](asEventArgs args) {

			Scene& scene = asScene::GetScene();
			scene.weathers.Clear();


			scene.weather = WeatherComponent();

			InvalidateProbes();

			});
		weatherWindow->AddWidget(preset0Button);

		asButton* preset1Button = new asButton("WeatherPreset - Daytime");
		preset1Button->SetTooltip("Apply this weather preset to the world.");
		preset1Button->SetSize(XMFLOAT2(240, 30));
		preset1Button->SetPos(XMFLOAT2(x - 100, y += step));
		preset1Button->OnClick([=](asEventArgs args) {

			auto& weather = GetWeather();
			weather.ambient = XMFLOAT3(0.1f, 0.1f, 0.1f);
			weather.horizon = XMFLOAT3(0.3f, 0.3f, 0.4f);
			weather.zenith = XMFLOAT3(37.0f / 255.0f, 61.0f / 255.0f, 142.0f / 255.0f);
			weather.cloudiness = 0.4f;
			weather.fogStart = 100;
			weather.fogEnd = 1000;
			weather.fogHeight = 0;

			InvalidateProbes();

			});
		weatherWindow->AddWidget(preset1Button);

		asButton* preset2Button = new asButton("WeatherPreset - Sunset");
		preset2Button->SetTooltip("Apply this weather preset to the world.");
		preset2Button->SetSize(XMFLOAT2(240, 30));
		preset2Button->SetPos(XMFLOAT2(x - 100, y += step));
		preset2Button->OnClick([=](asEventArgs args) {

			auto& weather = GetWeather();
			weather.ambient = XMFLOAT3(0.02f, 0.02f, 0.02f);
			weather.horizon = XMFLOAT3(0.2f, 0.05f, 0.15f);
			weather.zenith = XMFLOAT3(0.4f, 0.05f, 0.1f);
			weather.cloudiness = 0.36f;
			weather.fogStart = 50;
			weather.fogEnd = 600;
			weather.fogHeight = 0;

			InvalidateProbes();

			});
		weatherWindow->AddWidget(preset2Button);

		asButton* preset3Button = new asButton("WeatherPreset - Cloudy");
		preset3Button->SetTooltip("Apply this weather preset to the world.");
		preset3Button->SetSize(XMFLOAT2(240, 30));
		preset3Button->SetPos(XMFLOAT2(x - 100, y += step));
		preset3Button->OnClick([=](asEventArgs args) {

			auto& weather = GetWeather();
			weather.ambient = XMFLOAT3(0.1f, 0.1f, 0.1f);
			weather.horizon = XMFLOAT3(0.38f, 0.38f, 0.38f);
			weather.zenith = XMFLOAT3(0.42f, 0.42f, 0.42f);
			weather.cloudiness = 0.75f;
			weather.fogStart = 0;
			weather.fogEnd = 500;
			weather.fogHeight = 0;

			InvalidateProbes();

			});
		weatherWindow->AddWidget(preset3Button);

		asButton* preset4Button = new asButton("WeatherPreset - Night");
		preset4Button->SetTooltip("Apply this weather preset to the world.");
		preset4Button->SetSize(XMFLOAT2(240, 30));
		preset4Button->SetPos(XMFLOAT2(x - 100, y += step));
		preset4Button->OnClick([=](asEventArgs args) {

			auto& weather = GetWeather();
			weather.ambient = XMFLOAT3(0.01f, 0.01f, 0.02f);
			weather.horizon = XMFLOAT3(0.04f, 0.1f, 0.2f);
			weather.zenith = XMFLOAT3(0.02f, 0.04f, 0.08f);
			weather.cloudiness = 0.28f;
			weather.fogStart = 10;
			weather.fogEnd = 400;
			weather.fogHeight = 0;

			InvalidateProbes();

			});
		weatherWindow->AddWidget(preset4Button);


		asButton* eliminateCoarseCascadesButton = new asButton("HELPERSCRIPT - EliminateCoarseCascades");
		eliminateCoarseCascadesButton->SetTooltip("Eliminate the coarse cascade mask for every object in the scene.");
		eliminateCoarseCascadesButton->SetSize(XMFLOAT2(240, 30));
		eliminateCoarseCascadesButton->SetPos(XMFLOAT2(x - 100, y += step * 3));
		eliminateCoarseCascadesButton->OnClick([=](asEventArgs args) {

			Scene& scene = asScene::GetScene();
			for (size_t i = 0; i < scene.objects.GetCount(); ++i)
			{
				scene.objects[i].cascadeMask = 1;
			}

			});
		weatherWindow->AddWidget(eliminateCoarseCascadesButton);



		ambientColorPicker = new asColorPicker(GUI, "Ambient Color");
		ambientColorPicker->SetPos(XMFLOAT2(360, 40));
		ambientColorPicker->RemoveWidgets();
		ambientColorPicker->SetVisible(false);
		ambientColorPicker->SetEnabled(true);
		ambientColorPicker->OnColorChanged([&](asEventArgs args) {
			auto& weather = GetWeather();
			weather.ambient = args.color.toFloat3();
			});
		weatherWindow->AddWidget(ambientColorPicker);


		horizonColorPicker = new asColorPicker(GUI, "Horizon Color");
		horizonColorPicker->SetPos(XMFLOAT2(360, 300));
		horizonColorPicker->RemoveWidgets();
		horizonColorPicker->SetVisible(false);
		horizonColorPicker->SetEnabled(true);
		horizonColorPicker->OnColorChanged([&](asEventArgs args) {
			auto& weather = GetWeather();
			weather.horizon = args.color.toFloat3();
			});
		weatherWindow->AddWidget(horizonColorPicker);



		zenithColorPicker = new asColorPicker(GUI, "Zenith Color");
		zenithColorPicker->SetPos(XMFLOAT2(360, 560));
		zenithColorPicker->RemoveWidgets();
		zenithColorPicker->SetVisible(false);
		zenithColorPicker->SetEnabled(true);
		zenithColorPicker->OnColorChanged([&](asEventArgs args) {
			auto& weather = GetWeather();
			weather.zenith = args.color.toFloat3();
			});
		weatherWindow->AddWidget(zenithColorPicker);


		x = 840;
		y = 20;

		// Ocean params:
		ocean_enabledCheckBox = new asCheckBox("Ocean simulation enabled: ");
		ocean_enabledCheckBox->SetPos(XMFLOAT2(x + 100, y += step));
		ocean_enabledCheckBox->OnClick([&](asEventArgs args) {
			auto& weather = GetWeather();
			if (!weather.IsOceanEnabled())
			{
				weather.SetOceanEnabled(args.bValue);
				asRenderer::OceanRegenerate();
			}
			});
		weatherWindow->AddWidget(ocean_enabledCheckBox);


		ocean_patchSizeSlider = new asSlider(1, 1000, 1000, 100000, "Patch size: ");
		ocean_patchSizeSlider->SetSize(slider_size);
		ocean_patchSizeSlider->SetPos(XMFLOAT2(x, y += step));
		ocean_patchSizeSlider->SetValue(asScene::GetScene().weather.oceanParameters.patch_length);
		ocean_patchSizeSlider->SetTooltip("Adjust water tiling patch size");
		ocean_patchSizeSlider->OnSlide([&](asEventArgs args) {
			if (asScene::GetScene().weathers.GetCount() > 0)
			{
				WeatherComponent& weather = asScene::GetScene().weathers[0];
				if (std::abs(weather.oceanParameters.patch_length - args.fValue) > FLT_EPSILON)
				{
					weather.oceanParameters.patch_length = args.fValue;
					asRenderer::OceanRegenerate();
				}
			}
			});
		weatherWindow->AddWidget(ocean_patchSizeSlider);

		ocean_waveAmplitudeSlider = new asSlider(0, 1000, 1000, 100000, "Wave amplitude: ");
		ocean_waveAmplitudeSlider->SetSize(slider_size);
		ocean_waveAmplitudeSlider->SetPos(XMFLOAT2(x, y += step));
		ocean_waveAmplitudeSlider->SetValue(asScene::GetScene().weather.oceanParameters.wave_amplitude);
		ocean_waveAmplitudeSlider->SetTooltip("Adjust wave size");
		ocean_waveAmplitudeSlider->OnSlide([&](asEventArgs args) {
			if (asScene::GetScene().weathers.GetCount() > 0)
			{
				WeatherComponent& weather = asScene::GetScene().weathers[0];
				if (std::abs(weather.oceanParameters.wave_amplitude - args.fValue) > FLT_EPSILON)
				{
					weather.oceanParameters.wave_amplitude = args.fValue;
					asRenderer::OceanRegenerate();
				}
			}
			});
		weatherWindow->AddWidget(ocean_waveAmplitudeSlider);

		ocean_choppyScaleSlider = new asSlider(0, 10, 1000, 100000, "Choppiness: ");
		ocean_choppyScaleSlider->SetSize(slider_size);
		ocean_choppyScaleSlider->SetPos(XMFLOAT2(x, y += step));
		ocean_choppyScaleSlider->SetValue(asScene::GetScene().weather.oceanParameters.choppy_scale);
		ocean_choppyScaleSlider->SetTooltip("Adjust wave choppiness");
		ocean_choppyScaleSlider->OnSlide([&](asEventArgs args) {
			if (asScene::GetScene().weathers.GetCount() > 0)
			{
				WeatherComponent& weather = asScene::GetScene().weathers[0];
				weather.oceanParameters.choppy_scale = args.fValue;
			}
			});
		weatherWindow->AddWidget(ocean_choppyScaleSlider);

		ocean_windDependencySlider = new asSlider(0, 1, 1000, 100000, "Wind dependency: ");
		ocean_windDependencySlider->SetSize(slider_size);
		ocean_windDependencySlider->SetPos(XMFLOAT2(x, y += step));
		ocean_windDependencySlider->SetValue(asScene::GetScene().weather.oceanParameters.wind_dependency);
		ocean_windDependencySlider->SetTooltip("Adjust wind contribution");
		ocean_windDependencySlider->OnSlide([&](asEventArgs args) {
			if (asScene::GetScene().weathers.GetCount() > 0)
			{
				WeatherComponent& weather = asScene::GetScene().weathers[0];
				if (std::abs(weather.oceanParameters.wind_dependency - args.fValue) > FLT_EPSILON)
				{
					weather.oceanParameters.wind_dependency = args.fValue;
					asRenderer::OceanRegenerate();
				}
			}
			});
		weatherWindow->AddWidget(ocean_windDependencySlider);

		ocean_timeScaleSlider = new asSlider(0, 4, 1000, 100000, "Time scale: ");
		ocean_timeScaleSlider->SetSize(slider_size);
		ocean_timeScaleSlider->SetPos(XMFLOAT2(x, y += step));
		ocean_timeScaleSlider->SetValue(asScene::GetScene().weather.oceanParameters.time_scale);
		ocean_timeScaleSlider->SetTooltip("Adjust simulation speed");
		ocean_timeScaleSlider->OnSlide([&](asEventArgs args) {
			if (asScene::GetScene().weathers.GetCount() > 0)
			{
				WeatherComponent& weather = asScene::GetScene().weathers[0];
				weather.oceanParameters.time_scale = args.fValue;
			}
			});
		weatherWindow->AddWidget(ocean_timeScaleSlider);

		ocean_heightSlider = new asSlider(-100, 100, 0, 100000, "Water level: ");
		ocean_heightSlider->SetSize(slider_size);
		ocean_heightSlider->SetPos(XMFLOAT2(x, y += step));
		ocean_heightSlider->SetValue(0);
		ocean_heightSlider->SetTooltip("Adjust water level");
		ocean_heightSlider->OnSlide([&](asEventArgs args) {
			if (asScene::GetScene().weathers.GetCount() > 0)
			{
				WeatherComponent& weather = asScene::GetScene().weathers[0];
				weather.oceanParameters.waterHeight = args.fValue;
			}
			});
		weatherWindow->AddWidget(ocean_heightSlider);

		ocean_detailSlider = new asSlider(1, 10, 0, 9, "Surface Detail: ");
		ocean_detailSlider->SetSize(slider_size);
		ocean_detailSlider->SetPos(XMFLOAT2(x, y += step));
		ocean_detailSlider->SetValue(4);
		ocean_detailSlider->SetTooltip("Adjust surface tessellation resolution. High values can decrease performance.");
		ocean_detailSlider->OnSlide([&](asEventArgs args) {
			if (asScene::GetScene().weathers.GetCount() > 0)
			{
				WeatherComponent& weather = asScene::GetScene().weathers[0];
				weather.oceanParameters.surfaceDetail = (uint32_t)args.iValue;
			}
			});
		weatherWindow->AddWidget(ocean_detailSlider);

		ocean_toleranceSlider = new asSlider(1, 10, 0, 1000, "Displacement Tolerance: ");
		ocean_toleranceSlider->SetSize(slider_size);
		ocean_toleranceSlider->SetPos(XMFLOAT2(x, y += step));
		ocean_toleranceSlider->SetValue(2);
		ocean_toleranceSlider->SetTooltip("Big waves can introduce glitches on screen borders, this can fix that but surface detail will decrease.");
		ocean_toleranceSlider->OnSlide([&](asEventArgs args) {
			if (asScene::GetScene().weathers.GetCount() > 0)
			{
				WeatherComponent& weather = asScene::GetScene().weathers[0];
				weather.oceanParameters.surfaceDisplacementTolerance = args.fValue;
			}
			});
		weatherWindow->AddWidget(ocean_toleranceSlider);


		ocean_colorPicker = new asColorPicker(GUI, "Water Color");
		ocean_colorPicker->SetPos(XMFLOAT2(x - 160, y += step));
		ocean_colorPicker->RemoveWidgets();
		ocean_colorPicker->SetVisible(true);
		ocean_colorPicker->SetEnabled(true);
		ocean_colorPicker->OnColorChanged([&](asEventArgs args) {
			if (asScene::GetScene().weathers.GetCount() > 0)
			{
				WeatherComponent& weather = asScene::GetScene().weathers[0];
				weather.oceanParameters.waterColor = args.color.toFloat3();
			}
			});
		weatherWindow->AddWidget(ocean_colorPicker);

		step += ocean_colorPicker->GetScale().y;
		ocean_resetButton = new asButton("Reset Ocean to default");
		ocean_resetButton->SetTooltip("Reset ocean to default values.");
		ocean_resetButton->SetSize(XMFLOAT2(240, 30));
		ocean_resetButton->SetPos(XMFLOAT2(x - 100, y += step));
		ocean_resetButton->OnClick([=](asEventArgs args) {
			auto& weather = GetWeather();
			weather.oceanParameters = WeatherComponent::OceanParameters();
			});
		weatherWindow->AddWidget(ocean_resetButton);





		weatherWindow->Translate(XMFLOAT3(130, 30, 0));
		weatherWindow->SetVisible(false);
	}


	WeatherWindow::~WeatherWindow()
	{
		weatherWindow->RemoveWidgets(true);
		GUI->RemoveWidget(weatherWindow);
		SAFE_DELETE(weatherWindow);
	}

	void WeatherWindow::Update()
	{
		Scene& scene = asScene::GetScene();
		if (scene.weathers.GetCount() > 0)
		{
			auto& weather = scene.weathers[0];

			fogStartSlider->SetValue(weather.fogStart);
			fogEndSlider->SetValue(weather.fogEnd);
			fogHeightSlider->SetValue(weather.fogHeight);
			cloudinessSlider->SetValue(weather.cloudiness);
			cloudScaleSlider->SetValue(weather.cloudScale);
			cloudSpeedSlider->SetValue(weather.cloudSpeed);

			ambientColorPicker->SetPickColor(asColor::fromFloat3(weather.ambient));
			horizonColorPicker->SetPickColor(asColor::fromFloat3(weather.horizon));
			zenithColorPicker->SetPickColor(asColor::fromFloat3(weather.zenith));


			ocean_enabledCheckBox->SetCheck(weather.IsOceanEnabled());
			ocean_patchSizeSlider->SetValue(weather.oceanParameters.patch_length);
			ocean_waveAmplitudeSlider->SetValue(weather.oceanParameters.wave_amplitude);
			ocean_choppyScaleSlider->SetValue(weather.oceanParameters.choppy_scale);
			ocean_windDependencySlider->SetValue(weather.oceanParameters.wind_dependency);
			ocean_timeScaleSlider->SetValue(weather.oceanParameters.time_scale);
			ocean_heightSlider->SetValue(weather.oceanParameters.waterHeight);
			ocean_detailSlider->SetValue((float)weather.oceanParameters.surfaceDetail);
			ocean_toleranceSlider->SetValue(weather.oceanParameters.surfaceDisplacementTolerance);
			ocean_colorPicker->SetPickColor(asColor::fromFloat3(weather.oceanParameters.waterColor));
		}
	}

	WeatherComponent& WeatherWindow::GetWeather() const
	{
		Scene& scene = asScene::GetScene();
		if (scene.weathers.GetCount() == 0)
		{
			scene.weathers.Create(CreateEntity());
		}
		return scene.weathers[0];
	}

	void WeatherWindow::InvalidateProbes() const
	{
		Scene& scene = asScene::GetScene();

		// Also, we invalidate all environment probes to reflect the sky changes.
		for (size_t i = 0; i < scene.probes.GetCount(); ++i)
		{
			scene.probes[i].SetDirty();
		}
	}
}
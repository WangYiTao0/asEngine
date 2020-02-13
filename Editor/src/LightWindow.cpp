#include <asEngine.h>
#include "LightWindow.h"

#include <string>
namespace as
{

	using namespace asECS;
	using namespace asGraphics;
	using namespace asScene;


	LightWindow::LightWindow(asGUI* gui) : GUI(gui)
	{
		assert(GUI && "Invalid GUI!");

		float screenW = (float)asRenderer::GetDevice()->GetScreenWidth();
		float screenH = (float)asRenderer::GetDevice()->GetScreenHeight();

		lightWindow = new asWindow(GUI, "Light Window");
		lightWindow->SetSize(XMFLOAT2(650, 520));
		GUI->AddWidget(lightWindow);

		float x = 450;
		float y = 0;
		float step = 35;

		energySlider = new asSlider(0.1f, 64, 0, 100000, "Energy: ");
		energySlider->SetSize(XMFLOAT2(100, 30));
		energySlider->SetPos(XMFLOAT2(x, y += step));
		energySlider->OnSlide([&](asEventArgs args) {
			LightComponent* light = asScene::GetScene().lights.GetComponent(entity);
			if (light != nullptr)
			{
				light->energy = args.fValue;
			}
			});
		energySlider->SetEnabled(false);
		energySlider->SetTooltip("Adjust the light radiation amount inside the maximum range");
		lightWindow->AddWidget(energySlider);

		rangeSlider = new asSlider(1, 1000, 0, 100000, "Range: ");
		rangeSlider->SetSize(XMFLOAT2(100, 30));
		rangeSlider->SetPos(XMFLOAT2(x, y += step));
		rangeSlider->OnSlide([&](asEventArgs args) {
			LightComponent* light = asScene::GetScene().lights.GetComponent(entity);
			if (light != nullptr)
			{
				light->range_local = args.fValue;
			}
			});
		rangeSlider->SetEnabled(false);
		rangeSlider->SetTooltip("Adjust the maximum range the light can affect.");
		lightWindow->AddWidget(rangeSlider);

		radiusSlider = new asSlider(0.01f, 10, 0, 100000, "Radius: ");
		radiusSlider->SetSize(XMFLOAT2(100, 30));
		radiusSlider->SetPos(XMFLOAT2(x, y += step));
		radiusSlider->OnSlide([&](asEventArgs args) {
			LightComponent* light = asScene::GetScene().lights.GetComponent(entity);
			if (light != nullptr)
			{
				light->radius = args.fValue;
			}
			});
		radiusSlider->SetEnabled(false);
		radiusSlider->SetTooltip("Adjust the radius of an area light.");
		lightWindow->AddWidget(radiusSlider);

		widthSlider = new asSlider(1, 10, 0, 100000, "Width: ");
		widthSlider->SetSize(XMFLOAT2(100, 30));
		widthSlider->SetPos(XMFLOAT2(x, y += step));
		widthSlider->OnSlide([&](asEventArgs args) {
			LightComponent* light = asScene::GetScene().lights.GetComponent(entity);
			if (light != nullptr)
			{
				light->width = args.fValue;
			}
			});
		widthSlider->SetEnabled(false);
		widthSlider->SetTooltip("Adjust the width of an area light.");
		lightWindow->AddWidget(widthSlider);

		heightSlider = new asSlider(1, 10, 0, 100000, "Height: ");
		heightSlider->SetSize(XMFLOAT2(100, 30));
		heightSlider->SetPos(XMFLOAT2(x, y += step));
		heightSlider->OnSlide([&](asEventArgs args) {
			LightComponent* light = asScene::GetScene().lights.GetComponent(entity);
			if (light != nullptr)
			{
				light->height = args.fValue;
			}
			});
		heightSlider->SetEnabled(false);
		heightSlider->SetTooltip("Adjust the height of an area light.");
		lightWindow->AddWidget(heightSlider);

		fovSlider = new asSlider(0.1f, XM_PI - 0.01f, 0, 100000, "FOV: ");
		fovSlider->SetSize(XMFLOAT2(100, 30));
		fovSlider->SetPos(XMFLOAT2(x, y += step));
		fovSlider->OnSlide([&](asEventArgs args) {
			LightComponent* light = asScene::GetScene().lights.GetComponent(entity);
			if (light != nullptr)
			{
				light->fov = args.fValue;
			}
			});
		fovSlider->SetEnabled(false);
		fovSlider->SetTooltip("Adjust the cone aperture for spotlight.");
		lightWindow->AddWidget(fovSlider);

		biasSlider = new asSlider(0.0f, 0.2f, 0, 100000, "ShadowBias: ");
		biasSlider->SetSize(XMFLOAT2(100, 30));
		biasSlider->SetPos(XMFLOAT2(x, y += step));
		biasSlider->OnSlide([&](asEventArgs args) {
			LightComponent* light = asScene::GetScene().lights.GetComponent(entity);
			if (light != nullptr)
			{
				light->shadowBias = args.fValue;
			}
			});
		biasSlider->SetEnabled(false);
		biasSlider->SetTooltip("Adjust the shadow bias if shadow artifacts occur.");
		lightWindow->AddWidget(biasSlider);

		shadowCheckBox = new asCheckBox("Shadow: ");
		shadowCheckBox->SetPos(XMFLOAT2(x, y += step));
		shadowCheckBox->OnClick([&](asEventArgs args) {
			LightComponent* light = asScene::GetScene().lights.GetComponent(entity);
			if (light != nullptr)
			{
				light->SetCastShadow(args.bValue);
			}
			});
		shadowCheckBox->SetEnabled(false);
		shadowCheckBox->SetTooltip("Set light as shadow caster. Many shadow casters can affect performance!");
		lightWindow->AddWidget(shadowCheckBox);

		volumetricsCheckBox = new asCheckBox("Volumetric Scattering: ");
		volumetricsCheckBox->SetPos(XMFLOAT2(x, y += step));
		volumetricsCheckBox->OnClick([&](asEventArgs args) {
			LightComponent* light = asScene::GetScene().lights.GetComponent(entity);
			if (light != nullptr)
			{
				light->SetVolumetricsEnabled(args.bValue);
			}
			});
		volumetricsCheckBox->SetEnabled(false);
		volumetricsCheckBox->SetTooltip("Compute volumetric light scattering effect. The scattering is modulated by fog settings!");
		lightWindow->AddWidget(volumetricsCheckBox);

		haloCheckBox = new asCheckBox("Visualizer: ");
		haloCheckBox->SetPos(XMFLOAT2(x, y += step));
		haloCheckBox->OnClick([&](asEventArgs args) {
			LightComponent* light = asScene::GetScene().lights.GetComponent(entity);
			if (light != nullptr)
			{
				light->SetVisualizerEnabled(args.bValue);
			}
			});
		haloCheckBox->SetEnabled(false);
		haloCheckBox->SetTooltip("Visualize light source emission");
		lightWindow->AddWidget(haloCheckBox);

		staticCheckBox = new asCheckBox("Static: ");
		staticCheckBox->SetPos(XMFLOAT2(x, y += step));
		staticCheckBox->OnClick([&](asEventArgs args) {
			LightComponent* light = asScene::GetScene().lights.GetComponent(entity);
			if (light != nullptr)
			{
				light->SetStatic(args.bValue);
			}
			});
		staticCheckBox->SetEnabled(false);
		staticCheckBox->SetTooltip("Static lights will only be used for baking into lightmaps.");
		lightWindow->AddWidget(staticCheckBox);

		addLightButton = new asButton("Add Light");
		addLightButton->SetPos(XMFLOAT2(x, y += step));
		addLightButton->SetSize(XMFLOAT2(150, 30));
		addLightButton->OnClick([&](asEventArgs args) {
			Entity entity = asScene::GetScene().Entity_CreateLight("editorLight", XMFLOAT3(0, 3, 0), XMFLOAT3(1, 1, 1), 2, 60);
			LightComponent* light = asScene::GetScene().lights.GetComponent(entity);
			if (light != nullptr)
			{
				light->type = (LightComponent::LightType)typeSelectorComboBox->GetSelected();
			}
			else
			{
				assert(0);
			}
			});
		addLightButton->SetTooltip("Add a light to the scene.");
		lightWindow->AddWidget(addLightButton);


		colorPicker = new asColorPicker(GUI, "Light Color");
		colorPicker->SetPos(XMFLOAT2(10, 30));
		colorPicker->RemoveWidgets();
		colorPicker->SetVisible(true);
		colorPicker->SetEnabled(false);
		colorPicker->OnColorChanged([&](asEventArgs args) {
			LightComponent* light = asScene::GetScene().lights.GetComponent(entity);
			if (light != nullptr)
			{
				light->color = args.color.toFloat3();
			}
			});
		lightWindow->AddWidget(colorPicker);

		typeSelectorComboBox = new asComboBox("Type: ");
		typeSelectorComboBox->SetPos(XMFLOAT2(x, y += step));
		typeSelectorComboBox->OnSelect([&](asEventArgs args) {
			LightComponent* light = asScene::GetScene().lights.GetComponent(entity);
			if (light != nullptr && args.iValue >= 0)
			{
				light->SetType((LightComponent::LightType)args.iValue);
				SetLightType(light->GetType());
				biasSlider->SetValue(light->shadowBias);
			}
			});
		typeSelectorComboBox->AddItem("Directional");
		typeSelectorComboBox->AddItem("Point");
		typeSelectorComboBox->AddItem("Spot");
		typeSelectorComboBox->AddItem("Sphere");
		typeSelectorComboBox->AddItem("Disc");
		typeSelectorComboBox->AddItem("Rectangle");
		typeSelectorComboBox->AddItem("Tube");
		typeSelectorComboBox->SetTooltip("Choose the light source type...");
		typeSelectorComboBox->SetSelected((int)LightComponent::POINT);
		lightWindow->AddWidget(typeSelectorComboBox);



		x = 10;
		y = 280;
		step = 25;

		lensflare_Label = new asLabel("Lens flare textures: ");
		lensflare_Label->SetPos(XMFLOAT2(x, y += step));
		lensflare_Label->SetSize(XMFLOAT2(140, 20));
		lightWindow->AddWidget(lensflare_Label);

		for (size_t i = 0; i < arraysize(lensflare_Button); ++i)
		{
			lensflare_Button[i] = new asButton("LensFlareSlot");
			lensflare_Button[i]->SetText("");
			lensflare_Button[i]->SetTooltip("Load a lensflare texture to this slot");
			lensflare_Button[i]->SetPos(XMFLOAT2(x, y += step));
			lensflare_Button[i]->SetSize(XMFLOAT2(260, 20));
			lensflare_Button[i]->OnClick([=](asEventArgs args) {
				LightComponent* light = asScene::GetScene().lights.GetComponent(entity);
				if (light == nullptr)
					return;

				if (light->lensFlareRimTextures.size() <= i)
				{
					light->lensFlareRimTextures.resize(i + 1);
					light->lensFlareNames.resize(i + 1);
				}

				if (light->lensFlareRimTextures[i] != nullptr)
				{
					light->lensFlareNames[i] = "";
					light->lensFlareRimTextures[i] = nullptr;
					lensflare_Button[i]->SetText("");
				}
				else
				{
					asHelper::FileDialogParams params;
					asHelper::FileDialogResult result;
					params.type = asHelper::FileDialogParams::OPEN;
					params.description = "Texture";
					params.extensions.push_back("dds");
					params.extensions.push_back("png");
					params.extensions.push_back("jpg");
					params.extensions.push_back("tga");
					asHelper::FileDialog(params, result);

					if (result.ok) {
						std::string fileName = result.filenames.front();
						light->lensFlareRimTextures[i] = asResourceManager::Load(fileName);
						light->lensFlareNames[i] = fileName;
						fileName = asHelper::GetFileNameFromPath(fileName);
						lensflare_Button[i]->SetText(fileName);
					}
				}
				});
			lightWindow->AddWidget(lensflare_Button[i]);
		}


		lightWindow->Translate(XMFLOAT3(120, 30, 0));
		lightWindow->SetVisible(false);

		SetEntity(INVALID_ENTITY);
	}


	LightWindow::~LightWindow()
	{
		lightWindow->RemoveWidgets(true);
		GUI->RemoveWidget(lightWindow);
		SAFE_DELETE(lightWindow);
	}

	void LightWindow::SetEntity(Entity entity)
	{
		this->entity = entity;

		const LightComponent* light = asScene::GetScene().lights.GetComponent(entity);

		if (light != nullptr)
		{
			energySlider->SetEnabled(true);
			energySlider->SetValue(light->energy);
			rangeSlider->SetValue(light->range_local);
			radiusSlider->SetValue(light->radius);
			widthSlider->SetValue(light->width);
			heightSlider->SetValue(light->height);
			fovSlider->SetValue(light->fov);
			biasSlider->SetEnabled(true);
			biasSlider->SetValue(light->shadowBias);
			shadowCheckBox->SetEnabled(true);
			shadowCheckBox->SetCheck(light->IsCastingShadow());
			haloCheckBox->SetEnabled(true);
			haloCheckBox->SetCheck(light->IsVisualizerEnabled());
			volumetricsCheckBox->SetEnabled(true);
			volumetricsCheckBox->SetCheck(light->IsVolumetricsEnabled());
			staticCheckBox->SetEnabled(true);
			staticCheckBox->SetCheck(light->IsStatic());
			colorPicker->SetEnabled(true);
			colorPicker->SetPickColor(asColor::fromFloat3(light->color));
			typeSelectorComboBox->SetSelected((int)light->GetType());

			SetLightType(light->GetType());

			for (size_t i = 0; i < arraysize(lensflare_Button); ++i)
			{
				if (light->lensFlareRimTextures.size() > i&& light->lensFlareRimTextures[i] && !light->lensFlareNames[i].empty())
				{
					lensflare_Button[i]->SetText(light->lensFlareNames[i]);
				}
				else
				{
					lensflare_Button[i]->SetText("");
				}
				lensflare_Button[i]->SetEnabled(true);
			}
		}
		else
		{
			rangeSlider->SetEnabled(false);
			radiusSlider->SetEnabled(false);
			widthSlider->SetEnabled(false);
			heightSlider->SetEnabled(false);
			fovSlider->SetEnabled(false);
			biasSlider->SetEnabled(false);
			shadowCheckBox->SetEnabled(false);
			haloCheckBox->SetEnabled(false);
			volumetricsCheckBox->SetEnabled(false);
			staticCheckBox->SetEnabled(false);
			energySlider->SetEnabled(false);
			colorPicker->SetEnabled(false);

			for (size_t i = 0; i < arraysize(lensflare_Button); ++i)
			{
				lensflare_Button[i]->SetEnabled(false);
			}
		}
	}
	void LightWindow::SetLightType(LightComponent::LightType type)
	{
		if (type == LightComponent::DIRECTIONAL)
		{
			rangeSlider->SetEnabled(false);
			fovSlider->SetEnabled(false);
		}
		else
		{
			if (type == LightComponent::SPHERE || type == LightComponent::DISC || type == LightComponent::RECTANGLE || type == LightComponent::TUBE)
			{
				rangeSlider->SetEnabled(false);
				radiusSlider->SetEnabled(true);
				widthSlider->SetEnabled(true);
				heightSlider->SetEnabled(true);
				fovSlider->SetEnabled(false);
			}
			else
			{
				rangeSlider->SetEnabled(true);
				radiusSlider->SetEnabled(false);
				widthSlider->SetEnabled(false);
				heightSlider->SetEnabled(false);
				if (type == LightComponent::SPOT)
				{
					fovSlider->SetEnabled(true);
				}
				else
				{
					fovSlider->SetEnabled(false);
				}
			}
		}

	}
}

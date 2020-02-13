#include <asEngine.h>
#include "PostprocessWindow.h"

#include <thread>

using namespace std;

namespace as
{
	using namespace asGraphics;


	PostprocessWindow::PostprocessWindow(asGUI* gui, RenderPath3D* comp) : GUI(gui), component(comp)
	{
		assert(component && "PostprocessWnd invalid component!");
		assert(GUI && "Invalid GUI!");

		float screenW = (float)asRenderer::GetDevice()->GetScreenWidth();
		float screenH = (float)asRenderer::GetDevice()->GetScreenHeight();

		ppWindow = new asWindow(GUI, "PostProcess Window");
		ppWindow->SetSize(XMFLOAT2(400, 740));
		GUI->AddWidget(ppWindow);

		float x = 150;
		float y = 0;

		exposureSlider = new asSlider(0.0f, 3.0f, 1, 10000, "Exposure: ");
		exposureSlider->SetTooltip("Set the tonemap exposure value");
		exposureSlider->SetScriptTip("RenderPath3D::SetExposure(float value)");
		exposureSlider->SetSize(XMFLOAT2(100, 20));
		exposureSlider->SetPos(XMFLOAT2(x, y += 35));
		exposureSlider->SetValue(component->getExposure());
		exposureSlider->OnSlide([&](asEventArgs args) {
			component->setExposure(args.fValue);
			});
		ppWindow->AddWidget(exposureSlider);

		lensFlareCheckBox = new asCheckBox("LensFlare: ");
		lensFlareCheckBox->SetTooltip("Toggle visibility of light source flares. Additional setup needed per light for a lensflare to be visible.");
		lensFlareCheckBox->SetScriptTip("RenderPath3D::SetLensFlareEnabled(bool value)");
		lensFlareCheckBox->SetPos(XMFLOAT2(x, y += 35));
		lensFlareCheckBox->SetCheck(component->getLensFlareEnabled());
		lensFlareCheckBox->OnClick([&](asEventArgs args) {
			component->setLensFlareEnabled(args.bValue);
			});
		ppWindow->AddWidget(lensFlareCheckBox);

		lightShaftsCheckBox = new asCheckBox("LightShafts: ");
		lightShaftsCheckBox->SetTooltip("Enable light shaft for directional light sources.");
		lightShaftsCheckBox->SetScriptTip("RenderPath3D::SetLightShaftsEnabled(bool value)");
		lightShaftsCheckBox->SetPos(XMFLOAT2(x, y += 35));
		lightShaftsCheckBox->SetCheck(component->getLightShaftsEnabled());
		lightShaftsCheckBox->OnClick([&](asEventArgs args) {
			component->setLightShaftsEnabled(args.bValue);
			});
		ppWindow->AddWidget(lightShaftsCheckBox);

		ssaoCheckBox = new asCheckBox("SSAO: ");
		ssaoCheckBox->SetTooltip("Enable Screen Space Ambient Occlusion.");
		ssaoCheckBox->SetScriptTip("RenderPath3D::SetSSAOEnabled(bool value)");
		ssaoCheckBox->SetPos(XMFLOAT2(x, y += 35));
		ssaoCheckBox->SetCheck(component->getSSAOEnabled());
		ssaoCheckBox->OnClick([&](asEventArgs args) {
			component->setSSAOEnabled(args.bValue);
			});
		ppWindow->AddWidget(ssaoCheckBox);

		ssaoRangeSlider = new asSlider(0, 2, 1, 1000, "Range: ");
		ssaoRangeSlider->SetTooltip("Set SSAO Detection range.");
		ssaoRangeSlider->SetSize(XMFLOAT2(100, 20));
		ssaoRangeSlider->SetPos(XMFLOAT2(x + 100, y));
		ssaoRangeSlider->SetValue(component->getSSAORange());
		ssaoRangeSlider->OnSlide([&](asEventArgs args) {
			component->setSSAORange(args.fValue);
			});
		ppWindow->AddWidget(ssaoRangeSlider);

		ssaoSampleCountSlider = new asSlider(9, 64, 16, 64 - 9, "SampleCount: ");
		ssaoSampleCountSlider->SetTooltip("Set SSAO Sample Count. Higher values produce better quality, but slower to compute");
		ssaoSampleCountSlider->SetSize(XMFLOAT2(100, 20));
		ssaoSampleCountSlider->SetPos(XMFLOAT2(x + 100, y += 35));
		ssaoSampleCountSlider->SetValue((float)component->getSSAOSampleCount());
		ssaoSampleCountSlider->OnSlide([&](asEventArgs args) {
			component->setSSAOSampleCount((UINT)args.iValue);
			});
		ppWindow->AddWidget(ssaoSampleCountSlider);

		ssaoPowerSlider = new asSlider(0.25f, 8.0f, 2, 1000, "Power: ");
		ssaoPowerSlider->SetTooltip("Set SSAO Power. Higher values produce darker, more pronounced effect");
		ssaoPowerSlider->SetSize(XMFLOAT2(100, 20));
		ssaoPowerSlider->SetPos(XMFLOAT2(x + 100, y += 35));
		ssaoPowerSlider->SetValue((float)component->getSSAOPower());
		ssaoPowerSlider->OnSlide([&](asEventArgs args) {
			component->setSSAOPower(args.fValue);
			});
		ppWindow->AddWidget(ssaoPowerSlider);

		ssrCheckBox = new asCheckBox("SSR: ");
		ssrCheckBox->SetTooltip("Enable Screen Space Reflections.");
		ssrCheckBox->SetScriptTip("RenderPath3D::SetSSREnabled(bool value)");
		ssrCheckBox->SetPos(XMFLOAT2(x, y += 35));
		ssrCheckBox->SetCheck(component->getSSREnabled());
		ssrCheckBox->OnClick([&](asEventArgs args) {
			component->setSSREnabled(args.bValue);
			});
		ppWindow->AddWidget(ssrCheckBox);

		sssCheckBox = new asCheckBox("SSS: ");
		sssCheckBox->SetTooltip("Enable Subsurface Scattering. (Deferred only for now)");
		sssCheckBox->SetScriptTip("RenderPath3D::SetSSSEnabled(bool value)");
		sssCheckBox->SetPos(XMFLOAT2(x, y += 35));
		sssCheckBox->SetCheck(component->getSSSEnabled());
		sssCheckBox->OnClick([&](asEventArgs args) {
			component->setSSSEnabled(args.bValue);
			});
		ppWindow->AddWidget(sssCheckBox);

		eyeAdaptionCheckBox = new asCheckBox("EyeAdaption: ");
		eyeAdaptionCheckBox->SetTooltip("Enable eye adaption for the overall screen luminance");
		eyeAdaptionCheckBox->SetPos(XMFLOAT2(x, y += 35));
		eyeAdaptionCheckBox->SetCheck(component->getEyeAdaptionEnabled());
		eyeAdaptionCheckBox->OnClick([&](asEventArgs args) {
			component->setEyeAdaptionEnabled(args.bValue);
			});
		ppWindow->AddWidget(eyeAdaptionCheckBox);

		motionBlurCheckBox = new asCheckBox("MotionBlur: ");
		motionBlurCheckBox->SetTooltip("Enable motion blur for camera movement and animated meshes.");
		motionBlurCheckBox->SetScriptTip("RenderPath3D::SetMotionBlurEnabled(bool value)");
		motionBlurCheckBox->SetPos(XMFLOAT2(x, y += 35));
		motionBlurCheckBox->SetCheck(component->getMotionBlurEnabled());
		motionBlurCheckBox->OnClick([&](asEventArgs args) {
			component->setMotionBlurEnabled(args.bValue);
			});
		ppWindow->AddWidget(motionBlurCheckBox);

		motionBlurStrengthSlider = new asSlider(0.1f, 400, 100, 10000, "Strength: ");
		motionBlurStrengthSlider->SetTooltip("Set the camera shutter speed for motion blur (higher value means stronger blur).");
		motionBlurStrengthSlider->SetScriptTip("RenderPath3D::SetMotionBlurStrength(float value)");
		motionBlurStrengthSlider->SetSize(XMFLOAT2(100, 20));
		motionBlurStrengthSlider->SetPos(XMFLOAT2(x + 100, y));
		motionBlurStrengthSlider->SetValue(component->getMotionBlurStrength());
		motionBlurStrengthSlider->OnSlide([&](asEventArgs args) {
			component->setMotionBlurStrength(args.fValue);
			});
		ppWindow->AddWidget(motionBlurStrengthSlider);

		depthOfFieldCheckBox = new asCheckBox("DepthOfField: ");
		depthOfFieldCheckBox->SetTooltip("Enable Depth of field effect. Additional focus and strength setup required.");
		depthOfFieldCheckBox->SetScriptTip("RenderPath3D::SetDepthOfFieldEnabled(bool value)");
		depthOfFieldCheckBox->SetPos(XMFLOAT2(x, y += 35));
		depthOfFieldCheckBox->SetCheck(component->getDepthOfFieldEnabled());
		depthOfFieldCheckBox->OnClick([&](asEventArgs args) {
			component->setDepthOfFieldEnabled(args.bValue);
			});
		ppWindow->AddWidget(depthOfFieldCheckBox);

		depthOfFieldFocusSlider = new asSlider(0.1f, 100, 10, 10000, "Focus: ");
		depthOfFieldFocusSlider->SetTooltip("Set the focus distance from the camera. The picture will be sharper near the focus, and blurrier further from it.");
		depthOfFieldFocusSlider->SetScriptTip("RenderPath3D::SetDepthOfFieldFocus(float value)");
		depthOfFieldFocusSlider->SetSize(XMFLOAT2(100, 20));
		depthOfFieldFocusSlider->SetPos(XMFLOAT2(x + 100, y));
		depthOfFieldFocusSlider->SetValue(component->getDepthOfFieldFocus());
		depthOfFieldFocusSlider->OnSlide([&](asEventArgs args) {
			component->setDepthOfFieldFocus(args.fValue);
			});
		ppWindow->AddWidget(depthOfFieldFocusSlider);

		depthOfFieldScaleSlider = new asSlider(0.01f, 4, 100, 1000, "Scale: ");
		depthOfFieldScaleSlider->SetTooltip("Set depth of field scale/falloff.");
		depthOfFieldScaleSlider->SetScriptTip("RenderPath3D::SetDepthOfFieldStrength(float value)");
		depthOfFieldScaleSlider->SetSize(XMFLOAT2(100, 20));
		depthOfFieldScaleSlider->SetPos(XMFLOAT2(x + 100, y += 35));
		depthOfFieldScaleSlider->SetValue(component->getDepthOfFieldStrength());
		depthOfFieldScaleSlider->OnSlide([&](asEventArgs args) {
			component->setDepthOfFieldStrength(args.fValue);
			});
		ppWindow->AddWidget(depthOfFieldScaleSlider);

		depthOfFieldAspectSlider = new asSlider(0.01f, 2, 1, 1000, "Aspect: ");
		depthOfFieldAspectSlider->SetTooltip("Set depth of field bokeh aspect ratio (width/height).");
		depthOfFieldAspectSlider->SetScriptTip("RenderPath3D::SetDepthOfFieldAspect(float value)");
		depthOfFieldAspectSlider->SetSize(XMFLOAT2(100, 20));
		depthOfFieldAspectSlider->SetPos(XMFLOAT2(x + 100, y += 35));
		depthOfFieldAspectSlider->SetValue(component->getDepthOfFieldAspect());
		depthOfFieldAspectSlider->OnSlide([&](asEventArgs args) {
			component->setDepthOfFieldAspect(args.fValue);
			});
		ppWindow->AddWidget(depthOfFieldAspectSlider);

		bloomCheckBox = new asCheckBox("Bloom: ");
		bloomCheckBox->SetTooltip("Enable bloom. The effect adds color bleeding to the brightest parts of the scene.");
		bloomCheckBox->SetScriptTip("RenderPath3D::SetBloomEnabled(bool value)");
		bloomCheckBox->SetPos(XMFLOAT2(x, y += 35));
		bloomCheckBox->SetCheck(component->getBloomEnabled());
		bloomCheckBox->OnClick([&](asEventArgs args) {
			component->setBloomEnabled(args.bValue);
			});
		ppWindow->AddWidget(bloomCheckBox);

		bloomStrengthSlider = new asSlider(0.0f, 10, 1, 1000, "Threshold: ");
		bloomStrengthSlider->SetTooltip("Set bloom threshold. The values below this will not glow on the screen.");
		bloomStrengthSlider->SetSize(XMFLOAT2(100, 20));
		bloomStrengthSlider->SetPos(XMFLOAT2(x + 100, y));
		bloomStrengthSlider->SetValue(component->getBloomThreshold());
		bloomStrengthSlider->OnSlide([&](asEventArgs args) {
			component->setBloomThreshold(args.fValue);
			});
		ppWindow->AddWidget(bloomStrengthSlider);

		fxaaCheckBox = new asCheckBox("FXAA: ");
		fxaaCheckBox->SetTooltip("Fast Approximate Anti Aliasing. A fast antialiasing method, but can be a bit too blurry.");
		fxaaCheckBox->SetScriptTip("RenderPath3D::SetFXAAEnabled(bool value)");
		fxaaCheckBox->SetPos(XMFLOAT2(x, y += 35));
		fxaaCheckBox->SetCheck(component->getFXAAEnabled());
		fxaaCheckBox->OnClick([&](asEventArgs args) {
			component->setFXAAEnabled(args.bValue);
			});
		ppWindow->AddWidget(fxaaCheckBox);

		colorGradingCheckBox = new asCheckBox("Color Grading: ");
		colorGradingCheckBox->SetTooltip("Enable color grading of the final render. An additional lookup texture must be set for it to take effect.");
		colorGradingCheckBox->SetScriptTip("RenderPath3D::SetColorGradingEnabled(bool value)");
		colorGradingCheckBox->SetPos(XMFLOAT2(x, y += 35));
		colorGradingCheckBox->SetCheck(component->getColorGradingEnabled());
		colorGradingCheckBox->OnClick([&](asEventArgs args) {
			component->setColorGradingEnabled(args.bValue);
			});
		ppWindow->AddWidget(colorGradingCheckBox);

		colorGradingButton = new asButton("Load Color Grading LUT...");
		colorGradingButton->SetTooltip("Load a color grading lookup texture...");
		colorGradingButton->SetPos(XMFLOAT2(x + 35, y));
		colorGradingButton->SetSize(XMFLOAT2(200, 18));
		colorGradingButton->OnClick([=](asEventArgs args) {
			auto x = component->getColorGradingTexture();

			if (x == nullptr)
			{
				thread([&] {
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
						string fileName = result.filenames.front();
						component->setColorGradingTexture(asResourceManager::Load(fileName));
						if (component->getColorGradingTexture() != nullptr)
						{
							colorGradingButton->SetText(fileName);
						}
					}
					}).detach();
			}
			else
			{
				component->setColorGradingTexture(nullptr);
				colorGradingButton->SetText("Load Color Grading LUT...");
			}

			});
		ppWindow->AddWidget(colorGradingButton);

		sharpenFilterCheckBox = new asCheckBox("Sharpen Filter: ");
		sharpenFilterCheckBox->SetTooltip("Toggle sharpening post process of the final image.");
		sharpenFilterCheckBox->SetScriptTip("RenderPath3D::SetSharpenFilterEnabled(bool value)");
		sharpenFilterCheckBox->SetPos(XMFLOAT2(x, y += 35));
		sharpenFilterCheckBox->SetCheck(component->getSharpenFilterEnabled());
		sharpenFilterCheckBox->OnClick([&](asEventArgs args) {
			component->setSharpenFilterEnabled(args.bValue);
			});
		ppWindow->AddWidget(sharpenFilterCheckBox);

		sharpenFilterAmountSlider = new asSlider(0, 4, 1, 1000, "Amount: ");
		sharpenFilterAmountSlider->SetTooltip("Set sharpness filter strength.");
		sharpenFilterAmountSlider->SetScriptTip("RenderPath3D::SetSharpenFilterAmount(float value)");
		sharpenFilterAmountSlider->SetSize(XMFLOAT2(100, 20));
		sharpenFilterAmountSlider->SetPos(XMFLOAT2(x + 100, y));
		sharpenFilterAmountSlider->SetValue(component->getSharpenFilterAmount());
		sharpenFilterAmountSlider->OnSlide([&](asEventArgs args) {
			component->setSharpenFilterAmount(args.fValue);
			});
		ppWindow->AddWidget(sharpenFilterAmountSlider);

		outlineCheckBox = new asCheckBox("Cartoon Outline: ");
		outlineCheckBox->SetTooltip("Toggle the full screen cartoon outline effect.");
		outlineCheckBox->SetPos(XMFLOAT2(x, y += 35));
		outlineCheckBox->SetCheck(component->getOutlineEnabled());
		outlineCheckBox->OnClick([&](asEventArgs args) {
			component->setOutlineEnabled(args.bValue);
			});
		ppWindow->AddWidget(outlineCheckBox);

		outlineThresholdSlider = new asSlider(0, 1, 0.1f, 1000, "Threshold: ");
		outlineThresholdSlider->SetTooltip("Outline edge detection threshold. Increase if not enough otlines are detected, decrease if too many outlines are detected.");
		outlineThresholdSlider->SetSize(XMFLOAT2(100, 20));
		outlineThresholdSlider->SetPos(XMFLOAT2(x + 100, y));
		outlineThresholdSlider->SetValue(component->getOutlineThreshold());
		outlineThresholdSlider->OnSlide([&](asEventArgs args) {
			component->setOutlineThreshold(args.fValue);
			});
		ppWindow->AddWidget(outlineThresholdSlider);

		outlineThicknessSlider = new asSlider(0, 4, 1, 1000, "Thickness: ");
		outlineThicknessSlider->SetTooltip("Set outline thickness.");
		outlineThicknessSlider->SetSize(XMFLOAT2(100, 20));
		outlineThicknessSlider->SetPos(XMFLOAT2(x + 100, y += 35));
		outlineThicknessSlider->SetValue(component->getOutlineThickness());
		outlineThicknessSlider->OnSlide([&](asEventArgs args) {
			component->setOutlineThickness(args.fValue);
			});
		ppWindow->AddWidget(outlineThicknessSlider);

		chromaticaberrationCheckBox = new asCheckBox("Chromatic Aberration: ");
		chromaticaberrationCheckBox->SetTooltip("Toggle the full screen chromatic aberration effect. This simulates lens distortion at screen edges.");
		chromaticaberrationCheckBox->SetPos(XMFLOAT2(x, y += 35));
		chromaticaberrationCheckBox->SetCheck(component->getOutlineEnabled());
		chromaticaberrationCheckBox->OnClick([&](asEventArgs args) {
			component->setChromaticAberrationEnabled(args.bValue);
			});
		ppWindow->AddWidget(chromaticaberrationCheckBox);

		chromaticaberrationSlider = new asSlider(0, 4, 1.0f, 1000, "Amount: ");
		chromaticaberrationSlider->SetTooltip("The lens distortion amount.");
		chromaticaberrationSlider->SetSize(XMFLOAT2(100, 20));
		chromaticaberrationSlider->SetPos(XMFLOAT2(x + 100, y));
		chromaticaberrationSlider->SetValue(component->getChromaticAberrationAmount());
		chromaticaberrationSlider->OnSlide([&](asEventArgs args) {
			component->setChromaticAberrationAmount(args.fValue);
			});
		ppWindow->AddWidget(chromaticaberrationSlider);


		ppWindow->Translate(XMFLOAT3(screenW - 550, 50, 0));
		ppWindow->SetVisible(false);

	}


	PostprocessWindow::~PostprocessWindow()
	{
		ppWindow->RemoveWidgets(true);
		GUI->RemoveWidget(ppWindow);
		SAFE_DELETE(ppWindow);
	}
}

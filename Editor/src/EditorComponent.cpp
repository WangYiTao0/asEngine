#include "asEngine.h"
#include "EditorComponent.h"
#include "MaterialWindow.h"
#include "PostprocessWindow.h"
#include "WeatherWindow.h"
#include "ObjectWindow.h"
#include "MeshWindow.h"
#include "CameraWindow.h"
#include "RendererWindow.h"
#include "EnvProbeWindow.h"
#include "DecalWindow.h"
#include "LightWindow.h"
#include "AnimationWindow.h"
#include "EmitterWindow.h"
#include "HairParticleWindow.h"
#include "ForceFieldWindow.h"
#include "SoundWindow.h"

#include "ModelImporter.h"
#include "Translator.h"

#include <sstream>

using namespace std;
namespace as
{
	using namespace asGraphics;
	using namespace asRectPacker;
	using namespace asScene;
	using namespace asECS;


	void Editor::Initialize()
	{
		__super::Initialize();

		infoDisplay.active = true;
		infoDisplay.watermark = true;
		infoDisplay.fpsinfo = true;
		infoDisplay.resolution = true;

		asRenderer::GetDevice()->SetVSyncEnabled(true);
		asRenderer::SetOcclusionCullingEnabled(true);

		renderComponent = new EditorComponent;
		renderComponent->Initialize();
		loader = new EditorLoadingScreen;
		loader->Initialize();
		loader->Load();

		renderComponent->loader = loader;
		renderComponent->main = this;

		loader->addLoadingComponent(renderComponent, this, 0.2f);

		ActivatePath(loader, 0.2f);

	}

	void EditorLoadingScreen::Load()
	{
		font = asFont("Loading...", asFontParams((int)(asRenderer::GetDevice()->GetScreenWidth() * 0.5f), (int)(asRenderer::GetDevice()->GetScreenHeight() * 0.5f), 36,
			ASFALIGN_CENTER, ASFALIGN_CENTER));
		addFont(&font);

		sprite = asSprite("assets/images/as-logo.png");
		sprite.anim.opa = 1;
		sprite.anim.repeatable = true;
		sprite.params.pos = XMFLOAT3(asRenderer::GetDevice()->GetScreenWidth() * 0.5f, asRenderer::GetDevice()->GetScreenHeight() * 0.5f - font.textHeight(), 0);
		sprite.params.siz = XMFLOAT2(128, 128);
		sprite.params.pivot = XMFLOAT2(0.5f, 1.0f);
		sprite.params.quality = QUALITY_LINEAR;
		sprite.params.blendFlag = BLENDMODE_ALPHA;
		addSprite(&sprite);

		__super::Load();
	}
	void EditorLoadingScreen::Update(float dt)
	{
		font.params.posX = (int)(asRenderer::GetDevice()->GetScreenWidth() * 0.5f);
		font.params.posY = (int)(asRenderer::GetDevice()->GetScreenHeight() * 0.5f);
		sprite.params.pos = XMFLOAT3(asRenderer::GetDevice()->GetScreenWidth() * 0.5f, asRenderer::GetDevice()->GetScreenHeight() * 0.5f - font.textHeight(), 0);

		__super::Update(dt);
	}
	void EditorLoadingScreen::Unload()
	{

	}


	void EditorComponent::ChangeRenderPath(RENDERPATH path)
	{
		SAFE_DELETE(renderPath);

		switch (path)
		{
		case EditorComponent::RENDERPATH_FORWARD:
			renderPath = new RenderPath3D_Forward;
			break;
		case EditorComponent::RENDERPATH_DEFERRED:
			renderPath = new RenderPath3D_Deferred;
			break;
		case EditorComponent::RENDERPATH_TILEDFORWARD:
			renderPath = new RenderPath3D_TiledForward;
			break;
		case EditorComponent::RENDERPATH_TILEDDEFERRED:
			renderPath = new RenderPath3D_TiledDeferred;
			break;
		case EditorComponent::RENDERPATH_PATHTRACING:
			renderPath = new RenderPath3D_PathTracing;
			break;
		default:
			assert(0);
			break;
		}

		renderPath->setShadowsEnabled(true);
		renderPath->setReflectionsEnabled(true);
		renderPath->setSSAOEnabled(false);
		renderPath->setSSREnabled(false);
		renderPath->setMotionBlurEnabled(false);
		renderPath->setColorGradingEnabled(false);
		renderPath->setEyeAdaptionEnabled(false);
		renderPath->setFXAAEnabled(false);
		renderPath->setDepthOfFieldEnabled(false);
		renderPath->setLightShaftsEnabled(false);


		renderPath->Initialize();
		renderPath->Load();
		renderPath->Update(0);

		materialWnd.reset(new MaterialWindow(&GetGUI()));
		postprocessWnd.reset(new PostprocessWindow(&GetGUI(), renderPath));
		weatherWnd.reset(new WeatherWindow(&GetGUI()));
		objectWnd.reset(new ObjectWindow(this));
		meshWnd.reset(new MeshWindow(&GetGUI()));
		cameraWnd.reset(new CameraWindow(&GetGUI()));
		rendererWnd.reset(new RendererWindow(&GetGUI(), this, renderPath));
		envProbeWnd.reset(new EnvProbeWindow(&GetGUI()));
		soundWnd.reset(new SoundWindow(&GetGUI()));
		decalWnd.reset(new DecalWindow(&GetGUI()));
		lightWnd.reset(new LightWindow(&GetGUI()));
		animWnd.reset(new AnimationWindow(&GetGUI()));
		emitterWnd.reset(new EmitterWindow(&GetGUI()));
		hairWnd.reset(new HairParticleWindow(&GetGUI()));
		forceFieldWnd.reset(new ForceFieldWindow(&GetGUI()));

		ResizeBuffers();
	}

	void EditorComponent::ResizeBuffers()
	{
		__super::ResizeBuffers();

		GraphicsDevice* device = asRenderer::GetDevice();
		HRESULT hr;

		if (renderPath->GetDepthStencil() != nullptr)
		{
			TextureDesc desc;
			desc.Width = asRenderer::GetInternalResolution().x;
			desc.Height = asRenderer::GetInternalResolution().y;

			desc.Format = FORMAT_R8_UNORM;
			desc.BindFlags = BIND_RENDER_TARGET | BIND_SHADER_RESOURCE;
			if (renderPath->getMSAASampleCount() > 1)
			{
				desc.SampleCount = renderPath->getMSAASampleCount();
				hr = device->CreateTexture(&desc, nullptr, &rt_selectionOutline_MSAA);
				assert(SUCCEEDED(hr));
				desc.SampleCount = 1;
			}
			hr = device->CreateTexture(&desc, nullptr, &rt_selectionOutline[0]);
			assert(SUCCEEDED(hr));

			desc.Format = FORMAT_R8G8B8A8_UNORM;
			desc.BindFlags = BIND_RENDER_TARGET | BIND_SHADER_RESOURCE | BIND_UNORDERED_ACCESS;
			hr = device->CreateTexture(&desc, nullptr, &rt_selectionOutline[1]);
			assert(SUCCEEDED(hr));
		}

		{
			RenderPassDesc desc;
			desc.numAttachments = 2;
			desc.attachments[0] = { RenderPassAttachment::RENDERTARGET, RenderPassAttachment::LOADOP_CLEAR, &rt_selectionOutline[0], -1 };
			if (renderPath->getMSAASampleCount() > 1)
			{
				desc.attachments[0].texture = &rt_selectionOutline_MSAA;
			}
			desc.attachments[1] = { RenderPassAttachment::DEPTH_STENCIL, RenderPassAttachment::LOADOP_LOAD, renderPath->GetDepthStencil(), -1 };
			hr = device->CreateRenderPass(&desc, &renderpass_selectionOutline[0]);
			assert(SUCCEEDED(hr));

			desc.numAttachments = 1;
			desc.attachments[0] = { RenderPassAttachment::RENDERTARGET, RenderPassAttachment::LOADOP_CLEAR, &rt_selectionOutline[1], -1 };
			hr = device->CreateRenderPass(&desc, &renderpass_selectionOutline[1]);
			assert(SUCCEEDED(hr));
		}
	}
	void EditorComponent::Load()
	{
		__super::Load();

		asJobSystem::context ctx;
		asJobSystem::Execute(ctx, [this] { pointLightTex = asResourceManager::Load("assets/images/pointlight.dds"); });
		asJobSystem::Execute(ctx, [this] { spotLightTex = asResourceManager::Load("assets/images/spotlight.dds"); });
		asJobSystem::Execute(ctx, [this] { dirLightTex = asResourceManager::Load("assets/images/directional_light.dds"); });
		asJobSystem::Execute(ctx, [this] { areaLightTex = asResourceManager::Load("assets/images/arealight.dds"); });
		asJobSystem::Execute(ctx, [this] { decalTex = asResourceManager::Load("assets/images/decal.dds"); });
		asJobSystem::Execute(ctx, [this] { forceFieldTex = asResourceManager::Load("assets/images/forcefield.dds"); });
		asJobSystem::Execute(ctx, [this] { emitterTex = asResourceManager::Load("assets/images/emitter.dds"); });
		asJobSystem::Execute(ctx, [this] { hairTex = asResourceManager::Load("assets/images/hair.dds"); });
		asJobSystem::Execute(ctx, [this] { cameraTex = asResourceManager::Load("assets/images/camera.dds"); });
		asJobSystem::Execute(ctx, [this] { armatureTex = asResourceManager::Load("assets/images/armature.dds"); });
		asJobSystem::Execute(ctx, [this] { soundTex = asResourceManager::Load("assets/images/sound.dds"); });
		// wait for ctx is at the end of this function!

		translator.enabled = false;

		float screenW = (float)asRenderer::GetDevice()->GetScreenWidth();
		float screenH = (float)asRenderer::GetDevice()->GetScreenHeight();

		XMFLOAT2 option_size = XMFLOAT2(100, 28);
		float step = (option_size.y + 5) * -1, x = screenW - option_size.x, y = screenH - option_size.y;


		asButton* rendererWnd_Toggle = new asButton("Renderer");
		rendererWnd_Toggle->SetTooltip("Renderer settings window");
		rendererWnd_Toggle->SetPos(XMFLOAT2(x, y));
		rendererWnd_Toggle->SetSize(option_size);
		rendererWnd_Toggle->OnClick([=](asEventArgs args) {
			rendererWnd->rendererWindow->SetVisible(!rendererWnd->rendererWindow->IsVisible());
			});
		GetGUI().AddWidget(rendererWnd_Toggle);

		asButton* weatherWnd_Toggle = new asButton("Weather");
		weatherWnd_Toggle->SetTooltip("World settings window");
		weatherWnd_Toggle->SetPos(XMFLOAT2(x, y += step));
		weatherWnd_Toggle->SetSize(option_size);
		weatherWnd_Toggle->OnClick([=](asEventArgs args) {
			weatherWnd->weatherWindow->SetVisible(!weatherWnd->weatherWindow->IsVisible());
			});
		GetGUI().AddWidget(weatherWnd_Toggle);

		asButton* objectWnd_Toggle = new asButton("Object");
		objectWnd_Toggle->SetTooltip("Object settings window");
		objectWnd_Toggle->SetPos(XMFLOAT2(x, y += step));
		objectWnd_Toggle->SetSize(option_size);
		objectWnd_Toggle->OnClick([=](asEventArgs args) {
			objectWnd->objectWindow->SetVisible(!objectWnd->objectWindow->IsVisible());
			});
		GetGUI().AddWidget(objectWnd_Toggle);

		asButton* meshWnd_Toggle = new asButton("Mesh");
		meshWnd_Toggle->SetTooltip("Mesh settings window");
		meshWnd_Toggle->SetPos(XMFLOAT2(x, y += step));
		meshWnd_Toggle->SetSize(option_size);
		meshWnd_Toggle->OnClick([=](asEventArgs args) {
			meshWnd->meshWindow->SetVisible(!meshWnd->meshWindow->IsVisible());
			});
		GetGUI().AddWidget(meshWnd_Toggle);

		asButton* materialWnd_Toggle = new asButton("Material");
		materialWnd_Toggle->SetTooltip("Material settings window");
		materialWnd_Toggle->SetPos(XMFLOAT2(x, y += step));
		materialWnd_Toggle->SetSize(option_size);
		materialWnd_Toggle->OnClick([=](asEventArgs args) {
			materialWnd->materialWindow->SetVisible(!materialWnd->materialWindow->IsVisible());
			});
		GetGUI().AddWidget(materialWnd_Toggle);

		asButton* postprocessWnd_Toggle = new asButton("PostProcess");
		postprocessWnd_Toggle->SetTooltip("Postprocess settings window");
		postprocessWnd_Toggle->SetPos(XMFLOAT2(x, y += step));
		postprocessWnd_Toggle->SetSize(option_size);
		postprocessWnd_Toggle->OnClick([=](asEventArgs args) {
			postprocessWnd->ppWindow->SetVisible(!postprocessWnd->ppWindow->IsVisible());
			});
		GetGUI().AddWidget(postprocessWnd_Toggle);

		asButton* cameraWnd_Toggle = new asButton("Camera");
		cameraWnd_Toggle->SetTooltip("Camera settings window");
		cameraWnd_Toggle->SetPos(XMFLOAT2(x, y += step));
		cameraWnd_Toggle->SetSize(option_size);
		cameraWnd_Toggle->OnClick([=](asEventArgs args) {
			cameraWnd->cameraWindow->SetVisible(!cameraWnd->cameraWindow->IsVisible());
			});
		GetGUI().AddWidget(cameraWnd_Toggle);

		asButton* envProbeWnd_Toggle = new asButton("EnvProbe");
		envProbeWnd_Toggle->SetTooltip("Environment probe settings window");
		envProbeWnd_Toggle->SetPos(XMFLOAT2(x, y += step));
		envProbeWnd_Toggle->SetSize(option_size);
		envProbeWnd_Toggle->OnClick([=](asEventArgs args) {
			envProbeWnd->envProbeWindow->SetVisible(!envProbeWnd->envProbeWindow->IsVisible());
			});
		GetGUI().AddWidget(envProbeWnd_Toggle);

		asButton* decalWnd_Toggle = new asButton("Decal");
		decalWnd_Toggle->SetTooltip("Decal settings window");
		decalWnd_Toggle->SetPos(XMFLOAT2(x, y += step));
		decalWnd_Toggle->SetSize(option_size);
		decalWnd_Toggle->OnClick([=](asEventArgs args) {
			decalWnd->decalWindow->SetVisible(!decalWnd->decalWindow->IsVisible());
			});
		GetGUI().AddWidget(decalWnd_Toggle);

		asButton* soundWnd_Toggle = new asButton("Sound");
		soundWnd_Toggle->SetTooltip("Sound settings window");
		soundWnd_Toggle->SetPos(XMFLOAT2(x, y += step));
		soundWnd_Toggle->SetSize(option_size);
		soundWnd_Toggle->OnClick([=](asEventArgs args) {
			soundWnd->soundWindow->SetVisible(!soundWnd->soundWindow->IsVisible());
			});
		GetGUI().AddWidget(soundWnd_Toggle);

		asButton* lightWnd_Toggle = new asButton("Light");
		lightWnd_Toggle->SetTooltip("Light settings window");
		lightWnd_Toggle->SetPos(XMFLOAT2(x, y += step));
		lightWnd_Toggle->SetSize(option_size);
		lightWnd_Toggle->OnClick([=](asEventArgs args) {
			lightWnd->lightWindow->SetVisible(!lightWnd->lightWindow->IsVisible());
			});
		GetGUI().AddWidget(lightWnd_Toggle);

		asButton* animWnd_Toggle = new asButton("Animation");
		animWnd_Toggle->SetTooltip("Animation inspector window");
		animWnd_Toggle->SetPos(XMFLOAT2(x, y += step));
		animWnd_Toggle->SetSize(option_size);
		animWnd_Toggle->OnClick([=](asEventArgs args) {
			animWnd->animWindow->SetVisible(!animWnd->animWindow->IsVisible());
			});
		GetGUI().AddWidget(animWnd_Toggle);

		asButton* emitterWnd_Toggle = new asButton("Emitter");
		emitterWnd_Toggle->SetTooltip("Emitter Particle System properties");
		emitterWnd_Toggle->SetPos(XMFLOAT2(x, y += step));
		emitterWnd_Toggle->SetSize(option_size);
		emitterWnd_Toggle->OnClick([=](asEventArgs args) {
			emitterWnd->emitterWindow->SetVisible(!emitterWnd->emitterWindow->IsVisible());
			});
		GetGUI().AddWidget(emitterWnd_Toggle);

		asButton* hairWnd_Toggle = new asButton("HairParticle");
		hairWnd_Toggle->SetTooltip("Hair Particle System properties");
		hairWnd_Toggle->SetPos(XMFLOAT2(x, y += step));
		hairWnd_Toggle->SetSize(option_size);
		hairWnd_Toggle->OnClick([=](asEventArgs args) {
			hairWnd->hairWindow->SetVisible(!hairWnd->hairWindow->IsVisible());
			});
		GetGUI().AddWidget(hairWnd_Toggle);

		asButton* forceFieldWnd_Toggle = new asButton("ForceField");
		forceFieldWnd_Toggle->SetTooltip("Force Field properties");
		forceFieldWnd_Toggle->SetPos(XMFLOAT2(x, y += step));
		forceFieldWnd_Toggle->SetSize(option_size);
		forceFieldWnd_Toggle->OnClick([=](asEventArgs args) {
			forceFieldWnd->forceFieldWindow->SetVisible(!forceFieldWnd->forceFieldWindow->IsVisible());
			});
		GetGUI().AddWidget(forceFieldWnd_Toggle);

		////////////////////////////////////////////////////////////////////////////////////

		asCheckBox* translatorCheckBox = new asCheckBox("Translator: ");
		translatorCheckBox->SetTooltip("Enable the translator tool");
		translatorCheckBox->SetPos(XMFLOAT2(screenW - 50 - 55 - 105 * 5 - 25, 0));
		translatorCheckBox->SetSize(XMFLOAT2(18, 18));
		translatorCheckBox->OnClick([&](asEventArgs args) {
			EndTranslate();
			translator.enabled = args.bValue;
			BeginTranslate();
			});
		GetGUI().AddWidget(translatorCheckBox);

		asCheckBox* isScalatorCheckBox = new asCheckBox("S:");
		asCheckBox* isRotatorCheckBox = new asCheckBox("R:");
		asCheckBox* isTranslatorCheckBox = new asCheckBox("T:");
		{
			isScalatorCheckBox->SetTooltip("Scale");
			isScalatorCheckBox->SetPos(XMFLOAT2(screenW - 50 - 55 - 105 * 5 - 25 - 40 * 2, 22));
			isScalatorCheckBox->SetSize(XMFLOAT2(18, 18));
			isScalatorCheckBox->OnClick([&, isTranslatorCheckBox, isRotatorCheckBox](asEventArgs args) {
				translator.isScalator = args.bValue;
				translator.isTranslator = false;
				translator.isRotator = false;
				isTranslatorCheckBox->SetCheck(false);
				isRotatorCheckBox->SetCheck(false);
				});
			isScalatorCheckBox->SetCheck(translator.isScalator);
			GetGUI().AddWidget(isScalatorCheckBox);

			isRotatorCheckBox->SetTooltip("Rotate");
			isRotatorCheckBox->SetPos(XMFLOAT2(screenW - 50 - 55 - 105 * 5 - 25 - 40 * 1, 22));
			isRotatorCheckBox->SetSize(XMFLOAT2(18, 18));
			isRotatorCheckBox->OnClick([&, isTranslatorCheckBox, isScalatorCheckBox](asEventArgs args) {
				translator.isRotator = args.bValue;
				translator.isScalator = false;
				translator.isTranslator = false;
				isScalatorCheckBox->SetCheck(false);
				isTranslatorCheckBox->SetCheck(false);
				});
			isRotatorCheckBox->SetCheck(translator.isRotator);
			GetGUI().AddWidget(isRotatorCheckBox);

			isTranslatorCheckBox->SetTooltip("Translate");
			isTranslatorCheckBox->SetPos(XMFLOAT2(screenW - 50 - 55 - 105 * 5 - 25, 22));
			isTranslatorCheckBox->SetSize(XMFLOAT2(18, 18));
			isTranslatorCheckBox->OnClick([&, isScalatorCheckBox, isRotatorCheckBox](asEventArgs args) {
				translator.isTranslator = args.bValue;
				translator.isScalator = false;
				translator.isRotator = false;
				isScalatorCheckBox->SetCheck(false);
				isRotatorCheckBox->SetCheck(false);
				});
			isTranslatorCheckBox->SetCheck(translator.isTranslator);
			GetGUI().AddWidget(isTranslatorCheckBox);
		}


		asButton* saveButton = new asButton("Save");
		saveButton->SetTooltip("Save the current scene");
		saveButton->SetPos(XMFLOAT2(screenW - 50 - 55 - 105 * 5, 0));
		saveButton->SetSize(XMFLOAT2(100, 40));
		saveButton->SetColor(asColor(0, 198, 101, 200), asWidget::ASWIDGETSTATE::IDLE);
		saveButton->SetColor(asColor(0, 255, 140, 255), asWidget::ASWIDGETSTATE::FOCUS);
		saveButton->OnClick([=](asEventArgs args) {
			EndTranslate();

			asHelper::FileDialogParams params;
			asHelper::FileDialogResult result;
			params.type = asHelper::FileDialogParams::SAVE;
			params.description = "Wicked Scene";
			params.extensions.push_back("asScene");
			asHelper::FileDialog(params, result);

			if (result.ok) {
				string fileName = result.filenames.front();
				if (fileName.substr(fileName.length() - 8).compare(".asScene") != 0)
				{
					fileName += ".asScene";
				}
				asArchive archive(fileName, false);
				if (archive.IsOpen())
				{
					Scene& scene = asScene::GetScene();

					scene.Serialize(archive);

					ResetHistory();
				}
				else
				{
					asHelper::messageBox("Could not create " + fileName + "!");
				}
			}
			});
		GetGUI().AddWidget(saveButton);


		asButton* modelButton = new asButton("Load Model");
		modelButton->SetTooltip("Load a scene / import model into the editor...");
		modelButton->SetPos(XMFLOAT2(screenW - 50 - 55 - 105 * 4, 0));
		modelButton->SetSize(XMFLOAT2(100, 40));
		modelButton->SetColor(asColor(0, 89, 255, 200), asWidget::ASWIDGETSTATE::IDLE);
		modelButton->SetColor(asColor(112, 155, 255, 255), asWidget::ASWIDGETSTATE::FOCUS);
		modelButton->OnClick([=](asEventArgs args) {
			thread([&] {

				asHelper::FileDialogParams params;
				asHelper::FileDialogResult result;
				params.type = asHelper::FileDialogParams::OPEN;
				params.description = "Model formats (.asScene, .obj, .gltf, .glb)";
				params.extensions.push_back("asScene");
				params.extensions.push_back("obj");
				params.extensions.push_back("gltf");
				params.extensions.push_back("glb");
				asHelper::FileDialog(params, result);

				if (result.ok)
				{
					string fileName = result.filenames.front();

					loader->addLoadingFunction([=] {
						string extension = asHelper::toUpper(asHelper::GetExtensionFromFileName(fileName));

						if (!extension.compare("asScene")) // engine-serialized
						{
							asScene::LoadModel(fileName);
						}
						else if (!extension.compare("OBJ")) // wavefront-obj
						{
							Scene scene;
							ImportModel_OBJ(fileName, scene);
							asScene::GetScene().Merge(scene);
						}
						else if (!extension.compare("GLTF")) // text-based gltf
						{
							Scene scene;
							ImportModel_GLTF(fileName, scene);
							asScene::GetScene().Merge(scene);
						}
						else if (!extension.compare("GLB")) // binary gltf
						{
							Scene scene;
							ImportModel_GLTF(fileName, scene);
							asScene::GetScene().Merge(scene);
						}
						});
					loader->onFinished([=] {
						main->ActivatePath(this, 0.2f, asColor::Black());
						weatherWnd->Update();
						});
					main->ActivatePath(loader, 0.2f, asColor::Black());
					ResetHistory();
				}
				}).detach();
			});
		GetGUI().AddWidget(modelButton);


		asButton* scriptButton = new asButton("Load Script");
		scriptButton->SetTooltip("Load a Lua script...");
		scriptButton->SetPos(XMFLOAT2(screenW - 50 - 55 - 105 * 3, 0));
		scriptButton->SetSize(XMFLOAT2(100, 40));
		scriptButton->SetColor(asColor(255, 33, 140, 200), asWidget::ASWIDGETSTATE::IDLE);
		scriptButton->SetColor(asColor(255, 100, 140, 255), asWidget::ASWIDGETSTATE::FOCUS);
		scriptButton->OnClick([=](asEventArgs args) {
			thread([&] {

				asHelper::FileDialogParams params;
				asHelper::FileDialogResult result;
				params.type = asHelper::FileDialogParams::OPEN;
				params.description = "Lua script";
				params.extensions.push_back("lua");
				asHelper::FileDialog(params, result);

				if (result.ok) {
					string fileName = result.filenames.front();
					//wiLua::GetGlobal()->RunFile(fileName);
				}
				}).detach();

			});
		GetGUI().AddWidget(scriptButton);


		asButton* shaderButton = new asButton("Reload Shaders");
		shaderButton->SetTooltip("Reload shaders from the default directory...");
		shaderButton->SetPos(XMFLOAT2(screenW - 50 - 55 - 105 * 2, 0));
		shaderButton->SetSize(XMFLOAT2(100, 40));
		shaderButton->SetColor(asColor(255, 33, 140, 200), asWidget::ASWIDGETSTATE::IDLE);
		shaderButton->SetColor(asColor(255, 100, 140, 255), asWidget::ASWIDGETSTATE::FOCUS);
		shaderButton->OnClick([=](asEventArgs args) {

			asRenderer::ReloadShaders();

			Translator::LoadShaders();

			});
		GetGUI().AddWidget(shaderButton);


		asButton* clearButton = new asButton("Clear World");
		clearButton->SetTooltip("Delete every model from the scene");
		clearButton->SetPos(XMFLOAT2(screenW - 50 - 55 - 105 * 1, 0));
		clearButton->SetSize(XMFLOAT2(100, 40));
		clearButton->SetColor(asColor(255, 205, 43, 200), asWidget::ASWIDGETSTATE::IDLE);
		clearButton->SetColor(asColor(255, 235, 173, 255), asWidget::ASWIDGETSTATE::FOCUS);
		clearButton->OnClick([&](asEventArgs args) {
			selected.clear();
			EndTranslate();
			asRenderer::ClearWorld();
			objectWnd->SetEntity(INVALID_ENTITY);
			meshWnd->SetEntity(INVALID_ENTITY);
			lightWnd->SetEntity(INVALID_ENTITY);
			soundWnd->SetEntity(INVALID_ENTITY);
			decalWnd->SetEntity(INVALID_ENTITY);
			envProbeWnd->SetEntity(INVALID_ENTITY);
			materialWnd->SetEntity(INVALID_ENTITY);
			emitterWnd->SetEntity(INVALID_ENTITY);
			hairWnd->SetEntity(INVALID_ENTITY);
			forceFieldWnd->SetEntity(INVALID_ENTITY);
			cameraWnd->SetEntity(INVALID_ENTITY);
			});
		GetGUI().AddWidget(clearButton);


		asButton* helpButton = new asButton("?");
		helpButton->SetTooltip("Help");
		helpButton->SetPos(XMFLOAT2(screenW - 50 - 55, 0));
		helpButton->SetSize(XMFLOAT2(50, 40));
		helpButton->SetColor(asColor(34, 158, 214, 200), asWidget::ASWIDGETSTATE::IDLE);
		helpButton->SetColor(asColor(113, 183, 214, 255), asWidget::ASWIDGETSTATE::FOCUS);
		helpButton->OnClick([=](asEventArgs args) {
			static asLabel* helpLabel = nullptr;
			if (helpLabel == nullptr)
			{
				stringstream ss("");
				ss << "Help:   " << endl << "############" << endl << endl;
				ss << "Move camera: WASD" << endl;
				ss << "Look: Middle mouse button / arrow keys" << endl;
				ss << "Select: Right mouse button" << endl;
				ss << "Place decal, interact with water: Left mouse button when nothing is selected" << endl;
				ss << "Camera speed: SHIFT button" << endl;
				ss << "Camera up: E, down: Q" << endl;
				ss << "Duplicate entity: Ctrl + D" << endl;
				ss << "Select All: Ctrl + A" << endl;
				ss << "Undo: Ctrl + Z" << endl;
				ss << "Redo: Ctrl + Y" << endl;
				ss << "Copy: Ctrl + C" << endl;
				ss << "Paste: Ctrl + V" << endl;
				ss << "Delete: DELETE button" << endl;
				ss << "Place Instances: Ctrl + Shift + Left mouse click (place clipboard onto clicked surface)" << endl;
				ss << "Pin soft body triangle: Hold P while nothing is selected and click on soft body with Left mouse button" << endl;
				ss << "Script Console / backlog: HOME button" << endl;
				ss << endl;
				ss << "You can find sample scenes in the models directory. Try to load one." << endl;
				ss << "You can also import models from .OBJ, .GLTF, .GLB files." << endl;
				ss << "You can find a program configuration file at Editor/config.ini" << endl;
				ss << "You can find sample LUA scripts in the scripts directory. Try to load one." << endl;
				ss << "You can find a startup script at Editor/startup.lua (this will be executed on program start)" << endl;
				ss << endl << endl << "For questions, bug reports, feedback, requests, please open an issue at:" << endl;
				ss << "https://github.com/turanszkij/WickedEngine" << endl;

				helpLabel = new asLabel("HelpLabel");
				helpLabel->SetText(ss.str());
				helpLabel->SetSize(XMFLOAT2(screenW / 3.0f, screenH / 2.2f));
				helpLabel->SetPos(XMFLOAT2(screenW / 2.0f - helpLabel->scale.x / 2.0f, screenH / 2.0f - helpLabel->scale.y / 2.0f));
				helpLabel->SetVisible(false);
				GetGUI().AddWidget(helpLabel);
			}

			helpLabel->SetVisible(!helpLabel->IsVisible());
			});
		GetGUI().AddWidget(helpButton);


		asButton* exitButton = new asButton("X");
		exitButton->SetTooltip("Exit");
		exitButton->SetPos(XMFLOAT2(screenW - 50, 0));
		exitButton->SetSize(XMFLOAT2(50, 40));
		exitButton->SetColor(asColor(190, 0, 0, 200), asWidget::ASWIDGETSTATE::IDLE);
		exitButton->SetColor(asColor(255, 0, 0, 255), asWidget::ASWIDGETSTATE::FOCUS);
		exitButton->OnClick([](asEventArgs args) {
			exit(0);
			});
		GetGUI().AddWidget(exitButton);



		//asCheckBox* physicsEnabledCheckBox = new asCheckBox("Physics Enabled: ");
		//physicsEnabledCheckBox->SetSize(XMFLOAT2(18, 18));
		//physicsEnabledCheckBox->SetPos(XMFLOAT2(screenW - 25, 50));
		//physicsEnabledCheckBox->SetTooltip("Toggle Physics Engine On/Off");
		//physicsEnabledCheckBox->OnClick([&](asEventArgs args) {
		//	asPhysicsEngine::SetEnabled(args.bValue);
		//	});
		//physicsEnabledCheckBox->SetCheck(asPhysicsEngine::IsEnabled());
		//GetGUI().AddWidget(physicsEnabledCheckBox);

		cinemaModeCheckBox = new asCheckBox("Cinema Mode: ");
		cinemaModeCheckBox->SetSize(XMFLOAT2(18, 18));
		cinemaModeCheckBox->SetPos(XMFLOAT2(screenW - 25, 72));
		cinemaModeCheckBox->SetTooltip("Toggle Cinema Mode (All HUD disabled). Press ESC to exit.");
		cinemaModeCheckBox->OnClick([&](asEventArgs args) {
			if (renderPath != nullptr)
			{
				renderPath->GetGUI().SetVisible(false);
			}
			GetGUI().SetVisible(false);
			asProfiler::SetEnabled(false);
			main->infoDisplay.active = false;
			});
		GetGUI().AddWidget(cinemaModeCheckBox);


		asComboBox* renderPathComboBox = new asComboBox("Render Path: ");
		renderPathComboBox->SetSize(XMFLOAT2(100, 20));
		renderPathComboBox->SetPos(XMFLOAT2(screenW - 128, 94));
		renderPathComboBox->AddItem("Forward");
		renderPathComboBox->AddItem("Deferred");
		renderPathComboBox->AddItem("Tiled Forward");
		renderPathComboBox->AddItem("Tiled Deferred");
		renderPathComboBox->AddItem("Path Tracing");
		renderPathComboBox->OnSelect([&](asEventArgs args) {
			switch (args.iValue)
			{
			case 0:
				ChangeRenderPath(RENDERPATH_FORWARD);
				break;
			case 1:
				ChangeRenderPath(RENDERPATH_DEFERRED);
				break;
			case 2:
				ChangeRenderPath(RENDERPATH_TILEDFORWARD);
				break;
			case 3:
				ChangeRenderPath(RENDERPATH_TILEDDEFERRED);
				break;
			case 4:
				ChangeRenderPath(RENDERPATH_PATHTRACING);
				break;
			default:
				break;
			}
			});
		renderPathComboBox->SetSelected(2);
		renderPathComboBox->SetEnabled(true);
		renderPathComboBox->SetTooltip("Choose a render path...");
		GetGUI().AddWidget(renderPathComboBox);


		cameraWnd->ResetCam();

		asJobSystem::Wait(ctx);
	}
	void EditorComponent::Start()
	{
		__super::Start();
	}
	void EditorComponent::FixedUpdate()
	{
		__super::FixedUpdate();

		renderPath->FixedUpdate();
	}
	void EditorComponent::Update(float dt)
	{
		Scene& scene = asScene::GetScene();
		CameraComponent& camera = asRenderer::GetCamera();

		animWnd->Update();
		weatherWnd->Update();

		selectionOutlineTimer += dt;

		// Exit cinema mode:
		if (asInput::Down(asInput::KEYBOARD_BUTTON_ESCAPE))
		{
			if (renderPath != nullptr)
			{
				renderPath->GetGUI().SetVisible(true);
			}
			GetGUI().SetVisible(true);
			asProfiler::SetEnabled(true);
			main->infoDisplay.active = true;

			cinemaModeCheckBox->SetCheck(false);
		}

		if (!asBackLog::isActive() && !GetGUI().HasFocus())
		{

			// Camera control:
			static XMFLOAT4 originalMouse = XMFLOAT4(0, 0, 0, 0);
			static bool camControlStart = true;
			if (camControlStart)
			{
				originalMouse = asInput::GetPointer();
			}

			XMFLOAT4 currentMouse = asInput::GetPointer();
			float xDif = 0, yDif = 0;

			if (asInput::Down(asInput::MOUSE_BUTTON_MIDDLE))
			{
				camControlStart = false;
				xDif = currentMouse.x - originalMouse.x;
				yDif = currentMouse.y - originalMouse.y;
				xDif = 0.1f * xDif * (1.0f / 60.0f);
				yDif = 0.1f * yDif * (1.0f / 60.0f);
				asInput::SetPointer(originalMouse);
				asInput::HidePointer(true);
			}
			else
			{
				camControlStart = true;
				asInput::HidePointer(false);
			}

			const float buttonrotSpeed = 2.0f / 60.0f;
			if (asInput::Down(asInput::KEYBOARD_BUTTON_LEFT))
			{
				xDif -= buttonrotSpeed;
			}
			if (asInput::Down(asInput::KEYBOARD_BUTTON_RIGHT))
			{
				xDif += buttonrotSpeed;
			}
			if (asInput::Down(asInput::KEYBOARD_BUTTON_UP))
			{
				yDif -= buttonrotSpeed;
			}
			if (asInput::Down(asInput::KEYBOARD_BUTTON_DOWN))
			{
				yDif += buttonrotSpeed;
			}

			const XMFLOAT4 leftStick = asInput::GetAnalog(asInput::GAMEPAD_ANALOG_THUMBSTICK_L, 0);
			const XMFLOAT4 rightStick = asInput::GetAnalog(asInput::GAMEPAD_ANALOG_THUMBSTICK_R, 0);
			const XMFLOAT4 rightTrigger = asInput::GetAnalog(asInput::GAMEPAD_ANALOG_TRIGGER_R, 0);

			const float jostickrotspeed = 0.05f;
			xDif += rightStick.x * jostickrotspeed;
			yDif += rightStick.y * jostickrotspeed;

			xDif *= cameraWnd->rotationspeedSlider->GetValue();
			yDif *= cameraWnd->rotationspeedSlider->GetValue();


			if (cameraWnd->fpsCheckBox->GetCheck())
			{
				// FPS Camera
				const float clampedDT = min(dt, 0.1f); // if dt > 100 millisec, don't allow the camera to jump too far...

				const float speed = ((asInput::Down(asInput::KEYBOARD_BUTTON_LSHIFT) ? 10.0f : 1.0f) + rightTrigger.x * 10.0f) * cameraWnd->movespeedSlider->GetValue() * clampedDT;
				static XMVECTOR move = XMVectorSet(0, 0, 0, 0);
				XMVECTOR moveNew = XMVectorSet(leftStick.x, 0, leftStick.y, 0);

				if (!asInput::Down(asInput::KEYBOARD_BUTTON_LCONTROL))
				{
					// Only move camera if control not pressed
					if (asInput::Down((asInput::BUTTON)'A') || asInput::Down(asInput::GAMEPAD_BUTTON_LEFT)) { moveNew += XMVectorSet(-1, 0, 0, 0); }
					if (asInput::Down((asInput::BUTTON)'D') || asInput::Down(asInput::GAMEPAD_BUTTON_RIGHT)) { moveNew += XMVectorSet(1, 0, 0, 0); }
					if (asInput::Down((asInput::BUTTON)'W') || asInput::Down(asInput::GAMEPAD_BUTTON_UP)) { moveNew += XMVectorSet(0, 0, 1, 0); }
					if (asInput::Down((asInput::BUTTON)'S') || asInput::Down(asInput::GAMEPAD_BUTTON_DOWN)) { moveNew += XMVectorSet(0, 0, -1, 0); }
					if (asInput::Down((asInput::BUTTON)'E') || asInput::Down(asInput::GAMEPAD_BUTTON_2)) { moveNew += XMVectorSet(0, 1, 0, 0); }
					if (asInput::Down((asInput::BUTTON)'Q') || asInput::Down(asInput::GAMEPAD_BUTTON_1)) { moveNew += XMVectorSet(0, -1, 0, 0); }
					moveNew += XMVector3Normalize(moveNew);
				}
				moveNew *= speed;

				move = XMVectorLerp(move, moveNew, 0.18f * clampedDT / 0.0166f); // smooth the movement a bit
				float moveLength = XMVectorGetX(XMVector3Length(move));

				if (moveLength < 0.0001f)
				{
					move = XMVectorSet(0, 0, 0, 0);
				}

				if (abs(xDif) + abs(yDif) > 0 || moveLength > 0.0001f)
				{
					XMMATRIX camRot = XMMatrixRotationQuaternion(XMLoadFloat4(&cameraWnd->camera_transform.rotation_local));
					XMVECTOR move_rot = XMVector3TransformNormal(move, camRot);
					XMFLOAT3 _move;
					XMStoreFloat3(&_move, move_rot);
					cameraWnd->camera_transform.Translate(_move);
					cameraWnd->camera_transform.RotateRollPitchYaw(XMFLOAT3(yDif, xDif, 0));
					camera.SetDirty();
				}

				cameraWnd->camera_transform.UpdateTransform();
			}
			else
			{
				// Orbital Camera

				if (asInput::Down(asInput::KEYBOARD_BUTTON_LSHIFT))
				{
					XMVECTOR V = XMVectorAdd(camera.GetRight() * xDif, camera.GetUp() * yDif) * 10;
					XMFLOAT3 vec;
					XMStoreFloat3(&vec, V);
					cameraWnd->camera_target.Translate(vec);
				}
				else if (asInput::Down(asInput::KEYBOARD_BUTTON_LCONTROL) || currentMouse.z != 0.0f)
				{
					cameraWnd->camera_transform.Translate(XMFLOAT3(0, 0, yDif * 4 + currentMouse.z));
					cameraWnd->camera_transform.translation_local.z = std::min(0.0f, cameraWnd->camera_transform.translation_local.z);
					camera.SetDirty();
				}
				else if (abs(xDif) + abs(yDif) > 0)
				{
					cameraWnd->camera_target.RotateRollPitchYaw(XMFLOAT3(yDif * 2, xDif * 2, 0));
					camera.SetDirty();
				}

				cameraWnd->camera_target.UpdateTransform();
				cameraWnd->camera_transform.UpdateTransform_Parented(cameraWnd->camera_target);
			}

			// Begin picking:
			UINT pickMask = rendererWnd->GetPickType();
			RAY pickRay = asRenderer::GetPickRay((long)currentMouse.x, (long)currentMouse.y);
			{
				hovered = asScene::PickResult();

				// Try to pick objects-meshes:
				if (pickMask & PICK_OBJECT)
				{
					hovered = asScene::Pick(pickRay, pickMask);
				}

				if (pickMask & PICK_LIGHT)
				{
					for (size_t i = 0; i < scene.lights.GetCount(); ++i)
					{
						Entity entity = scene.lights.GetEntity(i);
						const TransformComponent& transform = *scene.transforms.GetComponent(entity);

						XMVECTOR disV = XMVector3LinePointDistance(XMLoadFloat3(&pickRay.origin), XMLoadFloat3(&pickRay.origin) + XMLoadFloat3(&pickRay.direction), transform.GetPositionV());
						float dis = XMVectorGetX(disV);
						if (dis < asMath::Distance(transform.GetPosition(), pickRay.origin) * 0.05f && dis < hovered.distance)
						{
							hovered = asScene::PickResult();
							hovered.entity = entity;
							hovered.distance = dis;
						}
					}
				}
				if (pickMask & PICK_DECAL)
				{
					for (size_t i = 0; i < scene.decals.GetCount(); ++i)
					{
						Entity entity = scene.decals.GetEntity(i);
						const TransformComponent& transform = *scene.transforms.GetComponent(entity);

						XMVECTOR disV = XMVector3LinePointDistance(XMLoadFloat3(&pickRay.origin), XMLoadFloat3(&pickRay.origin) + XMLoadFloat3(&pickRay.direction), transform.GetPositionV());
						float dis = XMVectorGetX(disV);
						if (dis < asMath::Distance(transform.GetPosition(), pickRay.origin) * 0.05f && dis < hovered.distance)
						{
							hovered = asScene::PickResult();
							hovered.entity = entity;
							hovered.distance = dis;
						}
					}
				}
				if (pickMask & PICK_FORCEFIELD)
				{
					for (size_t i = 0; i < scene.forces.GetCount(); ++i)
					{
						Entity entity = scene.forces.GetEntity(i);
						const TransformComponent& transform = *scene.transforms.GetComponent(entity);

						XMVECTOR disV = XMVector3LinePointDistance(XMLoadFloat3(&pickRay.origin), XMLoadFloat3(&pickRay.origin) + XMLoadFloat3(&pickRay.direction), transform.GetPositionV());
						float dis = XMVectorGetX(disV);
						if (dis < asMath::Distance(transform.GetPosition(), pickRay.origin) * 0.05f && dis < hovered.distance)
						{
							hovered = asScene::PickResult();
							hovered.entity = entity;
							hovered.distance = dis;
						}
					}
				}
				if (pickMask & PICK_EMITTER)
				{
					for (size_t i = 0; i < scene.emitters.GetCount(); ++i)
					{
						Entity entity = scene.emitters.GetEntity(i);
						const TransformComponent& transform = *scene.transforms.GetComponent(entity);

						XMVECTOR disV = XMVector3LinePointDistance(XMLoadFloat3(&pickRay.origin), XMLoadFloat3(&pickRay.origin) + XMLoadFloat3(&pickRay.direction), transform.GetPositionV());
						float dis = XMVectorGetX(disV);
						if (dis < asMath::Distance(transform.GetPosition(), pickRay.origin) * 0.05f && dis < hovered.distance)
						{
							hovered = asScene::PickResult();
							hovered.entity = entity;
							hovered.distance = dis;
						}
					}
				}
				if (pickMask & PICK_HAIR)
				{
					for (size_t i = 0; i < scene.hairs.GetCount(); ++i)
					{
						Entity entity = scene.hairs.GetEntity(i);
						const TransformComponent& transform = *scene.transforms.GetComponent(entity);

						XMVECTOR disV = XMVector3LinePointDistance(XMLoadFloat3(&pickRay.origin), XMLoadFloat3(&pickRay.origin) + XMLoadFloat3(&pickRay.direction), transform.GetPositionV());
						float dis = XMVectorGetX(disV);
						if (dis < asMath::Distance(transform.GetPosition(), pickRay.origin) * 0.05f && dis < hovered.distance)
						{
							hovered = asScene::PickResult();
							hovered.entity = entity;
							hovered.distance = dis;
						}
					}
				}
				if (pickMask & PICK_ENVPROBE)
				{
					for (size_t i = 0; i < scene.probes.GetCount(); ++i)
					{
						Entity entity = scene.probes.GetEntity(i);
						const TransformComponent& transform = *scene.transforms.GetComponent(entity);

						if (SPHERE(transform.GetPosition(), 1).intersects(pickRay))
						{
							float dis = asMath::Distance(transform.GetPosition(), pickRay.origin);
							if (dis < hovered.distance)
							{
								hovered = asScene::PickResult();
								hovered.entity = entity;
								hovered.distance = dis;
							}
						}
					}
				}
				if (pickMask & PICK_CAMERA)
				{
					for (size_t i = 0; i < scene.cameras.GetCount(); ++i)
					{
						Entity entity = scene.cameras.GetEntity(i);

						const TransformComponent& transform = *scene.transforms.GetComponent(entity);

						XMVECTOR disV = XMVector3LinePointDistance(XMLoadFloat3(&pickRay.origin), XMLoadFloat3(&pickRay.origin) + XMLoadFloat3(&pickRay.direction), transform.GetPositionV());
						float dis = XMVectorGetX(disV);
						if (dis < asMath::Distance(transform.GetPosition(), pickRay.origin) * 0.05f && dis < hovered.distance)
						{
							hovered = asScene::PickResult();
							hovered.entity = entity;
							hovered.distance = dis;
						}
					}
				}
				if (pickMask & PICK_ARMATURE)
				{
					for (size_t i = 0; i < scene.armatures.GetCount(); ++i)
					{
						Entity entity = scene.armatures.GetEntity(i);
						const TransformComponent& transform = *scene.transforms.GetComponent(entity);

						XMVECTOR disV = XMVector3LinePointDistance(XMLoadFloat3(&pickRay.origin), XMLoadFloat3(&pickRay.origin) + XMLoadFloat3(&pickRay.direction), transform.GetPositionV());
						float dis = XMVectorGetX(disV);
						if (dis < asMath::Distance(transform.GetPosition(), pickRay.origin) * 0.05f && dis < hovered.distance)
						{
							hovered = asScene::PickResult();
							hovered.entity = entity;
							hovered.distance = dis;
						}
					}
				}
				if (pickMask & PICK_SOUND)
				{
					for (size_t i = 0; i < scene.sounds.GetCount(); ++i)
					{
						Entity entity = scene.sounds.GetEntity(i);
						const TransformComponent& transform = *scene.transforms.GetComponent(entity);

						XMVECTOR disV = XMVector3LinePointDistance(XMLoadFloat3(&pickRay.origin), XMLoadFloat3(&pickRay.origin) + XMLoadFloat3(&pickRay.direction), transform.GetPositionV());
						float dis = XMVectorGetX(disV);
						if (dis < asMath::Distance(transform.GetPosition(), pickRay.origin) * 0.05f && dis < hovered.distance)
						{
							hovered = asScene::PickResult();
							hovered.entity = entity;
							hovered.distance = dis;
						}
					}
				}

			}



			// Interact:
			if (hovered.entity != INVALID_ENTITY && selected.empty())
			{
				const ObjectComponent* object = scene.objects.GetComponent(hovered.entity);
				if (object != nullptr)
				{
					if (object->GetRenderTypes() & RENDERTYPE_WATER)
					{
						if (asInput::Down(asInput::MOUSE_BUTTON_LEFT))
						{
							// if water, then put a water ripple onto it:
							asRenderer::PutWaterRipple(asHelper::GetOriginalWorkingDirectory() + "assets/images/ripple.png", hovered.position);
						}
					}
					else
					{
						if (asInput::Press(asInput::MOUSE_BUTTON_LEFT))
						{
							//SoftBodyPhysicsComponent* softBody = scene.softbodies.GetComponent(object->meshID);
							//if (softBody != nullptr && asInput::Down((asInput::BUTTON)'P'))
							//{
							//	MeshComponent* mesh = scene.meshes.GetComponent(object->meshID);

							//	// If softbody, pin the triangle:
							//	if (softBody->graphicsToPhysicsVertexMapping.empty())
							//	{
							//		softBody->CreateFromMesh(*mesh);
							//	}
							//	uint32_t physicsIndex0 = softBody->graphicsToPhysicsVertexMapping[hovered.vertexID0];
							//	uint32_t physicsIndex1 = softBody->graphicsToPhysicsVertexMapping[hovered.vertexID1];
							//	uint32_t physicsIndex2 = softBody->graphicsToPhysicsVertexMapping[hovered.vertexID2];
							//	softBody->weights[physicsIndex0] = 0;
							//	softBody->weights[physicsIndex1] = 0;
							//	softBody->weights[physicsIndex2] = 0;
							//}
							//else
							//{
							//	// if not water or softbody, put a decal on it:
							//	static int decalselector = 0;
							//	decalselector = (decalselector + 1) % 2;
							//	Entity entity = scene.Entity_CreateDecal("editorDecal", asHelper::GetOriginalWorkingDirectory() + (decalselector == 0 ? "assets/images/leaf.dds" : "assets/images/blood1.png"));
							//	TransformComponent& transform = *scene.transforms.GetComponent(entity);
							//	transform.MatrixTransform(hovered.orientation);
							//	transform.RotateRollPitchYaw(XMFLOAT3(XM_PIDIV2, 0, 0));
							//	transform.Scale(XMFLOAT3(2, 2, 2));
							//	scene.Component_Attach(entity, hovered.entity);
							//}
						}
					}
				}

			}

			// Visualize soft body pinning:
			if (asInput::Down((asInput::BUTTON)'P'))
			{
				//for (size_t i = 0; i < scene.softbodies.GetCount(); ++i)
				//{
				//	const SoftBodyPhysicsComponent& softbody = scene.softbodies[i];
				//	Entity entity = scene.softbodies.GetEntity(i);
				//	const MeshComponent& mesh = *scene.meshes.GetComponent(entity);

				//	XMMATRIX W = XMLoadFloat4x4(&softbody.worldMatrix);
				//	int physicsIndex = 0;
				//	for (auto& weight : softbody.weights)
				//	{
				//		if (weight == 0)
				//		{
				//			asRenderer::RenderablePoint point;
				//			point.color = XMFLOAT4(1, 0, 0, 1);
				//			point.size = 0.2f;
				//			point.position = mesh.vertex_positions[softbody.physicsToGraphicsVertexMapping[physicsIndex]];
				//			if (!asPhysicsEngine::IsEnabled()) // todo: better
				//			{
				//				XMVECTOR P = XMLoadFloat3(&point.position);
				//				P = XMVector3Transform(P, W);
				//				XMStoreFloat3(&point.position, P);
				//			}
				//			asRenderer::AddRenderablePoint(point);
				//		}
				//		++physicsIndex;
				//	}
				//}
			}

			// Select...
			static bool selectAll = false;
			if (asInput::Press(asInput::MOUSE_BUTTON_RIGHT) || selectAll)
			{

				asArchive* archive = AdvanceHistory();
				*archive << HISTORYOP_SELECTION;
				// record PREVIOUS selection state...
				*archive << selected.size();
				for (auto& x : selected)
				{
					*archive << x.entity;
					*archive << x.position;
					*archive << x.normal;
					*archive << x.subsetIndex;
					*archive << x.distance;
				}
				savedHierarchy.Serialize(*archive);

				if (selectAll)
				{
					// Add everything to selection:
					selectAll = false;
					EndTranslate();

					for (size_t i = 0; i < scene.transforms.GetCount(); ++i)
					{
						Entity entity = scene.transforms.GetEntity(i);

						if (scene.hierarchy.Contains(entity))
						{
							// Parented animated objects don't work properly when attached to translator in bulk
							continue;
						}

						asScene::PickResult picked;
						picked.entity = entity;
						AddSelected(picked);
					}

					BeginTranslate();
				}
				else if (hovered.entity != INVALID_ENTITY)
				{
					// Add the hovered item to the selection:

					if (!selected.empty() && asInput::Down(asInput::KEYBOARD_BUTTON_LSHIFT))
					{
						// Union selection:
						list<asScene::PickResult> saved = selected;
						EndTranslate();
						selected.clear(); // endtranslate would clear it, but not if translator is not enabled
						for (const asScene::PickResult& picked : saved)
						{
							AddSelected(picked);
						}
						AddSelected(hovered);
					}
					else
					{
						// Replace selection:
						EndTranslate();
						selected.clear(); // endtranslate would clear it, but not if translator is not enabled
						AddSelected(hovered);
					}

					BeginTranslate();
				}
				else
				{
					// Clear selection:
					EndTranslate();
					selected.clear(); // endtranslate would clear it, but not if translator is not enabled
				}

				// record NEW selection state...
				*archive << selected.size();
				for (auto& x : selected)
				{
					*archive << x.entity;
					*archive << x.position;
					*archive << x.normal;
					*archive << x.subsetIndex;
					*archive << x.distance;
				}
				savedHierarchy.Serialize(*archive);
			}

			// Update window data bindings...
			if (selected.empty())
			{
				objectWnd->SetEntity(INVALID_ENTITY);
				emitterWnd->SetEntity(INVALID_ENTITY);
				hairWnd->SetEntity(INVALID_ENTITY);
				meshWnd->SetEntity(INVALID_ENTITY);
				materialWnd->SetEntity(INVALID_ENTITY);
				lightWnd->SetEntity(INVALID_ENTITY);
				soundWnd->SetEntity(INVALID_ENTITY);
				decalWnd->SetEntity(INVALID_ENTITY);
				envProbeWnd->SetEntity(INVALID_ENTITY);
				forceFieldWnd->SetEntity(INVALID_ENTITY);
				cameraWnd->SetEntity(INVALID_ENTITY);
			}
			else
			{
				const asScene::PickResult& picked = selected.back();

				assert(picked.entity != INVALID_ENTITY);

				objectWnd->SetEntity(INVALID_ENTITY);
				for (auto& x : selected)
				{
					if (scene.objects.GetComponent(x.entity) != nullptr)
					{
						objectWnd->SetEntity(x.entity);
						break;
					}
				}

				emitterWnd->SetEntity(picked.entity);
				hairWnd->SetEntity(picked.entity);
				lightWnd->SetEntity(picked.entity);
				soundWnd->SetEntity(picked.entity);
				decalWnd->SetEntity(picked.entity);
				envProbeWnd->SetEntity(picked.entity);
				forceFieldWnd->SetEntity(picked.entity);
				cameraWnd->SetEntity(picked.entity);

				if (picked.subsetIndex >= 0)
				{
					const ObjectComponent* object = scene.objects.GetComponent(picked.entity);
					if (object != nullptr) // maybe it was deleted...
					{
						meshWnd->SetEntity(object->meshID);

						const MeshComponent* mesh = scene.meshes.GetComponent(object->meshID);
						if (mesh != nullptr && (int)mesh->subsets.size() > picked.subsetIndex)
						{
							materialWnd->SetEntity(mesh->subsets[picked.subsetIndex].materialID);
						}
					}
				}
				else
				{
					materialWnd->SetEntity(picked.entity);
				}

			}

			// Clear highlite state:
			for (size_t i = 0; i < scene.materials.GetCount(); ++i)
			{
				scene.materials[i].SetUserStencilRef(EDITORSTENCILREF_CLEAR);
			}
			for (size_t i = 0; i < scene.objects.GetCount(); ++i)
			{
				scene.objects[i].SetUserStencilRef(EDITORSTENCILREF_CLEAR);
			}
			for (auto& x : selected)
			{
				if (x.subsetIndex >= 0)
				{
					ObjectComponent* object = scene.objects.GetComponent(x.entity);
					if (object != nullptr) // maybe it was deleted...
					{
						object->SetUserStencilRef(EDITORSTENCILREF_HIGHLIGHT_OBJECT);
						const MeshComponent* mesh = scene.meshes.GetComponent(object->meshID);
						if (mesh != nullptr && (int)mesh->subsets.size() > x.subsetIndex)
						{
							MaterialComponent* material = scene.materials.GetComponent(mesh->subsets[x.subsetIndex].materialID);
							if (material != nullptr)
							{
								material->SetUserStencilRef(EDITORSTENCILREF_HIGHLIGHT_MATERIAL);
							}
						}
					}
				}
			}

			// Delete
			if (asInput::Press(asInput::KEYBOARD_BUTTON_DELETE))
			{

				asArchive* archive = AdvanceHistory();
				*archive << HISTORYOP_DELETE;

				*archive << selected.size();
				for (auto& x : selected)
				{
					*archive << x.entity;
				}
				for (auto& x : selected)
				{
					scene.Entity_Serialize(*archive, x.entity);
				}
				for (auto& x : selected)
				{
					scene.Entity_Remove(x.entity);
					savedHierarchy.Remove_KeepSorted(x.entity);
				}

				EndTranslate();
			}

			// Control operations...
			if (asInput::Down(asInput::KEYBOARD_BUTTON_LCONTROL))
			{
				// Select All
				if (asInput::Press((asInput::BUTTON)'A'))
				{
					selectAll = true;
				}
				// Copy
				if (asInput::Press((asInput::BUTTON)'C'))
				{
					auto prevSel = selected;
					EndTranslate();

					SAFE_DELETE(clipboard);
					clipboard = new asArchive();
					*clipboard << prevSel.size();
					for (auto& x : prevSel)
					{
						scene.Entity_Serialize(*clipboard, x.entity, 0);
						AddSelected(x);
					}

					BeginTranslate();
				}
				// Paste
				if (asInput::Press((asInput::BUTTON)'V'))
				{
					auto prevSel = selected;
					EndTranslate();

					clipboard->SetReadModeAndResetPos(true);
					size_t count;
					*clipboard >> count;
					for (size_t i = 0; i < count; ++i)
					{
						asScene::PickResult picked;
						picked.entity = scene.Entity_Serialize(*clipboard, INVALID_ENTITY, asRandom::getRandom(1, INT_MAX), false);
						AddSelected(picked);
					}

					BeginTranslate();
				}
				// Duplicate Instances
				if (asInput::Press((asInput::BUTTON)'D'))
				{
					auto prevSel = selected;
					EndTranslate();
					for (auto& x : prevSel)
					{
						asScene::PickResult picked;
						picked.entity = scene.Entity_Duplicate(x.entity);
						AddSelected(picked);
					}
					BeginTranslate();
				}
				// Put Instances
				if (clipboard != nullptr && hovered.subsetIndex >= 0 && asInput::Down(asInput::KEYBOARD_BUTTON_LSHIFT) && asInput::Press(asInput::MOUSE_BUTTON_LEFT))
				{
					XMMATRIX M = XMLoadFloat4x4(&hovered.orientation);

					clipboard->SetReadModeAndResetPos(true);
					size_t count;
					*clipboard >> count;
					for (size_t i = 0; i < count; ++i)
					{
						Entity entity = scene.Entity_Serialize(*clipboard, INVALID_ENTITY, asRandom::getRandom(1, INT_MAX), false);
						TransformComponent* transform = scene.transforms.GetComponent(entity);
						if (transform != nullptr)
						{
							transform->ClearTransform();
							transform->MatrixTransform(M);
						}
					}
				}
				// Undo
				if (asInput::Press((asInput::BUTTON)'Z'))
				{
					ConsumeHistoryOperation(true);
				}
				// Redo
				if (asInput::Press((asInput::BUTTON)'Y'))
				{
					ConsumeHistoryOperation(false);
				}
			}

		}

		translator.Update();

		if (translator.IsDragEnded())
		{
			asArchive* archive = AdvanceHistory();
			*archive << HISTORYOP_TRANSLATOR;
			*archive << translator.GetDragStart();
			*archive << translator.GetDragEnd();
		}

		emitterWnd->UpdateData();
		hairWnd->UpdateData();

		// Follow camera proxy:
		if (cameraWnd->followCheckBox->IsEnabled() && cameraWnd->followCheckBox->GetCheck())
		{
			TransformComponent* proxy = scene.transforms.GetComponent(cameraWnd->proxy);
			if (proxy != nullptr)
			{
				cameraWnd->camera_transform.Lerp(cameraWnd->camera_transform, *proxy, 1.0f - cameraWnd->followSlider->GetValue());
				cameraWnd->camera_transform.UpdateTransform();
			}
		}

		camera.TransformCamera(cameraWnd->camera_transform);
		camera.UpdateCamera();

		__super::Update(dt);

		renderPath->Update(dt);
	}
	void EditorComponent::Render() const
	{
		Scene& scene = asScene::GetScene();

		// Hovered item boxes:
		if (!cinemaModeCheckBox->GetCheck())
		{
			if (hovered.entity != INVALID_ENTITY)
			{
				const ObjectComponent* object = scene.objects.GetComponent(hovered.entity);
				if (object != nullptr)
				{
					const AABB& aabb = *scene.aabb_objects.GetComponent(hovered.entity);

					XMFLOAT4X4 hoverBox;
					XMStoreFloat4x4(&hoverBox, aabb.getAsBoxMatrix());
					asRenderer::AddRenderableBox(hoverBox, XMFLOAT4(0.5f, 0.5f, 0.5f, 0.5f));
				}

				const LightComponent* light = scene.lights.GetComponent(hovered.entity);
				if (light != nullptr)
				{
					const AABB& aabb = *scene.aabb_lights.GetComponent(hovered.entity);

					XMFLOAT4X4 hoverBox;
					XMStoreFloat4x4(&hoverBox, aabb.getAsBoxMatrix());
					asRenderer::AddRenderableBox(hoverBox, XMFLOAT4(0.5f, 0.5f, 0, 0.5f));
				}

				const DecalComponent* decal = scene.decals.GetComponent(hovered.entity);
				if (decal != nullptr)
				{
					asRenderer::AddRenderableBox(decal->world, XMFLOAT4(0.5f, 0, 0.5f, 0.5f));
				}

				const EnvironmentProbeComponent* probe = scene.probes.GetComponent(hovered.entity);
				if (probe != nullptr)
				{
					const AABB& aabb = *scene.aabb_probes.GetComponent(hovered.entity);

					XMFLOAT4X4 hoverBox;
					XMStoreFloat4x4(&hoverBox, aabb.getAsBoxMatrix());
					asRenderer::AddRenderableBox(hoverBox, XMFLOAT4(0.5f, 0.5f, 0.5f, 0.5f));
				}

				const asHairParticle* hair = scene.hairs.GetComponent(hovered.entity);
				if (hair != nullptr)
				{
					XMFLOAT4X4 hoverBox;
					XMStoreFloat4x4(&hoverBox, hair->aabb.getAsBoxMatrix());
					asRenderer::AddRenderableBox(hoverBox, XMFLOAT4(0, 0.5f, 0, 0.5f));
				}
			}

		}

		// Selected items box:
		if (!cinemaModeCheckBox->GetCheck() && !selected.empty())
		{
			AABB selectedAABB = AABB(XMFLOAT3(FLT_MAX, FLT_MAX, FLT_MAX), XMFLOAT3(-FLT_MAX, -FLT_MAX, -FLT_MAX));
			for (auto& picked : selected)
			{
				if (picked.entity != INVALID_ENTITY)
				{
					const ObjectComponent* object = scene.objects.GetComponent(picked.entity);
					if (object != nullptr)
					{
						const AABB& aabb = *scene.aabb_objects.GetComponent(picked.entity);
						selectedAABB = AABB::Merge(selectedAABB, aabb);
					}

					const LightComponent* light = scene.lights.GetComponent(picked.entity);
					if (light != nullptr)
					{
						const AABB& aabb = *scene.aabb_lights.GetComponent(picked.entity);
						selectedAABB = AABB::Merge(selectedAABB, aabb);
					}

					const DecalComponent* decal = scene.decals.GetComponent(picked.entity);
					if (decal != nullptr)
					{
						const AABB& aabb = *scene.aabb_decals.GetComponent(picked.entity);
						selectedAABB = AABB::Merge(selectedAABB, aabb);

						// also display decal OBB:
						XMFLOAT4X4 selectionBox;
						selectionBox = decal->world;
						asRenderer::AddRenderableBox(selectionBox, XMFLOAT4(1, 0, 1, 1));
					}

					const EnvironmentProbeComponent* probe = scene.probes.GetComponent(picked.entity);
					if (probe != nullptr)
					{
						const AABB& aabb = *scene.aabb_probes.GetComponent(picked.entity);
						selectedAABB = AABB::Merge(selectedAABB, aabb);
					}

					const asHairParticle* hair = scene.hairs.GetComponent(picked.entity);
					if (hair != nullptr)
					{
						selectedAABB = AABB::Merge(selectedAABB, hair->aabb);
					}

				}
			}

			XMFLOAT4X4 selectionBox;
			XMStoreFloat4x4(&selectionBox, selectedAABB.getAsBoxMatrix());
			asRenderer::AddRenderableBox(selectionBox, XMFLOAT4(1, 1, 1, 1));
		}

		renderPath->Render();

		// Selection outline:
		if (renderPath->GetDepthStencil() != nullptr && !selected.empty())
		{
			GraphicsDevice* device = asRenderer::GetDevice();
			CommandList cmd = device->BeginCommandList();

			device->EventBegin("Editor - Selection Outline", cmd);

			Viewport vp;
			vp.Width = (float)rt_selectionOutline[0].GetDesc().Width;
			vp.Height = (float)rt_selectionOutline[0].GetDesc().Height;
			device->BindViewports(1, &vp, cmd);

			asImageParams fx;
			fx.enableFullScreen();
			fx.stencilComp = STENCILMODE::STENCILMODE_EQUAL;

			// We will specify the stencil ref in user-space, don't care about engine stencil refs here:
			//	Otherwise would need to take into account engine ref and draw multiple permutations of stencil refs.
			fx.stencilRefMode = STENCILREFMODE_USER;

			device->RenderPassBegin(&renderpass_selectionOutline[1], cmd); // this renderpass just clears so its empty
			device->RenderPassEnd(cmd);

			// Materials outline (green):
			{
				device->RenderPassBegin(&renderpass_selectionOutline[0], cmd);

				// Draw solid blocks of selected materials
				fx.stencilRef = EDITORSTENCILREF_HIGHLIGHT_MATERIAL;
				asImage::Draw(asTextureHelper::getWhite(), fx, cmd);

				device->RenderPassEnd(cmd);

				if (renderPath->getMSAASampleCount() > 1)
				{
					device->MSAAResolve(&rt_selectionOutline[0], &rt_selectionOutline_MSAA, cmd);
				}

				// Outline the solid blocks:
				asRenderer::BindCommonResources(cmd);
				asRenderer::Postprocess_Outline(rt_selectionOutline[0], rt_selectionOutline[1], cmd, 0.1f, 1, selectionColor2);
			}

			// Objects outline (orange):
			{
				device->RenderPassBegin(&renderpass_selectionOutline[0], cmd);

				// Draw solid blocks of selected objects
				fx.stencilRef = EDITORSTENCILREF_HIGHLIGHT_OBJECT;
				asImage::Draw(asTextureHelper::getWhite(), fx, cmd);

				device->RenderPassEnd(cmd);

				if (renderPath->getMSAASampleCount() > 1)
				{
					device->MSAAResolve(&rt_selectionOutline[0], &rt_selectionOutline_MSAA, cmd);
				}

				// Outline the solid blocks:
				asRenderer::BindCommonResources(cmd);
				asRenderer::Postprocess_Outline(rt_selectionOutline[0], rt_selectionOutline[1], cmd, 0.1f, 1, selectionColor);
			}

			device->EventEnd(cmd);
		}

		__super::Render();

	}
	void EditorComponent::Compose(CommandList cmd) const
	{
		renderPath->Compose(cmd);

		if (cinemaModeCheckBox->GetCheck())
		{
			return;
		}

		// Compose the selection outline to the screen:
		const float selectionColorIntensity = std::sinf(selectionOutlineTimer * XM_2PI * 0.8f) * 0.5f + 0.5f;
		if (renderPath->GetDepthStencil() != nullptr && !selected.empty())
		{
			asImageParams fx;
			fx.enableFullScreen();
			fx.opacity = asMath::Lerp(0.4f, 1.0f, selectionColorIntensity);
			asImage::Draw(&rt_selectionOutline[1], fx, cmd);
		}

		const CameraComponent& camera = asRenderer::GetCamera();

		Scene& scene = asScene::GetScene();

		const XMFLOAT4 selectedEntityColor = asMath::Lerp(asMath::Lerp(XMFLOAT4(1, 1, 1, 1), selectionColor, 0.4f), selectionColor, selectionColorIntensity);

		if (rendererWnd->GetPickType() & PICK_LIGHT)
		{
			for (size_t i = 0; i < scene.lights.GetCount(); ++i)
			{
				const LightComponent& light = scene.lights[i];
				Entity entity = scene.lights.GetEntity(i);
				const TransformComponent& transform = *scene.transforms.GetComponent(entity);

				float dist = asMath::Distance(transform.GetPosition(), camera.Eye) * 0.08f;

				asImageParams fx;
				fx.pos = transform.GetPosition();
				fx.siz = XMFLOAT2(dist, dist);
				fx.typeFlag = ImageType::WORLD;
				fx.pivot = XMFLOAT2(0.5f, 0.5f);
				fx.col = XMFLOAT4(1, 1, 1, 0.5f);

				if (hovered.entity == entity)
				{
					fx.col = XMFLOAT4(1, 1, 1, 1);
				}
				for (auto& picked : selected)
				{
					if (picked.entity == entity)
					{
						fx.col = selectedEntityColor;
						break;
					}
				}

				switch (light.GetType())
				{
				case LightComponent::POINT:
					asImage::Draw(pointLightTex->texture, fx, cmd);
					break;
				case LightComponent::SPOT:
					asImage::Draw(spotLightTex->texture, fx, cmd);
					break;
				case LightComponent::DIRECTIONAL:
					asImage::Draw(dirLightTex->texture, fx, cmd);
					break;
				default:
					asImage::Draw(areaLightTex->texture, fx, cmd);
					break;
				}
			}
		}


		if (rendererWnd->GetPickType() & PICK_DECAL)
		{
			for (size_t i = 0; i < scene.decals.GetCount(); ++i)
			{
				Entity entity = scene.decals.GetEntity(i);
				const TransformComponent& transform = *scene.transforms.GetComponent(entity);

				float dist = asMath::Distance(transform.GetPosition(), camera.Eye) * 0.08f;

				asImageParams fx;
				fx.pos = transform.GetPosition();
				fx.siz = XMFLOAT2(dist, dist);
				fx.typeFlag = ImageType::WORLD;
				fx.pivot = XMFLOAT2(0.5f, 0.5f);
				fx.col = XMFLOAT4(1, 1, 1, 0.5f);

				if (hovered.entity == entity)
				{
					fx.col = XMFLOAT4(1, 1, 1, 1);
				}
				for (auto& picked : selected)
				{
					if (picked.entity == entity)
					{
						fx.col = selectedEntityColor;
						break;
					}
				}


				asImage::Draw(decalTex->texture, fx, cmd);

			}
		}

		if (rendererWnd->GetPickType() & PICK_FORCEFIELD)
		{
			for (size_t i = 0; i < scene.forces.GetCount(); ++i)
			{
				Entity entity = scene.forces.GetEntity(i);
				const TransformComponent& transform = *scene.transforms.GetComponent(entity);

				float dist = asMath::Distance(transform.GetPosition(), camera.Eye) * 0.08f;

				asImageParams fx;
				fx.pos = transform.GetPosition();
				fx.siz = XMFLOAT2(dist, dist);
				fx.typeFlag = ImageType::WORLD;
				fx.pivot = XMFLOAT2(0.5f, 0.5f);
				fx.col = XMFLOAT4(1, 1, 1, 0.5f);

				if (hovered.entity == entity)
				{
					fx.col = XMFLOAT4(1, 1, 1, 1);
				}
				for (auto& picked : selected)
				{
					if (picked.entity == entity)
					{
						fx.col = selectedEntityColor;
						break;
					}
				}


				asImage::Draw(forceFieldTex->texture, fx, cmd);
			}
		}

		if (rendererWnd->GetPickType() & PICK_CAMERA)
		{
			for (size_t i = 0; i < scene.cameras.GetCount(); ++i)
			{
				Entity entity = scene.cameras.GetEntity(i);

				const TransformComponent& transform = *scene.transforms.GetComponent(entity);

				float dist = asMath::Distance(transform.GetPosition(), camera.Eye) * 0.08f;

				asImageParams fx;
				fx.pos = transform.GetPosition();
				fx.siz = XMFLOAT2(dist, dist);
				fx.typeFlag = ImageType::WORLD;
				fx.pivot = XMFLOAT2(0.5f, 0.5f);
				fx.col = XMFLOAT4(1, 1, 1, 0.5f);

				if (hovered.entity == entity)
				{
					fx.col = XMFLOAT4(1, 1, 1, 1);
				}
				for (auto& picked : selected)
				{
					if (picked.entity == entity)
					{
						fx.col = selectedEntityColor;
						break;
					}
				}


				asImage::Draw(cameraTex->texture, fx, cmd);
			}
		}

		if (rendererWnd->GetPickType() & PICK_ARMATURE)
		{
			for (size_t i = 0; i < scene.armatures.GetCount(); ++i)
			{
				Entity entity = scene.armatures.GetEntity(i);
				const TransformComponent& transform = *scene.transforms.GetComponent(entity);

				float dist = asMath::Distance(transform.GetPosition(), camera.Eye) * 0.08f;

				asImageParams fx;
				fx.pos = transform.GetPosition();
				fx.siz = XMFLOAT2(dist, dist);
				fx.typeFlag = ImageType::WORLD;
				fx.pivot = XMFLOAT2(0.5f, 0.5f);
				fx.col = XMFLOAT4(1, 1, 1, 0.5f);

				if (hovered.entity == entity)
				{
					fx.col = XMFLOAT4(1, 1, 1, 1);
				}
				for (auto& picked : selected)
				{
					if (picked.entity == entity)
					{
						fx.col = selectedEntityColor;
						break;
					}
				}


				asImage::Draw(armatureTex->texture, fx, cmd);
			}
		}

		if (rendererWnd->GetPickType() & PICK_EMITTER)
		{
			for (size_t i = 0; i < scene.emitters.GetCount(); ++i)
			{
				Entity entity = scene.emitters.GetEntity(i);
				const TransformComponent& transform = *scene.transforms.GetComponent(entity);

				float dist = asMath::Distance(transform.GetPosition(), camera.Eye) * 0.08f;

				asImageParams fx;
				fx.pos = transform.GetPosition();
				fx.siz = XMFLOAT2(dist, dist);
				fx.typeFlag = ImageType::WORLD;
				fx.pivot = XMFLOAT2(0.5f, 0.5f);
				fx.col = XMFLOAT4(1, 1, 1, 0.5f);

				if (hovered.entity == entity)
				{
					fx.col = XMFLOAT4(1, 1, 1, 1);
				}
				for (auto& picked : selected)
				{
					if (picked.entity == entity)
					{
						fx.col = selectedEntityColor;
						break;
					}
				}


				asImage::Draw(emitterTex->texture, fx, cmd);
			}
		}

		if (rendererWnd->GetPickType() & PICK_HAIR)
		{
			for (size_t i = 0; i < scene.hairs.GetCount(); ++i)
			{
				Entity entity = scene.hairs.GetEntity(i);
				const TransformComponent& transform = *scene.transforms.GetComponent(entity);

				float dist = asMath::Distance(transform.GetPosition(), camera.Eye) * 0.08f;

				asImageParams fx;
				fx.pos = transform.GetPosition();
				fx.siz = XMFLOAT2(dist, dist);
				fx.typeFlag = ImageType::WORLD;
				fx.pivot = XMFLOAT2(0.5f, 0.5f);
				fx.col = XMFLOAT4(1, 1, 1, 0.5f);

				if (hovered.entity == entity)
				{
					fx.col = XMFLOAT4(1, 1, 1, 1);
				}
				for (auto& picked : selected)
				{
					if (picked.entity == entity)
					{
						fx.col = selectedEntityColor;
						break;
					}
				}


				asImage::Draw(hairTex->texture, fx, cmd);
			}
		}

		if (rendererWnd->GetPickType() & PICK_SOUND)
		{
			for (size_t i = 0; i < scene.sounds.GetCount(); ++i)
			{
				Entity entity = scene.sounds.GetEntity(i);
				const TransformComponent& transform = *scene.transforms.GetComponent(entity);

				float dist = asMath::Distance(transform.GetPosition(), camera.Eye) * 0.08f;

				asImageParams fx;
				fx.pos = transform.GetPosition();
				fx.siz = XMFLOAT2(dist, dist);
				fx.typeFlag = ImageType::WORLD;
				fx.pivot = XMFLOAT2(0.5f, 0.5f);
				fx.col = XMFLOAT4(1, 1, 1, 0.5f);

				if (hovered.entity == entity)
				{
					fx.col = XMFLOAT4(1, 1, 1, 1);
				}
				for (auto& picked : selected)
				{
					if (picked.entity == entity)
					{
						fx.col = selectedEntityColor;
						break;
					}
				}


				asImage::Draw(soundTex->texture, fx, cmd);
			}
		}


		if (!selected.empty() && translator.enabled)
		{
			translator.Draw(camera, cmd);
		}

		__super::Compose(cmd);
	}
	void EditorComponent::Unload()
	{
		renderPath->Unload();

		__super::Unload();
	}



	void EditorComponent::BeginTranslate()
	{
		if (selected.empty() || !translator.enabled)
		{
			return;
		}

		Scene& scene = asScene::GetScene();

		// Insert translator into scene:
		scene.transforms.Create(translator.entityID);

		// Begin translation, save scene hierarchy from before:
		savedHierarchy.Copy(scene.hierarchy);

		// All selected entities will be attached to translator entity:
		TransformComponent* translator_transform = asScene::GetScene().transforms.GetComponent(translator.entityID);
		translator_transform->ClearTransform();

		// Find the center of all the entities that are selected:
		XMVECTOR centerV = XMVectorSet(0, 0, 0, 0);
		float count = 0;
		for (auto& x : selected)
		{
			TransformComponent* transform = asScene::GetScene().transforms.GetComponent(x.entity);
			if (transform != nullptr)
			{
				centerV = XMVectorAdd(centerV, transform->GetPositionV());
				count += 1.0f;
			}
		}

		// Offset translator to center position and perform attachments:
		if (count > 0)
		{
			centerV /= count;
			XMFLOAT3 center;
			XMStoreFloat3(&center, centerV);
			translator_transform->ClearTransform();
			translator_transform->Translate(center);
			translator_transform->UpdateTransform();

			for (auto& x : selected)
			{
				asScene::GetScene().Component_Attach(x.entity, translator.entityID);
			}
		}
	}
	void EditorComponent::EndTranslate()
	{
		if (selected.empty() || !translator.enabled)
		{
			return;
		}

		Scene& scene = asScene::GetScene();

		// Remove translator from scene:
		scene.Entity_Remove(translator.entityID);

		// Translation ended, apply all final transformations as local pose:
		for (size_t i = 0; i < scene.hierarchy.GetCount(); ++i)
		{
			HierarchyComponent& parent = scene.hierarchy[i];

			if (parent.parentID == translator.entityID) // only to entities that were attached to translator!
			{
				Entity entity = scene.hierarchy.GetEntity(i);
				TransformComponent* transform = scene.transforms.GetComponent(entity);
				if (transform != nullptr)
				{
					transform->ApplyTransform(); // (**)
				}
			}
		}

		// Restore scene hierarchy from before translation:
		scene.hierarchy.Copy(savedHierarchy);

		// If an attached entity got moved, then the world transform was applied to it (**),
		//	so we need to reattach it properly to the parent matrix:
		for (const asScene::PickResult& x : selected)
		{
			HierarchyComponent* parent = scene.hierarchy.GetComponent(x.entity);
			if (parent != nullptr)
			{
				TransformComponent* transform_parent = scene.transforms.GetComponent(parent->parentID);
				if (transform_parent != nullptr)
				{
					// Save the parent's inverse worldmatrix:
					XMStoreFloat4x4(&parent->world_parent_inverse_bind, XMMatrixInverse(nullptr, XMLoadFloat4x4(&transform_parent->world)));

					TransformComponent* transform_child = scene.transforms.GetComponent(x.entity);
					if (transform_child != nullptr)
					{
						// Child updated immediately, to that it can be immediately attached to afterwards:
						transform_child->UpdateTransform_Parented(*transform_parent, parent->world_parent_inverse_bind);
					}
				}

			}
		}

		selected.clear();
	}
	void EditorComponent::AddSelected(const asScene::PickResult& picked)
	{
		for (auto it = selected.begin(); it != selected.end(); ++it)
		{
			if ((*it) == picked)
			{
				// If already selected, it will be deselected now:
				selected.erase(it);
				return;
			}
		}

		selected.push_back(picked);
	}

	void EditorComponent::ResetHistory()
	{
		historyPos = -1;

		for (auto& x : history)
		{
			SAFE_DELETE(x);
		}
		history.clear();
	}
	asArchive* EditorComponent::AdvanceHistory()
	{
		historyPos++;

		while (static_cast<int>(history.size()) > historyPos)
		{
			SAFE_DELETE(history.back());
			history.pop_back();
		}

		asArchive* archive = new asArchive;
		archive->SetReadModeAndResetPos(false);
		history.push_back(archive);

		return archive;
	}
	void EditorComponent::ConsumeHistoryOperation(bool undo)
	{
		if ((undo && historyPos >= 0) || (!undo && historyPos < (int)history.size() - 1))
		{
			if (!undo)
			{
				historyPos++;
			}

			asArchive* archive = history[historyPos];
			archive->SetReadModeAndResetPos(true);

			int temp;
			*archive >> temp;
			HistoryOperationType type = (HistoryOperationType)temp;

			switch (type)
			{
			case HISTORYOP_TRANSLATOR:
			{
				XMFLOAT4X4 start, end;
				*archive >> start >> end;
				translator.enabled = true;

				Scene& scene = asScene::GetScene();

				TransformComponent& transform = *scene.transforms.GetComponent(translator.entityID);
				transform.ClearTransform();
				if (undo)
				{
					transform.MatrixTransform(XMLoadFloat4x4(&start));
				}
				else
				{
					transform.MatrixTransform(XMLoadFloat4x4(&end));
				}
			}
			break;
			case HISTORYOP_DELETE:
			{
				Scene& scene = asScene::GetScene();

				size_t count;
				*archive >> count;
				vector<Entity> deletedEntities(count);
				for (size_t i = 0; i < count; ++i)
				{
					*archive >> deletedEntities[i];
				}

				if (undo)
				{
					for (size_t i = 0; i < count; ++i)
					{
						scene.Entity_Serialize(*archive);
					}
				}
				else
				{
					for (size_t i = 0; i < count; ++i)
					{
						scene.Entity_Remove(deletedEntities[i]);
					}
				}

			}
			break;
			case HISTORYOP_SELECTION:
			{
				EndTranslate();

				// Read selections states from archive:

				list<asScene::PickResult> selectedBEFORE;
				size_t selectionCountBEFORE;
				*archive >> selectionCountBEFORE;
				for (size_t i = 0; i < selectionCountBEFORE; ++i)
				{
					asScene::PickResult sel;
					*archive >> sel.entity;
					*archive >> sel.position;
					*archive >> sel.normal;
					*archive >> sel.subsetIndex;
					*archive >> sel.distance;

					selectedBEFORE.push_back(sel);
				}
				ComponentManager<HierarchyComponent> savedHierarchyBEFORE;
				savedHierarchyBEFORE.Serialize(*archive);

				list<asScene::PickResult> selectedAFTER;
				size_t selectionCountAFTER;
				*archive >> selectionCountAFTER;
				for (size_t i = 0; i < selectionCountAFTER; ++i)
				{
					asScene::PickResult sel;
					*archive >> sel.entity;
					*archive >> sel.position;
					*archive >> sel.normal;
					*archive >> sel.subsetIndex;
					*archive >> sel.distance;

					selectedAFTER.push_back(sel);
				}
				ComponentManager<HierarchyComponent> savedHierarchyAFTER;
				savedHierarchyAFTER.Serialize(*archive);


				// Restore proper selection state:

				if (undo)
				{
					selected = selectedBEFORE;
					savedHierarchy.Copy(savedHierarchyBEFORE);
				}
				else
				{
					selected = selectedAFTER;
					savedHierarchy.Copy(savedHierarchyAFTER);
				}

				BeginTranslate();
			}
			break;
			case HISTORYOP_NONE:
				assert(0);
				break;
			default:
				break;
			}

			if (undo)
			{
				historyPos--;
			}
		}
	}
}

#pragma once
#include "Translator.h"

namespace as
{
	class MaterialWindow;
	class PostprocessWindow;
	class WeatherWindow;
	class ObjectWindow;
	class MeshWindow;
	class CameraWindow;
	class RendererWindow;
	class EnvProbeWindow;
	class DecalWindow;
	class LightWindow;
	class AnimationWindow;
	class EmitterWindow;
	class HairParticleWindow;
	class ForceFieldWindow;
	class SoundWindow;

	class EditorLoadingScreen : public LoadingScreen
	{
	private:
		as::asSprite sprite;
		as::asFont font;
	public:
		void Load() override;
		void Update(float dt) override;
		void Unload() override;
	};
	class Editor;
	class EditorComponent : public RenderPath2D
	{
	private:
		std::shared_ptr<asResource> pointLightTex, spotLightTex, dirLightTex, areaLightTex, decalTex, forceFieldTex, emitterTex, hairTex, cameraTex, armatureTex, soundTex;
	public:
		std::unique_ptr<MaterialWindow>			materialWnd;
		std::unique_ptr<PostprocessWindow>		postprocessWnd;
		std::unique_ptr<WeatherWindow>			weatherWnd;
		std::unique_ptr<ObjectWindow>			objectWnd;
		std::unique_ptr<MeshWindow>				meshWnd;
		std::unique_ptr<CameraWindow>			cameraWnd;
		std::unique_ptr<RendererWindow>			rendererWnd;
		std::unique_ptr<EnvProbeWindow>			envProbeWnd;
		std::unique_ptr<DecalWindow>			decalWnd;
		std::unique_ptr<SoundWindow>			soundWnd;
		std::unique_ptr<LightWindow>			lightWnd;
		std::unique_ptr<AnimationWindow>		animWnd;
		std::unique_ptr<EmitterWindow>			emitterWnd;
		std::unique_ptr<HairParticleWindow>		hairWnd;
		std::unique_ptr<ForceFieldWindow>		forceFieldWnd;

		Editor* main = nullptr;

		asCheckBox* cinemaModeCheckBox = nullptr;

		EditorLoadingScreen* loader = nullptr;
		RenderPath3D* renderPath = nullptr;
		enum RENDERPATH
		{
			RENDERPATH_FORWARD,
			RENDERPATH_DEFERRED,
			RENDERPATH_TILEDFORWARD,
			RENDERPATH_TILEDDEFERRED,
			RENDERPATH_PATHTRACING,
		};
		void ChangeRenderPath(RENDERPATH path);

		void ResizeBuffers() override;
		void Load() override;
		void Start() override;
		void FixedUpdate() override;
		void Update(float dt) override;
		void Render() const override;
		void Compose(asGraphics::CommandList cmd) const override;
		void Unload() override;


		enum EDITORSTENCILREF
		{
			EDITORSTENCILREF_CLEAR = 0x00,
			EDITORSTENCILREF_HIGHLIGHT_OBJECT = 0x01,
			EDITORSTENCILREF_HIGHLIGHT_MATERIAL = 0x02,
			EDITORSTENCILREF_LAST = 0x0F,
		};
		asGraphics::Texture rt_selectionOutline_MSAA;
		asGraphics::Texture rt_selectionOutline[2];
		asGraphics::RenderPass renderpass_selectionOutline[2];
		float selectionOutlineTimer = 0;
		const XMFLOAT4 selectionColor = XMFLOAT4(1, 0.6f, 0, 1);
		const XMFLOAT4 selectionColor2 = XMFLOAT4(0, 1, 0.6f, 0.35f);

		Translator translator;
		std::list<asScene::PickResult> selected;
		asECS::ComponentManager<asScene::HierarchyComponent> savedHierarchy;
		asScene::PickResult hovered;

		void BeginTranslate();
		void EndTranslate();
		void AddSelected(const asScene::PickResult& picked);

		asArchive* clipboard = nullptr;

		std::vector<asArchive*> history;
		int historyPos = -1;
		enum HistoryOperationType
		{
			HISTORYOP_TRANSLATOR,
			HISTORYOP_DELETE,
			HISTORYOP_SELECTION,
			HISTORYOP_NONE
		};

		void ResetHistory();
		asArchive* AdvanceHistory();
		void ConsumeHistoryOperation(bool undo);
	};


	class Editor : public MainComponent
	{
	public:
		Editor(){}
		~Editor() {}		
		
		EditorComponent* renderComponent = nullptr;
		EditorLoadingScreen* loader = nullptr;

		virtual void Initialize()override; 


	};

}
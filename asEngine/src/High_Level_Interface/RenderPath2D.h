#pragma once
#include "RenderPath.h"
#include "GUI/asGUI.h"

#include <string>



namespace as
{
	class asSprite;
	class asFont;

	static const std::string DEFAULT_RENDERLAYER = "default";
	struct RenderItem2D
	{
		enum TYPE
		{
			SPRITE,
			FONT,
		} type;
		asSprite* sprite = nullptr;
		asFont* font = nullptr;
		int order = 0;
	};
	struct RenderLayer2D
	{
		std::vector<RenderItem2D> items;
		std::string name;
		int order = 0;

		RenderLayer2D(const std::string& name) :name(name) {}
	};


	class RenderPath2D :
		public RenderPath
	{
	private:
		as::asGraphics::Texture rtStenciled;
		as::asGraphics::Texture rtFinal;

		as::asGraphics::RenderPass renderpass_stenciled;
		as::asGraphics::RenderPass renderpass_final;

		asGUI GUI;
		float spriteSpeed = 1.0f;

	protected:
		void ResizeBuffers() override;
	public:
		RenderPath2D();

		virtual void Initialize() override;
		virtual void Load() override;
		virtual void Unload() override;
		virtual void Start() override;
		virtual void Update(float dt) override;
		virtual void FixedUpdate() override;
		virtual void Render() const override;
		virtual void Compose(as::asGraphics::CommandList cmd) const override;

		const as::asGraphics::Texture& GetRenderResult() const { return rtFinal; }
		virtual const as::asGraphics::Texture* GetDepthStencil() const { return nullptr; }

		void addSprite(asSprite* sprite, const std::string& layer = DEFAULT_RENDERLAYER);
		void removeSprite(asSprite* sprite);
		void clearSprites();
		void setSpriteSpeed(float value) { spriteSpeed = value; }
		float getSpriteSpeed() { return spriteSpeed; }
		int getSpriteOrder(asSprite* sprite);

		void addFont(asFont* font, const std::string& layer = DEFAULT_RENDERLAYER);
		void removeFont(asFont* font);
		void clearFonts();
		int getFontOrder(asFont* font);

		std::vector<RenderLayer2D> layers;
		void addLayer(const std::string& name);
		void setLayerOrder(const std::string& name, int order);
		void SetSpriteOrder(asSprite* sprite, int order);
		void SetFontOrder(asFont* font, int order);
		void SortLayers();
		void CleanLayers();

		const asGUI& GetGUI() const { return GUI; }
		asGUI& GetGUI() { return GUI; }
	};

}

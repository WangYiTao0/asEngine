#include "aspch.h"
#include "RenderPath2D.h"
#include "Helpers/asResourceManager.h"
#include "Graphics/asSprite.h"
#include "Graphics/asFont.h"
#include "Graphics/asRenderer.h"

namespace as
{
	using namespace asGraphics;

	RenderPath2D::RenderPath2D()
	{
		addLayer(DEFAULT_RENDERLAYER);
	}

	void RenderPath2D::ResizeBuffers()
	{
		RenderPath::ResizeBuffers();

		GraphicsDevice* device = asRenderer::GetDevice();

		FORMAT defaultTextureFormat = device->GetBackBufferFormat();

		if (GetDepthStencil() != nullptr && asRenderer::GetResolutionScale() != 1.0f)
		{
			TextureDesc desc;
			desc.BindFlags = BIND_RENDER_TARGET | BIND_SHADER_RESOURCE;
			desc.Format = defaultTextureFormat;
			desc.Width = asRenderer::GetInternalResolution().x;
			desc.Height = asRenderer::GetInternalResolution().y;
			device->CreateTexture(&desc, nullptr, &rtStenciled);
			device->SetName(&rtStenciled, "rtStenciled");
		}
		{
			TextureDesc desc;
			desc.BindFlags = BIND_RENDER_TARGET | BIND_SHADER_RESOURCE;
			desc.Format = defaultTextureFormat;
			desc.Width = device->GetScreenWidth();
			desc.Height = device->GetScreenHeight();
			device->CreateTexture(&desc, nullptr, &rtFinal);
			device->SetName(&rtFinal, "rtFinal");
		}

		const Texture* dsv = GetDepthStencil();
		if (dsv != nullptr && asRenderer::GetResolutionScale() != 1.0f)
		{
			RenderPassDesc desc;
			desc.numAttachments = 2;
			desc.attachments[0] = { RenderPassAttachment::RENDERTARGET,RenderPassAttachment::LOADOP_CLEAR,&rtStenciled,-1 };
			desc.attachments[1] = { RenderPassAttachment::DEPTH_STENCIL,RenderPassAttachment::LOADOP_LOAD,dsv,-1 };

			device->CreateRenderPass(&desc, &renderpass_stenciled);

			dsv = nullptr;
		}
		{
			RenderPassDesc desc;
			desc.numAttachments = 1;
			desc.attachments[0] = { RenderPassAttachment::RENDERTARGET,RenderPassAttachment::LOADOP_CLEAR,&rtFinal,-1 };

			if (dsv != nullptr)
			{
				desc.numAttachments = 2;
				desc.attachments[1] = { RenderPassAttachment::DEPTH_STENCIL,RenderPassAttachment::LOADOP_LOAD,dsv,-1 };
			}

			device->CreateRenderPass(&desc, &renderpass_final);
		}

	}

	void RenderPath2D::Initialize()
	{
		RenderPath::Initialize();
	}

	void RenderPath2D::Load()
	{
		RenderPath::Load();
	}
	void RenderPath2D::Unload()
	{
		for (auto& x : layers)
		{
			for (auto& y : x.items)
			{
				if (y.sprite != nullptr)
				{
					delete y.sprite;
				}
				if (y.font != nullptr)
				{
					delete y.font;
				}
			}
		}
		layers.clear();

		RenderPath::Unload();
	}
	void RenderPath2D::Start()
	{
		RenderPath::Start();
	}
	void RenderPath2D::Update(float dt)
	{
		GetGUI().Update(dt);

		for (auto& x : layers)
		{
			for (auto& y : x.items)
			{
				if (y.sprite != nullptr)
				{
					y.sprite->Update(dt * getSpriteSpeed());
				}
			}
		}

		RenderPath::Update(dt);
	}
	void RenderPath2D::FixedUpdate()
	{
		for (auto& x : layers)
		{
			for (auto& y : x.items)
			{
				if (y.sprite != nullptr)
				{
					y.sprite->FixedUpdate(getSpriteSpeed());
				}
			}
		}

		RenderPath::FixedUpdate();
	}
	void RenderPath2D::Render() const
	{
		GraphicsDevice* device = asRenderer::GetDevice();
		CommandList cmd = device->BeginCommandList();


		// Special care for internal resolution, because stencil buffer is of internal resolution, 
		//	so we might need to render stencil sprites to separate render target that matches internal resolution!
		if (GetDepthStencil() != nullptr && asRenderer::GetResolutionScale() != 1.0f)
		{
			device->RenderPassBegin(&renderpass_stenciled, cmd);

			Viewport vp;
			vp.Width = (float)rtStenciled.GetDesc().Width;
			vp.Height = (float)rtStenciled.GetDesc().Height;
			device->BindViewports(1, &vp, cmd);

			asRenderer::GetDevice()->EventBegin("STENCIL Sprite Layers", cmd);
			for (auto& x : layers)
			{
				for (auto& y : x.items)
				{
					if (y.sprite != nullptr && y.sprite->params.stencilComp != STENCILMODE_DISABLED)
					{
						y.sprite->Draw(cmd);
					}
				}
			}
			asRenderer::GetDevice()->EventEnd(cmd);

			device->RenderPassEnd(cmd);
		}

		device->RenderPassBegin(&renderpass_final, cmd);

		Viewport vp;
		vp.Width = (float)rtFinal.GetDesc().Width;
		vp.Height = (float)rtFinal.GetDesc().Height;
		device->BindViewports(1, &vp, cmd);

		if (GetDepthStencil() != nullptr)
		{
			if (asRenderer::GetResolutionScale() != 1.0f)
			{
				asRenderer::GetDevice()->EventBegin("Copy STENCIL Sprite Layers", cmd);
				asImageParams fx;
				fx.enableFullScreen();
				asImage::Draw(&rtStenciled, fx, cmd);
				asRenderer::GetDevice()->EventEnd(cmd);
			}
			else
			{
				asRenderer::GetDevice()->EventBegin("STENCIL Sprite Layers", cmd);
				for (auto& x : layers)
				{
					for (auto& y : x.items)
					{
						if (y.sprite != nullptr && y.sprite->params.stencilComp != STENCILMODE_DISABLED)
						{
							y.sprite->Draw(cmd);
						}
					}
				}
				asRenderer::GetDevice()->EventEnd(cmd);
			}
		}

		asRenderer::GetDevice()->EventBegin("Sprite Layers", cmd);
		for (auto& x : layers)
		{
			for (auto& y : x.items)
			{
				if (y.sprite != nullptr && y.sprite->params.stencilComp == STENCILMODE_DISABLED)
				{
					y.sprite->Draw(cmd);
				}
				if (y.font != nullptr)
				{
					y.font->Draw(cmd);
				}
			}
		}
		asRenderer::GetDevice()->EventEnd(cmd);

		GetGUI().Render(cmd);

		device->RenderPassEnd(cmd);

		RenderPath::Render();
	}
	void RenderPath2D::Compose(CommandList cmd) const
	{
		asImageParams fx;
		fx.enableFullScreen();
		fx.blendFlag = BLENDMODE_PREMULTIPLIED;

		asImage::Draw(&rtFinal, fx, cmd);

		RenderPath::Compose(cmd);
	}


	void RenderPath2D::addSprite(asSprite* sprite, const std::string& layer)
	{
		for (auto& x : layers)
		{
			if (!x.name.compare(layer))
			{
				x.items.push_back(RenderItem2D());
				x.items.back().type = RenderItem2D::SPRITE;
				x.items.back().sprite = sprite;
			}
		}
		SortLayers();
	}
	void RenderPath2D::removeSprite(asSprite* sprite)
	{
		for (auto& x : layers)
		{
			for (auto& y : x.items)
			{
				if (y.sprite == sprite)
				{
					y.sprite = nullptr;
				}
			}
		}
		CleanLayers();
	}
	void RenderPath2D::clearSprites()
	{
		for (auto& x : layers)
		{
			for (auto& y : x.items)
			{
				y.sprite = nullptr;
			}
		}
		CleanLayers();
	}
	int RenderPath2D::getSpriteOrder(asSprite* sprite)
	{
		for (auto& x : layers)
		{
			for (auto& y : x.items)
			{
				if (y.sprite == sprite)
				{
					return y.order;
				}
			}
		}
		return 0;
	}

	void RenderPath2D::addFont(as::asFont* font, const std::string& layer)
	{
		for (auto& x : layers)
		{
			if (!x.name.compare(layer))
			{
				x.items.push_back(RenderItem2D());
				x.items.back().type = RenderItem2D::FONT;
				x.items.back().font = font;
			}
		}
		SortLayers();
	}
	void RenderPath2D::removeFont(as::asFont* font)
	{
		for (auto& x : layers)
		{
			for (auto& y : x.items)
			{
				if (y.font == font)
				{
					y.font = nullptr;
				}
			}
		}
		CleanLayers();
	}
	void RenderPath2D::clearFonts()
	{
		for (auto& x : layers)
		{
			for (auto& y : x.items)
			{
				y.font = nullptr;
			}
		}
		CleanLayers();
	}
	int RenderPath2D::getFontOrder(asFont* font)
	{
		for (auto& x : layers)
		{
			for (auto& y : x.items)
			{
				if (y.font == font)
				{
					return y.order;
				}
			}
		}
		return 0;
	}


	void RenderPath2D::addLayer(const std::string& name)
	{
		for (auto& x : layers)
		{
			if (!x.name.compare(name))
				return;
		}
		RenderLayer2D layer = RenderLayer2D(name);
		layer.order = (int)layers.size();
		layers.push_back(layer);
		layers.back().items.clear();
	}
	void RenderPath2D::setLayerOrder(const std::string& name, int order)
	{
		for (auto& x : layers)
		{
			if (!x.name.compare(name))
			{
				x.order = order;
				break;
			}
		}
		SortLayers();
	}
	void RenderPath2D::SetSpriteOrder(asSprite* sprite, int order)
	{
		for (auto& x : layers)
		{
			for (auto& y : x.items)
			{
				if (y.type == RenderItem2D::SPRITE && y.sprite == sprite)
				{
					y.order = order;
				}
			}
		}
		SortLayers();
	}
	void RenderPath2D::SetFontOrder(asFont* font, int order)
	{
		for (auto& x : layers)
		{
			for (auto& y : x.items)
			{
				if (y.type == RenderItem2D::FONT && y.font == font)
				{
					y.order = order;
				}
			}
		}
		SortLayers();
	}
	void RenderPath2D::SortLayers()
	{
		if (layers.empty())
		{
			return;
		}

		for (size_t i = 0; i < layers.size() - 1; ++i)
		{
			for (size_t j = i + 1; j < layers.size(); ++j)
			{
				if (layers[i].order > layers[j].order)
				{
					RenderLayer2D swap = layers[i];
					layers[i] = layers[j];
					layers[j] = swap;
				}
			}
		}
		for (auto& x : layers)
		{
			if (x.items.empty())
			{
				continue;
			}
			for (size_t i = 0; i < x.items.size() - 1; ++i)
			{
				for (size_t j = i + 1; j < x.items.size(); ++j)
				{
					if (x.items[i].order > x.items[j].order)
					{
						RenderItem2D swap = x.items[i];
						x.items[i] = x.items[j];
						x.items[j] = swap;
					}
				}
			}
		}
	}

	void RenderPath2D::CleanLayers()
	{
		for (auto& x : layers)
		{
			if (x.items.empty())
			{
				continue;
			}
			std::vector<RenderItem2D> itemsToRetain(0);
			for (auto& y : x.items)
			{
				if (y.sprite != nullptr || y.font != nullptr)
				{
					itemsToRetain.push_back(y);
				}
			}
			x.items.clear();
			x.items = itemsToRetain;
		}
	}
}


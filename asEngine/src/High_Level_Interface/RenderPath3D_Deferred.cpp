#include "aspch.h"
#include "RenderPath3D_Deferred.h"
#include "Graphics/asRenderer.h"
#include "Graphics/asImage.h"
#include "Helpers/asHelper.h"
#include "Graphics/asTextureHelper.h"
#include "Graphics/asSprite.h"
#include "Graphics/GPUMapping/ResourceMapping.h"
#include "Tools/asProfiler.h"
#include "System/asJobSystem.h"

namespace as
{
	using namespace asGraphics;
	void RenderPath3D_Deferred::ResizeBuffers()
	{
		RenderPath3D::ResizeBuffers();

		GraphicsDevice* device = asRenderer::GetDevice();

		FORMAT defaultTextureFormat = device->GetBackBufferFormat();

		{
			TextureDesc desc;
			desc.BindFlags = BIND_RENDER_TARGET | BIND_SHADER_RESOURCE;
			desc.Width = asRenderer::GetInternalResolution().x;
			desc.Height = asRenderer::GetInternalResolution().y;

			desc.Format = FORMAT_R8G8B8A8_UNORM;
			device->CreateTexture(&desc, nullptr, &rtGBuffer[0]);
			device->SetName(&rtGBuffer[0], "rtGBuffer[0]");

			desc.Format = FORMAT_R16G16B16A16_FLOAT;
			device->CreateTexture(&desc, nullptr, &rtGBuffer[1]);
			device->SetName(&rtGBuffer[1], "rtGBuffer[1]");

			desc.Format = FORMAT_R8G8B8A8_UNORM;
			device->CreateTexture(&desc, nullptr, &rtGBuffer[2]);
			device->SetName(&rtGBuffer[2], "rtGBuffer[2]");
		}
		{
			TextureDesc desc;
			desc.BindFlags = BIND_RENDER_TARGET | BIND_SHADER_RESOURCE | BIND_UNORDERED_ACCESS;
			desc.Format = FORMAT_R11G11B10_FLOAT;
			desc.Width = asRenderer::GetInternalResolution().x;
			desc.Height = asRenderer::GetInternalResolution().y;
			device->CreateTexture(&desc, nullptr, &rtDeferred);
			device->SetName(&rtDeferred, "rtDeferred");
		}
		{
			TextureDesc desc;
			desc.BindFlags = BIND_RENDER_TARGET | BIND_SHADER_RESOURCE | BIND_UNORDERED_ACCESS;
			desc.Format = FORMAT_R11G11B10_FLOAT;
			desc.Width = asRenderer::GetInternalResolution().x;
			desc.Height = asRenderer::GetInternalResolution().y;
			device->CreateTexture(&desc, nullptr, &lightbuffer_diffuse);
			device->SetName(&lightbuffer_diffuse, "lightbuffer_diffuse");
			device->CreateTexture(&desc, nullptr, &lightbuffer_specular);
			device->SetName(&lightbuffer_specular, "lightbuffer_specular");
		}
		{
			TextureDesc desc;
			desc.BindFlags = BIND_RENDER_TARGET | BIND_SHADER_RESOURCE;
			desc.Format = FORMAT_R11G11B10_FLOAT;
			desc.Width = asRenderer::GetInternalResolution().x;
			desc.Height = asRenderer::GetInternalResolution().y;
			device->CreateTexture(&desc, nullptr, &rtSSS[0]);
			device->SetName(&rtSSS[0], "rtSSS[0]");
			device->CreateTexture(&desc, nullptr, &rtSSS[1]);
			device->SetName(&rtSSS[1], "rtSSS[1]");
		}

		{
			RenderPassDesc desc;
			desc.numAttachments = 6;
			desc.attachments[0] = { RenderPassAttachment::RENDERTARGET,RenderPassAttachment::LOADOP_DONTCARE,&rtGBuffer[0],-1 };
			desc.attachments[1] = { RenderPassAttachment::RENDERTARGET,RenderPassAttachment::LOADOP_CLEAR,&rtGBuffer[1],-1 };
			desc.attachments[2] = { RenderPassAttachment::RENDERTARGET,RenderPassAttachment::LOADOP_DONTCARE,&rtGBuffer[2],-1 };
			desc.attachments[3] = { RenderPassAttachment::RENDERTARGET,RenderPassAttachment::LOADOP_DONTCARE,&lightbuffer_diffuse,-1 };
			desc.attachments[4] = { RenderPassAttachment::RENDERTARGET,RenderPassAttachment::LOADOP_DONTCARE,&lightbuffer_specular,-1 };
			desc.attachments[5] = { RenderPassAttachment::DEPTH_STENCIL,RenderPassAttachment::LOADOP_CLEAR,&depthBuffer,-1 };

			device->CreateRenderPass(&desc, &renderpass_gbuffer);
		}
		{
			RenderPassDesc desc;
			desc.numAttachments = 3;
			desc.attachments[0] = { RenderPassAttachment::RENDERTARGET,RenderPassAttachment::LOADOP_LOAD,&lightbuffer_diffuse,-1 };
			desc.attachments[1] = { RenderPassAttachment::RENDERTARGET,RenderPassAttachment::LOADOP_LOAD,&lightbuffer_specular,-1 };
			desc.attachments[2] = { RenderPassAttachment::DEPTH_STENCIL,RenderPassAttachment::LOADOP_LOAD,&depthBuffer,-1 };

			device->CreateRenderPass(&desc, &renderpass_lights);
		}
		{
			RenderPassDesc desc;
			desc.numAttachments = 2;
			desc.attachments[0] = { RenderPassAttachment::RENDERTARGET,RenderPassAttachment::LOADOP_LOAD,&rtGBuffer[0],-1 };
			desc.attachments[1] = { RenderPassAttachment::DEPTH_STENCIL,RenderPassAttachment::LOADOP_LOAD,&depthBuffer,-1 };

			device->CreateRenderPass(&desc, &renderpass_decals);
		}
		{
			RenderPassDesc desc;
			desc.numAttachments = 2;
			desc.attachments[0] = { RenderPassAttachment::RENDERTARGET,RenderPassAttachment::LOADOP_LOAD,&rtDeferred,-1 };
			desc.attachments[1] = { RenderPassAttachment::DEPTH_STENCIL,RenderPassAttachment::LOADOP_LOAD,&depthBuffer,-1 };

			device->CreateRenderPass(&desc, &renderpass_deferredcomposition);
		}
		{
			RenderPassDesc desc;
			desc.numAttachments = 2;
			desc.attachments[1] = { RenderPassAttachment::DEPTH_STENCIL,RenderPassAttachment::LOADOP_LOAD,&depthBuffer,-1 };

			desc.attachments[0] = { RenderPassAttachment::RENDERTARGET,RenderPassAttachment::LOADOP_LOAD,&lightbuffer_diffuse,-1 };
			device->CreateRenderPass(&desc, &renderpass_SSS[0]);

			desc.attachments[0] = { RenderPassAttachment::RENDERTARGET,RenderPassAttachment::LOADOP_LOAD,&rtSSS[0],-1 };
			device->CreateRenderPass(&desc, &renderpass_SSS[1]);

			desc.attachments[0] = { RenderPassAttachment::RENDERTARGET,RenderPassAttachment::LOADOP_LOAD,&rtSSS[1],-1 };
			device->CreateRenderPass(&desc, &renderpass_SSS[2]);
		}
		{
			RenderPassDesc desc;
			desc.numAttachments = 2;
			desc.attachments[0] = { RenderPassAttachment::RENDERTARGET,RenderPassAttachment::LOADOP_LOAD,&rtDeferred,-1 };
			desc.attachments[1] = { RenderPassAttachment::DEPTH_STENCIL,RenderPassAttachment::LOADOP_LOAD,&depthBuffer,-1,RenderPassAttachment::STOREOP_DONTCARE };

			device->CreateRenderPass(&desc, &renderpass_transparent);
		}
	}

	void RenderPath3D_Deferred::Render() const
	{
		GraphicsDevice* device = asRenderer::GetDevice();
		asJobSystem::context ctx;
		CommandList cmd;

		cmd = device->BeginCommandList();
		asJobSystem::Execute(ctx, [this, cmd] { RenderFrameSetUp(cmd); });
		cmd = device->BeginCommandList();
		asJobSystem::Execute(ctx, [this, cmd] { RenderShadows(cmd); });
		cmd = device->BeginCommandList();
		asJobSystem::Execute(ctx, [this, cmd] { RenderReflections(cmd); });

		// Main scene:
		cmd = device->BeginCommandList();
		asJobSystem::Execute(ctx, [this, device, cmd] {

			asRenderer::UpdateCameraCB(asRenderer::GetCamera(), cmd);

			device->Barrier(&GPUBarrier::Image(&depthBuffer, IMAGE_LAYOUT_DEPTHSTENCIL_READONLY, IMAGE_LAYOUT_DEPTHSTENCIL), 1, cmd);

			{
				auto range = asProfiler::BeginRangeGPU("Opaque Scene", cmd);

				device->RenderPassBegin(&renderpass_gbuffer, cmd);

				Viewport vp;
				vp.Width = (float)depthBuffer.GetDesc().Width;
				vp.Height = (float)depthBuffer.GetDesc().Height;
				device->BindViewports(1, &vp, cmd);

				device->BindResource(PS, getReflectionsEnabled() ? &rtReflection : asTextureHelper::getTransparent(), TEXSLOT_RENDERPATH_REFLECTION, cmd);
				asRenderer::DrawScene(asRenderer::GetCamera(), getTessellationEnabled(), cmd, RENDERPASS_DEFERRED, true, true);

				device->RenderPassEnd(cmd);

				asProfiler::EndRange(range); // Opaque Scene
			}

			{
				GPUBarrier barriers[] = {
					GPUBarrier::Image(&depthBuffer, IMAGE_LAYOUT_DEPTHSTENCIL, IMAGE_LAYOUT_COPY_SRC),
					GPUBarrier::Image(&depthBuffer_Copy, IMAGE_LAYOUT_SHADER_RESOURCE, IMAGE_LAYOUT_COPY_DST)
				};
				device->Barrier(barriers, arraysize(barriers), cmd);
			}

			device->CopyResource(&depthBuffer_Copy, &depthBuffer, cmd);

			{
				GPUBarrier barriers[] = {
					GPUBarrier::Image(&depthBuffer, IMAGE_LAYOUT_COPY_SRC, IMAGE_LAYOUT_DEPTHSTENCIL_READONLY),
					GPUBarrier::Image(&depthBuffer_Copy, IMAGE_LAYOUT_COPY_DST, IMAGE_LAYOUT_SHADER_RESOURCE)
				};
				device->Barrier(barriers, arraysize(barriers), cmd);
			}

			RenderLinearDepth(cmd);

			RenderSSAO(cmd);
			});

		cmd = device->BeginCommandList();
		asJobSystem::Execute(ctx, [this, device, cmd] {

			asRenderer::UpdateCameraCB(asRenderer::GetCamera(), cmd);
			asRenderer::BindCommonResources(cmd);

			RenderDecals(cmd);

			// Deferred lights:
			{
				device->RenderPassBegin(&renderpass_lights, cmd);

				Viewport vp;
				vp.Width = (float)depthBuffer.GetDesc().Width;
				vp.Height = (float)depthBuffer.GetDesc().Height;
				device->BindViewports(1, &vp, cmd);

				device->BindResource(PS, getSSAOEnabled() ? &rtSSAO[0] : asTextureHelper::getWhite(), TEXSLOT_RENDERPATH_SSAO, cmd);
				device->BindResource(PS, getSSREnabled() ? &rtSSR : asTextureHelper::getTransparent(), TEXSLOT_RENDERPATH_SSR, cmd);
				asRenderer::DrawDeferredLights(asRenderer::GetCamera(), depthBuffer_Copy, rtGBuffer[0], rtGBuffer[1], rtGBuffer[2], cmd);

				device->RenderPassEnd(cmd);
			}
			});


		cmd = device->BeginCommandList();
		asJobSystem::Execute(ctx, [this, device, cmd] {

			asRenderer::UpdateCameraCB(asRenderer::GetCamera(), cmd);
			asRenderer::BindCommonResources(cmd);

			RenderSSS(cmd);

			RenderDeferredComposition(cmd);

			RenderSSR(rtDeferred, rtGBuffer[1], cmd);

			DownsampleDepthBuffer(cmd);

			RenderLightShafts(cmd);

			RenderVolumetrics(cmd);

			RenderRefractionSource(rtDeferred, cmd);

			RenderTransparents(renderpass_transparent, RENDERPASS_FORWARD, cmd);

			RenderOutline(rtDeferred, cmd);

			RenderPostprocessChain(rtDeferred, rtGBuffer[1], cmd);

			});

		RenderPath2D::Render();

		asJobSystem::Wait(ctx);
	}

	void RenderPath3D_Deferred::RenderSSS(CommandList cmd) const
	{
		if (getSSSEnabled())
		{
			asRenderer::Postprocess_SSS(
				rtLinearDepth,
				rtGBuffer[0],
				renderpass_SSS[0],
				renderpass_SSS[1],
				renderpass_SSS[2],
				cmd
			);
		}
	}
	void RenderPath3D_Deferred::RenderDecals(CommandList cmd) const
	{
		GraphicsDevice* device = asRenderer::GetDevice();

		device->RenderPassBegin(&renderpass_decals, cmd);

		Viewport vp;
		vp.Width = (float)depthBuffer.GetDesc().Width;
		vp.Height = (float)depthBuffer.GetDesc().Height;
		device->BindViewports(1, &vp, cmd);

		asRenderer::DrawDeferredDecals(asRenderer::GetCamera(), depthBuffer_Copy, cmd);

		device->RenderPassEnd(cmd);
	}
	void RenderPath3D_Deferred::RenderDeferredComposition(CommandList cmd) const
	{
		GraphicsDevice* device = asRenderer::GetDevice();

		device->RenderPassBegin(&renderpass_deferredcomposition, cmd);

		Viewport vp;
		vp.Width = (float)depthBuffer.GetDesc().Width;
		vp.Height = (float)depthBuffer.GetDesc().Height;
		device->BindViewports(1, &vp, cmd);

		asRenderer::DeferredComposition(
			rtGBuffer[0],
			rtGBuffer[1],
			rtGBuffer[2],
			lightbuffer_diffuse,
			lightbuffer_specular,
			getSSAOEnabled() ? rtSSAO[0] : *asTextureHelper::getWhite(),
			rtLinearDepth,
			cmd
		);
		asRenderer::DrawSky(cmd);

		device->RenderPassEnd(cmd);
	}
}



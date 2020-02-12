#include "aspch.h"

#include "RenderPath3D_TiledDeferred.h"
#include "Graphics/asRenderer.h"
#include "Graphics/asImage.h"
#include "Helpers/asHelper.h"
#include "Graphics/asTextureHelper.h"
#include "Graphics/asSprite.h"
#include "Graphics/GPUMapping/ResourceMapping.h"
#include "Tools/asProfiler.h"
#include "Tools/asBackLog.h"
#include "System/asJobSystem.h"

namespace as
{
	using namespace asGraphics;

	void RenderPath3D_TiledDeferred::ResizeBuffers()
	{
		RenderPath3D_Deferred::ResizeBuffers();

		GraphicsDevice* device = asRenderer::GetDevice();

		// Workaround textures if R11G11B10 UAV loads are not supported by the GPU:
		if (!device->CheckCapability(GraphicsDevice::GRAPHICSDEVICE_CAPABILITY_UAV_LOAD_FORMAT_R11G11B10_FLOAT))
		{
			asBackLog::post("\nWARNING: GRAPHICSDEVICE_CAPABILITY_UAV_LOAD_FORMAT_R11G11B10_FLOAT not supported, Tiled deferred will be using workaround slow path!\n");
			AS_CORE_INFO("\nWARNING: GRAPHICSDEVICE_CAPABILITY_UAV_LOAD_FORMAT_R11G11B10_FLOAT not supported, Tiled deferred will be using workaround slow path!\n");
			TextureDesc desc;
			desc = lightbuffer_diffuse.GetDesc();
			desc.Format = FORMAT_R16G16B16A16_FLOAT;
			device->CreateTexture(&desc, nullptr, &lightbuffer_diffuse_noR11G11B10supportavailable);
			device->SetName(&lightbuffer_diffuse_noR11G11B10supportavailable, "lightbuffer_diffuse_noR11G11B10supportavailable");

			desc = lightbuffer_specular.GetDesc();
			desc.Format = FORMAT_R16G16B16A16_FLOAT;
			device->CreateTexture(&desc, nullptr, &lightbuffer_specular_noR11G11B10supportavailable);
			device->SetName(&lightbuffer_specular_noR11G11B10supportavailable, "lightbuffer_specular_noR11G11B10supportavailable");
		}
	}

	void RenderPath3D_TiledDeferred::Render() const
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

			device->BindResource(CS, getSSAOEnabled() ? &rtSSAO[0] : asTextureHelper::getWhite(), TEXSLOT_RENDERPATH_SSAO, cmd);
			device->BindResource(CS, getSSREnabled() ? &rtSSR : asTextureHelper::getTransparent(), TEXSLOT_RENDERPATH_SSR, cmd);


			if (device->CheckCapability(GraphicsDevice::GRAPHICSDEVICE_CAPABILITY_UAV_LOAD_FORMAT_R11G11B10_FLOAT))
			{
				asRenderer::ComputeTiledLightCulling(
					depthBuffer_Copy,
					cmd,
					&rtGBuffer[0],
					&rtGBuffer[1],
					&rtGBuffer[2],
					&lightbuffer_diffuse,
					&lightbuffer_specular
				);
			}
			else
			{
				// This workaround if R11G11B10_FLOAT can't be used with UAV loads copies into R16G16B16A16_FLOAT, does the tiled deferred then copies back:
				device->EventBegin("WARNING: GRAPHICSDEVICE_CAPABILITY_UAV_LOAD_FORMAT_R11G11B10_FLOAT not supported workaround!", cmd);

				asRenderer::CopyTexture2D(lightbuffer_diffuse_noR11G11B10supportavailable, 0, 0, 0, lightbuffer_diffuse, 0, cmd);
				asRenderer::CopyTexture2D(lightbuffer_specular_noR11G11B10supportavailable, 0, 0, 0, lightbuffer_specular, 0, cmd);

				asRenderer::ComputeTiledLightCulling(
					depthBuffer_Copy,
					cmd,
					&rtGBuffer[0],
					&rtGBuffer[1],
					&rtGBuffer[2],
					&lightbuffer_diffuse_noR11G11B10supportavailable,
					&lightbuffer_specular_noR11G11B10supportavailable
				);

				asRenderer::CopyTexture2D(lightbuffer_diffuse, 0, 0, 0, lightbuffer_diffuse_noR11G11B10supportavailable, 0, cmd);
				asRenderer::CopyTexture2D(lightbuffer_specular, 0, 0, 0, lightbuffer_specular_noR11G11B10supportavailable, 0, cmd);

				device->EventEnd(cmd);
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

			RenderTransparents(renderpass_transparent, RENDERPASS_TILEDFORWARD, cmd);

			RenderOutline(rtDeferred, cmd);

			RenderPostprocessChain(rtDeferred, rtGBuffer[1], cmd);

			});

		RenderPath2D::Render();

		asJobSystem::Wait(ctx);
	}
}


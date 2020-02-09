#include "aspch.h"

#include "RenderPath3D_TiledForward.h"
#include "Graphics/asRenderer.h"
#include "Graphics/asImage.h"
#include "Helpers/asHelper.h"
#include "Tools/asProfiler.h"
#include "System/asJobSystem.h"
#include "Graphics/GPUMapping/ResourceMapping.h"
#include "Graphics/asTextureHelper.h"

namespace as
{

	using namespace asGraphics;

	void RenderPath3D_TiledForward::Render() const
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

			// depth prepass
			{
				auto range = asProfiler::BeginRangeGPU("Z-Prepass", cmd);

				device->RenderPassBegin(&renderpass_depthprepass, cmd);

				Viewport vp;
				vp.Width = (float)depthBuffer.GetDesc().Width;
				vp.Height = (float)depthBuffer.GetDesc().Height;
				device->BindViewports(1, &vp, cmd);

				asRenderer::DrawScene(asRenderer::GetCamera(), getTessellationEnabled(), cmd, RENDERPASS_DEPTHONLY, true, true);

				device->RenderPassEnd(cmd);

				asProfiler::EndRange(range);
			}

			if (getMSAASampleCount() > 1)
			{
				device->Barrier(&GPUBarrier::Image(&depthBuffer, IMAGE_LAYOUT_DEPTHSTENCIL, IMAGE_LAYOUT_SHADER_RESOURCE), 1, cmd);
				asRenderer::ResolveMSAADepthBuffer(depthBuffer_Copy, depthBuffer, cmd);
				device->Barrier(&GPUBarrier::Image(&depthBuffer, IMAGE_LAYOUT_SHADER_RESOURCE, IMAGE_LAYOUT_DEPTHSTENCIL_READONLY), 1, cmd);
			}
			else
			{
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
			}

			RenderLinearDepth(cmd);

			RenderSSAO(cmd);
			});

		cmd = device->BeginCommandList();
		asJobSystem::Execute(ctx, [this, device, cmd] {

			asRenderer::ComputeTiledLightCulling(
				depthBuffer_Copy,
				cmd
			);

			device->UnbindResources(TEXSLOT_ONDEMAND0, 1, cmd);

			// Opaque scene:
			{
				auto range = asProfiler::BeginRangeGPU("Opaque Scene", cmd);

				device->RenderPassBegin(&renderpass_main, cmd);

				Viewport vp;
				vp.Width = (float)depthBuffer.GetDesc().Width;
				vp.Height = (float)depthBuffer.GetDesc().Height;
				device->BindViewports(1, &vp, cmd);

				device->BindResource(PS, getReflectionsEnabled() ? &rtReflection : asTextureHelper::getTransparent(), TEXSLOT_RENDERPATH_REFLECTION, cmd);
				device->BindResource(PS, getSSAOEnabled() ? &rtSSAO[0] : asTextureHelper::getWhite(), TEXSLOT_RENDERPATH_SSAO, cmd);
				device->BindResource(PS, getSSREnabled() ? &rtSSR : asTextureHelper::getTransparent(), TEXSLOT_RENDERPATH_SSR, cmd);
				asRenderer::DrawScene(asRenderer::GetCamera(), getTessellationEnabled(), cmd, RENDERPASS_TILEDFORWARD, true, true);
				asRenderer::DrawSky(cmd);

				device->RenderPassEnd(cmd);

				asProfiler::EndRange(range); // Opaque Scene
			}
			});

		cmd = device->BeginCommandList();
		asJobSystem::Execute(ctx, [this, device, cmd] {

			asRenderer::UpdateCameraCB(asRenderer::GetCamera(), cmd);
			asRenderer::BindCommonResources(cmd);

			if (getMSAASampleCount() > 1)
			{
				device->MSAAResolve(GetSceneRT_Read(0), &rtMain[0], cmd);
				device->MSAAResolve(GetSceneRT_Read(1), &rtMain[1], cmd);
			}

			RenderSSR(*GetSceneRT_Read(0), *GetSceneRT_Read(1), cmd);

			DownsampleDepthBuffer(cmd);

			RenderLightShafts(cmd);

			RenderVolumetrics(cmd);

			RenderRefractionSource(*GetSceneRT_Read(0), cmd);

			RenderTransparents(renderpass_transparent, RENDERPASS_TILEDFORWARD, cmd);

			if (getMSAASampleCount() > 1)
			{
				device->MSAAResolve(GetSceneRT_Read(0), &rtMain[0], cmd);
			}

			RenderOutline(*GetSceneRT_Read(0), cmd);

			RenderPostprocessChain(*GetSceneRT_Read(0), *GetSceneRT_Read(1), cmd);
			});

		RenderPath2D::Render();

		asJobSystem::Wait(ctx);
	}

}
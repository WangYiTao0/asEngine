#include "aspch.h"
#include "RenderPath3D_PathTracing.h"
#include "Graphics/asRenderer.h"
#include "Graphics/asImage.h"
#include "Helpers/asHelper.h"
#include "Graphics/asTextureHelper.h"
#include "Graphics/asSprite.h"
#include "Graphics/GPUMapping/ResourceMapping.h"
#include "Tools/asProfiler.h"
#include "System/asScene.h"


namespace as
{
	using namespace asGraphics;
	using namespace asScene;


	void RenderPath3D_PathTracing::ResizeBuffers()
	{
		RenderPath2D::ResizeBuffers(); // we don't need to use any buffers from RenderPath3D, so skip those

		GraphicsDevice* device = asRenderer::GetDevice();

		FORMAT defaultTextureFormat = device->GetBackBufferFormat();

		{
			TextureDesc desc;
			desc.BindFlags = BIND_UNORDERED_ACCESS | BIND_SHADER_RESOURCE | BIND_RENDER_TARGET;
			desc.Format = FORMAT_R32G32B32A32_FLOAT;
			desc.Width = asRenderer::GetInternalResolution().x;
			desc.Height = asRenderer::GetInternalResolution().y;
			device->CreateTexture(&desc, nullptr, &traceResult);
			device->SetName(&traceResult, "traceResult");
		}
		{
			TextureDesc desc;
			desc.BindFlags = BIND_SHADER_RESOURCE | BIND_UNORDERED_ACCESS;
			desc.Format = defaultTextureFormat;
			desc.Width = asRenderer::GetInternalResolution().x;
			desc.Height = asRenderer::GetInternalResolution().y;
			device->CreateTexture(&desc, nullptr, &rtPostprocess_LDR[0]);
			device->SetName(&rtPostprocess_LDR[0], "rtPostprocess_LDR[0]");
		}

		{
			RenderPassDesc desc;
			desc.numAttachments = 1;
			desc.attachments[0] = { RenderPassAttachment::RENDERTARGET,RenderPassAttachment::LOADOP_CLEAR,&traceResult,-1 };

			device->CreateRenderPass(&desc, &renderpass_debugbvh);
		}

		// also reset accumulation buffer state:
		sam = -1;
	}

	void RenderPath3D_PathTracing::Update(float dt)
	{
		const Scene& scene = asScene::GetScene();

		if (asRenderer::GetCamera().IsDirty())
		{
			asRenderer::GetCamera().SetDirty(false);
			sam = -1;
		}
		else
		{
			for (size_t i = 0; i < scene.transforms.GetCount(); ++i)
			{
				const TransformComponent& transform = scene.transforms[i];

				if (transform.IsDirty())
				{
					sam = -1;
					break;
				}
			}

			if (sam >= 0)
			{
				for (size_t i = 0; i < scene.materials.GetCount(); ++i)
				{
					const MaterialComponent& material = scene.materials[i];

					if (material.IsDirty())
					{
						sam = -1;
						break;
					}
				}
			}
		}
		sam++;

		RenderPath3D::Update(dt);
	}

	void RenderPath3D_PathTracing::Render() const
	{
		GraphicsDevice* device = asRenderer::GetDevice();
		asJobSystem::context ctx;
		CommandList cmd;

		// Setup:
		cmd = device->BeginCommandList();
		asJobSystem::Execute(ctx, [this, cmd] {

			asRenderer::UpdateRenderData(cmd);

			if (sam == 0)
			{
				asRenderer::BuildSceneBVH(cmd);
			}
			});

		// Main scene:
		cmd = device->BeginCommandList();
		asJobSystem::Execute(ctx, [this, device, cmd] {

			asRenderer::BindCommonResources(cmd);

			if (asRenderer::GetRaytraceDebugBVHVisualizerEnabled())
			{
				device->RenderPassBegin(&renderpass_debugbvh, cmd);

				Viewport vp;
				vp.Width = (float)traceResult.GetDesc().Width;
				vp.Height = (float)traceResult.GetDesc().Height;
				device->BindViewports(1, &vp, cmd);

				asRenderer::UpdateCameraCB(asRenderer::GetCamera(), cmd);
				asRenderer::RayTraceSceneBVH(cmd);

				device->RenderPassEnd(cmd);
			}
			else
			{
				auto range = asProfiler::BeginRangeGPU("Traced Scene", cmd);

				asRenderer::UpdateCameraCB(asRenderer::GetCamera(), cmd);

				asRenderer::RayBuffers* rayBuffers = asRenderer::GenerateScreenRayBuffers(asRenderer::GetCamera(), cmd);
				asRenderer::RayTraceScene(rayBuffers, &traceResult, sam, cmd);


				asProfiler::EndRange(range); // Traced Scene
			}

			asRenderer::Postprocess_Tonemap(
				traceResult,
				*asTextureHelper::getColor(asColor::Gray()),
				*asTextureHelper::getBlack(),
				rtPostprocess_LDR[0],
				cmd,
				getExposure()
			);
			});

		RenderPath2D::Render();

		asJobSystem::Wait(ctx);
	}

	void RenderPath3D_PathTracing::Compose(CommandList cmd) const
	{
		GraphicsDevice* device = asRenderer::GetDevice();

		device->EventBegin("RenderPath3D_PathTracing::Compose", cmd);

		asRenderer::BindCommonResources(cmd);

		asImageParams fx;
		fx.enableFullScreen();
		fx.blendFlag = BLENDMODE_OPAQUE;
		fx.quality = QUALITY_LINEAR;
		asImage::Draw(&rtPostprocess_LDR[0], fx, cmd);

		device->EventEnd(cmd);

		RenderPath2D::Compose(cmd);
	}

}

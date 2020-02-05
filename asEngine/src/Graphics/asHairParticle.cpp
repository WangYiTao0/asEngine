#include "aspch.h"
#include "asHairParticle.h"
#include "Graphics/asRenderer.h"
#include "Helpers/asResourceManager.h"
#include "Helpers\asMath.h"
#include "Helpers\asIntersect.h"
#include "Helpers\asRandom.h"
#include "Graphics\GPUMapping\ResourceMapping.h"
#include "Helpers\asArchive.h"
#include "Graphics\GPUMapping\ShaderInterop.h"
#include "asTextureHelper.h"
#include "System/asScene.h"
#include "Graphics\GPUMapping\ShaderInterop_HairParticle.h"
#include "Tools\asBackLog.h"

using namespace asGraphics;

namespace asScene
{
	static VertexShader vs;
	static PixelShader ps_alphatestonly;
	static PixelShader ps_deferred;
	static PixelShader ps_forward;
	static PixelShader ps_forward_transparent;
	static PixelShader ps_tiledforward;
	static PixelShader ps_tiledforward_transparent;
	static PixelShader ps_simplest;
	static ComputeShader cs_simulate;
	static DepthStencilState dss_default, dss_equal, dss_rejectopaque_keeptransparent;
	static RasterizerState rs, ncrs, wirers;
	static BlendState bs[2];
	static PipelineState PSO[RENDERPASS_COUNT][2];
	static PipelineState PSO_wire;

	void asHairParticle::UpdateCPU(const TransformComponent& transform, const MeshComponent& mesh, float dt)
	{
		world = transform.world;

		XMFLOAT3 _min = mesh.aabb.getMin();
		XMFLOAT3 _max = mesh.aabb.getMax();

		_max.x += length;
		_max.y += length;
		_max.z += length;

		_min.x -= length;
		_min.y -= length;
		_min.z -= length;

		aabb = AABB(_min, _max);
		aabb = aabb.transform(world);

		if (dt > 0)
		{
			_flags &= ~REGENERATE_FRAME;
			if (cb == nullptr || (strandCount * segmentCount) != particleBuffer->GetDesc().ByteWidth / sizeof(Patch))
			{
				_flags |= REGENERATE_FRAME;

				cb.reset(new GPUBuffer);
				particleBuffer.reset(new GPUBuffer);
				simulationBuffer.reset(new GPUBuffer);

				GPUBufferDesc bd;
				bd.Usage = USAGE_DEFAULT;
				bd.BindFlags = BIND_SHADER_RESOURCE | BIND_UNORDERED_ACCESS;
				bd.CPUAccessFlags = 0;
				bd.MiscFlags = RESOURCE_MISC_BUFFER_STRUCTURED;

				if (strandCount * segmentCount > 0)
				{
					bd.StructureByteStride = sizeof(Patch);
					bd.ByteWidth = bd.StructureByteStride * strandCount * segmentCount;
					asRenderer::GetDevice()->CreateBuffer(&bd, nullptr, particleBuffer.get());

					bd.StructureByteStride = sizeof(PatchSimulationData);
					bd.ByteWidth = bd.StructureByteStride * strandCount * segmentCount;
					asRenderer::GetDevice()->CreateBuffer(&bd, nullptr, simulationBuffer.get());
				}

				bd.Usage = USAGE_DEFAULT;
				bd.ByteWidth = sizeof(HairParticleCB);
				bd.BindFlags = BIND_CONSTANT_BUFFER;
				bd.CPUAccessFlags = 0;
				bd.MiscFlags = 0;
				asRenderer::GetDevice()->CreateBuffer(&bd, nullptr, cb.get());
			}
		}

	}
	void asHairParticle::UpdateGPU(const MeshComponent& mesh, const MaterialComponent& material, CommandList cmd) const
	{
		if (strandCount == 0 || particleBuffer == nullptr)
		{
			return;
		}

		GraphicsDevice* device = asRenderer::GetDevice();
		device->EventBegin("HairParticle - UpdateRenderData", cmd);

		device->BindComputeShader(&cs_simulate, cmd);

		HairParticleCB hcb;
		hcb.xWorld = world;
		hcb.xColor = material.baseColor;
		hcb.xHairRegenerate = (_flags & REGENERATE_FRAME) ? 1 : 0;
		hcb.xLength = length;
		hcb.xStiffness = stiffness;
		hcb.xHairRandomness = randomness;
		hcb.xHairStrandCount = strandCount;
		hcb.xHairSegmentCount = std::max(segmentCount, 1u);
		hcb.xHairParticleCount = hcb.xHairStrandCount * hcb.xHairSegmentCount;
		hcb.xHairRandomSeed = randomSeed;
		hcb.xHairViewDistance = viewDistance;
		hcb.xHairBaseMeshIndexCount = (uint)mesh.indices.size();
		hcb.xHairBaseMeshVertexPositionStride = sizeof(MeshComponent::Vertex_POS);
		// segmentCount will be loop in the shader, not a threadgroup so we don't need it here:
		hcb.xHairNumDispatchGroups = (uint)ceilf((float)strandCount / (float)THREADCOUNT_SIMULATEHAIR);
		device->UpdateBuffer(cb.get(), &hcb, cmd);

		device->BindConstantBuffer(CS, cb.get(), CB_GETBINDSLOT(HairParticleCB), cmd);

		GPUResource* uavs[] = {
			particleBuffer.get(),
			simulationBuffer.get()
		};
		device->BindUAVs(CS, uavs, 0, arraysize(uavs), cmd);

		GPUResource* res[] = {
			mesh.indexBuffer.get(),
			mesh.streamoutBuffer_POS != nullptr ? mesh.streamoutBuffer_POS.get() : mesh.vertexBuffer_POS.get(),
		};
		device->BindResources(CS, res, TEXSLOT_ONDEMAND0, arraysize(res), cmd);

		device->Dispatch(hcb.xHairNumDispatchGroups, 1, 1, cmd);

		device->UnbindUAVs(0, arraysize(uavs), cmd);
		device->UnbindResources(TEXSLOT_ONDEMAND0, arraysize(res), cmd);

		device->EventEnd(cmd);
	}

	void asHairParticle::Draw(const CameraComponent& camera, const MaterialComponent& material, RENDERPASS renderPass, bool transparent, CommandList cmd) const
	{
		if (strandCount == 0 || cb == nullptr)
		{
			return;
		}

		GraphicsDevice* device = asRenderer::GetDevice();
		device->EventBegin("HairParticle - Draw", cmd);

		device->BindStencilRef(STENCILREF_DEFAULT, cmd);

		if (asRenderer::IsWireRender())
		{
			if (transparent || renderPass == RENDERPASS_DEPTHONLY)
			{
				return;
			}
			device->BindPipelineState(&PSO_wire, cmd);
			device->BindResource(VS, asTextureHelper::getWhite(), TEXSLOT_ONDEMAND0, cmd);
		}
		else
		{
			device->BindPipelineState(&PSO[renderPass][transparent], cmd);

			const GPUResource* res[] = {
				material.GetBaseColorMap()
			};
			device->BindResources(PS, res, TEXSLOT_ONDEMAND0, arraysize(res), cmd);
			device->BindResources(VS, res, TEXSLOT_ONDEMAND0, arraysize(res), cmd);
		}

		device->BindConstantBuffer(VS, cb.get(), CB_GETBINDSLOT(HairParticleCB), cmd);

		device->BindResource(VS, particleBuffer.get(), 0, cmd);

		device->Draw(strandCount * 12 * std::max(segmentCount, 1u), 0, cmd);

		device->EventEnd(cmd);
	}


	void asHairParticle::Serialize(asArchive& archive, uint32_t seed)
	{
		if (archive.IsReadMode())
		{
			archive >> _flags;
			asECS::SerializeEntity(archive, meshID, seed);
			archive >> strandCount;
			archive >> segmentCount;
			archive >> randomSeed;
			archive >> length;
			archive >> stiffness;
			archive >> randomness;
			archive >> viewDistance;
		}
		else
		{
			archive << _flags;
			asECS::SerializeEntity(archive, meshID, seed);
			archive << strandCount;
			archive << segmentCount;
			archive << randomSeed;
			archive << length;
			archive << stiffness;
			archive << randomness;
			archive << viewDistance;
		}
	}


	void asHairParticle::LoadShaders()
	{
		std::string path = asRenderer::GetShaderPath();

		asRenderer::LoadVertexShader(vs, "hairparticleVS.cso");

		asRenderer::LoadPixelShader(ps_simplest, "hairparticlePS_simplest.cso");
		asRenderer::LoadPixelShader(ps_alphatestonly, "hairparticlePS_alphatestonly.cso");
		asRenderer::LoadPixelShader(ps_deferred, "hairparticlePS_deferred.cso");
		asRenderer::LoadPixelShader(ps_forward, "hairparticlePS_forward.cso");
		asRenderer::LoadPixelShader(ps_forward_transparent, "hairparticlePS_forward_transparent.cso");
		asRenderer::LoadPixelShader(ps_tiledforward, "hairparticlePS_tiledforward.cso");
		asRenderer::LoadPixelShader(ps_tiledforward_transparent, "hairparticlePS_tiledforward_transparent.cso");

		asRenderer::LoadComputeShader(cs_simulate, "hairparticle_simulateCS.cso");

		GraphicsDevice* device = asRenderer::GetDevice();

		for (int i = 0; i < RENDERPASS_COUNT; ++i)
		{
			if (i == RENDERPASS_DEPTHONLY || i == RENDERPASS_DEFERRED || i == RENDERPASS_FORWARD || i == RENDERPASS_TILEDFORWARD)
			{
				for (int j = 0; j < 2; ++j)
				{
					if ((i == RENDERPASS_DEPTHONLY || i == RENDERPASS_DEFERRED) && j == 1)
					{
						continue;
					}

					PipelineStateDesc desc;
					desc.vs = &vs;
					desc.bs = &bs[j];
					desc.rs = &ncrs;
					desc.dss = &dss_default;

					switch (i)
					{
					case RENDERPASS_DEPTHONLY:
						desc.ps = &ps_alphatestonly;
						break;
					case RENDERPASS_DEFERRED:
						desc.ps = &ps_deferred;
						break;
					case RENDERPASS_FORWARD:
						if (j == 0)
						{
							desc.ps = &ps_forward;
							desc.dss = &dss_equal;
						}
						else
						{
							desc.ps = &ps_forward_transparent;
						}
						break;
					case RENDERPASS_TILEDFORWARD:
						if (j == 0)
						{
							desc.ps = &ps_tiledforward;
							desc.dss = &dss_equal;
						}
						else
						{
							desc.ps = &ps_tiledforward_transparent;
						}
						break;
					}

					if (j == 1)
					{
						desc.dss = &dss_rejectopaque_keeptransparent; // transparent
					}

					device->CreatePipelineState(&desc, &PSO[i][j]);
				}
			}
		}

		{
			PipelineStateDesc desc;
			desc.vs = &vs;
			desc.ps = &ps_simplest;
			desc.bs = &bs[0];
			desc.rs = &wirers;
			desc.dss = &dss_default;
			device->CreatePipelineState(&desc, &PSO_wire);
		}


	}
	void asHairParticle::Initialize()
	{

		RasterizerStateDesc rsd;
		rsd.FillMode = FILL_SOLID;
		rsd.CullMode = CULL_BACK;
		rsd.FrontCounterClockwise = true;
		rsd.DepthBias = 0;
		rsd.DepthBiasClamp = 0;
		rsd.SlopeScaledDepthBias = 0;
		rsd.DepthClipEnable = true;
		rsd.MultisampleEnable = false;
		rsd.AntialiasedLineEnable = false;
		asRenderer::GetDevice()->CreateRasterizerState(&rsd, &rs);

		rsd.FillMode = FILL_SOLID;
		rsd.CullMode = CULL_NONE;
		rsd.FrontCounterClockwise = true;
		rsd.DepthBias = 0;
		rsd.DepthBiasClamp = 0;
		rsd.SlopeScaledDepthBias = 0;
		rsd.DepthClipEnable = true;
		rsd.MultisampleEnable = false;
		rsd.AntialiasedLineEnable = false;
		asRenderer::GetDevice()->CreateRasterizerState(&rsd, &ncrs);

		rsd.FillMode = FILL_WIREFRAME;
		rsd.CullMode = CULL_NONE;
		rsd.FrontCounterClockwise = true;
		rsd.DepthBias = 0;
		rsd.DepthBiasClamp = 0;
		rsd.SlopeScaledDepthBias = 0;
		rsd.DepthClipEnable = true;
		rsd.MultisampleEnable = false;
		rsd.AntialiasedLineEnable = false;
		asRenderer::GetDevice()->CreateRasterizerState(&rsd, &wirers);


		DepthStencilStateDesc dsd;
		dsd.DepthEnable = true;
		dsd.DepthWriteMask = DEPTH_WRITE_MASK_ALL;
		dsd.DepthFunc = COMPARISON_GREATER;

		dsd.StencilEnable = true;
		dsd.StencilReadMask = 0xFF;
		dsd.StencilWriteMask = 0xFF;
		dsd.FrontFace.StencilFunc = COMPARISON_ALWAYS;
		dsd.FrontFace.StencilPassOp = STENCIL_OP_REPLACE;
		dsd.FrontFace.StencilFailOp = STENCIL_OP_KEEP;
		dsd.FrontFace.StencilDepthFailOp = STENCIL_OP_KEEP;
		dsd.BackFace.StencilFunc = COMPARISON_ALWAYS;
		dsd.BackFace.StencilPassOp = STENCIL_OP_REPLACE;
		dsd.BackFace.StencilFailOp = STENCIL_OP_KEEP;
		dsd.BackFace.StencilDepthFailOp = STENCIL_OP_KEEP;
		asRenderer::GetDevice()->CreateDepthStencilState(&dsd, &dss_default);

		dsd.DepthWriteMask = DEPTH_WRITE_MASK_ZERO;
		dsd.DepthFunc = COMPARISON_EQUAL;
		asRenderer::GetDevice()->CreateDepthStencilState(&dsd, &dss_equal);
		dsd.DepthFunc = COMPARISON_GREATER;
		asRenderer::GetDevice()->CreateDepthStencilState(&dsd, &dss_rejectopaque_keeptransparent);


		BlendStateDesc bld;
		bld.RenderTarget[0].BlendEnable = false;
		bld.AlphaToCoverageEnable = false; // maybe for msaa
		asRenderer::GetDevice()->CreateBlendState(&bld, &bs[0]);

		bld.RenderTarget[0].SrcBlend = BLEND_SRC_ALPHA;
		bld.RenderTarget[0].DestBlend = BLEND_INV_SRC_ALPHA;
		bld.RenderTarget[0].BlendOp = BLEND_OP_ADD;
		bld.RenderTarget[0].SrcBlendAlpha = BLEND_ONE;
		bld.RenderTarget[0].DestBlendAlpha = BLEND_ONE;
		bld.RenderTarget[0].BlendOpAlpha = BLEND_OP_ADD;
		bld.RenderTarget[0].BlendEnable = true;
		bld.RenderTarget[0].RenderTargetWriteMask = COLOR_WRITE_ENABLE_ALL;
		bld.AlphaToCoverageEnable = false;
		bld.IndependentBlendEnable = false;
		asRenderer::GetDevice()->CreateBlendState(&bld, &bs[1]);

		LoadShaders();

		asBackLog::post("asHairParticle Initialized");
	}

}





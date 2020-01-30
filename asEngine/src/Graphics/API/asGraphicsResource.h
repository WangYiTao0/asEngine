#pragma once
#include "CommonInclude.h"
#include "asGraphicsDescriptors.h"

#include <memory>
#include <vector>

namespace asGraphics
{
	class GraphicsDevice;

	struct GraphicsDeviceChild
	{
		std::shared_ptr<GraphicsDevice> device;
		inline void Register(std::shared_ptr<GraphicsDevice> dev) { device = dev; }
		inline bool IsValid() const { return device != nullptr; }
	};

	struct ShaderByteCode
	{
		uint8_t* data = nullptr;
		size_t size = 0;
		~ShaderByteCode() { SAFE_DELETE_ARRAY(data); }
	};

	struct VertexShader : public GraphicsDeviceChild
	{
		~VertexShader();

		ShaderByteCode code;
		asCPUHandle resource = AS_NULL_HANDLE;
	};

	struct PixelShader : public GraphicsDeviceChild
	{
		~PixelShader();

		ShaderByteCode code;
		asCPUHandle resource = AS_NULL_HANDLE;
	};

	struct GeometryShader : public GraphicsDeviceChild
	{
		~GeometryShader();

		ShaderByteCode code;
		asCPUHandle resource = AS_NULL_HANDLE;
	};

	struct HullShader : public GraphicsDeviceChild
	{
		~HullShader();

		ShaderByteCode code;
		asCPUHandle resource = AS_NULL_HANDLE;
	};

	struct DomainShader : public GraphicsDeviceChild
	{
		~DomainShader();

		ShaderByteCode code;
		asCPUHandle resource = AS_NULL_HANDLE;
	};

	struct ComputeShader : public GraphicsDeviceChild
	{
		~ComputeShader();

		ShaderByteCode code;
		asCPUHandle resource = AS_NULL_HANDLE;
	};

	struct Sampler : public GraphicsDeviceChild
	{
		asCPUHandle resource = AS_NULL_HANDLE;
		SamplerDesc desc;

		~Sampler();

		const SamplerDesc& GetDesc() const { return desc; }
	};

	struct GPUResource : public GraphicsDeviceChild
	{
		enum class GPU_RESOURCE_TYPE
		{
			BUFFER,
			TEXTURE,
			UNKNOWN_TYPE,
		} type = GPU_RESOURCE_TYPE::UNKNOWN_TYPE;
		inline bool IsTexture() const { return type == GPU_RESOURCE_TYPE::TEXTURE; }
		inline bool IsBuffer() const { return type == GPU_RESOURCE_TYPE::BUFFER; }

		asCPUHandle SRV = AS_NULL_HANDLE;
		std::vector<asCPUHandle> subresourceSRVs;

		asCPUHandle UAV = AS_NULL_HANDLE;
		std::vector<asCPUHandle> subresourceUAVs;

		asCPUHandle resource = AS_NULL_HANDLE;

		virtual ~GPUResource();
	};

	struct GPUBuffer : public GPUResource
	{
		asCPUHandle CBV = AS_NULL_HANDLE;
		GPUBufferDesc desc;

		virtual ~GPUBuffer();

		const GPUBufferDesc& GetDesc() const { return desc; }
	};

	struct VertexLayout : public GraphicsDeviceChild
	{
		asCPUHandle	resource = AS_NULL_HANDLE;

		std::vector<VertexLayoutDesc> desc;

		~VertexLayout();
	};

	struct BlendState : public GraphicsDeviceChild
	{
		asCPUHandle resource = AS_NULL_HANDLE;
		BlendStateDesc desc;

		~BlendState();

		const BlendStateDesc& GetDesc() const { return desc; }
	};

	struct DepthStencilState : public GraphicsDeviceChild
	{
		asCPUHandle resource = AS_NULL_HANDLE;
		DepthStencilStateDesc desc;

		~DepthStencilState();

		const DepthStencilStateDesc& GetDesc() const { return desc; }
	};

	struct RasterizerState : public GraphicsDeviceChild
	{
		asCPUHandle resource = AS_NULL_HANDLE;
		RasterizerStateDesc desc;

		~RasterizerState();

		const RasterizerStateDesc& GetDesc() const { return desc; }
	};

	struct Texture : public GPUResource
	{
		TextureDesc	desc;
		asCPUHandle	RTV = AS_NULL_HANDLE;
		std::vector<asCPUHandle> subresourceRTVs;
		asCPUHandle	DSV = AS_NULL_HANDLE;
		std::vector<asCPUHandle> subresourceDSVs;

		~Texture();

		const TextureDesc& GetDesc() const { return desc; }
	};


	struct GPUQuery : public GraphicsDeviceChild
	{
		asCPUHandle	resource = AS_NULL_HANDLE;
		GPUQueryDesc desc;

		~GPUQuery();

		const GPUQueryDesc& GetDesc() const { return desc; }
	};


	struct PipelineState : public GraphicsDeviceChild
	{
		size_t hash = 0;
		PipelineStateDesc desc;

		const PipelineStateDesc& GetDesc() const { return desc; }

		~PipelineState();
	};


	struct RenderPass : public GraphicsDeviceChild
	{
		size_t hash = 0;
		asCPUHandle	framebuffer = AS_NULL_HANDLE;
		asCPUHandle	renderpass = AS_NULL_HANDLE;
		RenderPassDesc desc;

		const RenderPassDesc& GetDesc() const { return desc; }

		~RenderPass();
	};
}
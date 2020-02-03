#pragma once
#include "CommonInclude.h"
#include "asGraphicsDevice.h"
#include "System\asPlatform.h"
#include "Helpers\asContainers.h"

#include <d3d11_3.h>
#include <dxgi1_3.h>

#include <atomic>

#include <wrl\client.h>

namespace asGraphics
{
	class GraphicsDevice_DX11 : public GraphicsDevice
	{
	private:
		Microsoft::WRL::ComPtr<ID3D11Device> device = nullptr;
		D3D_DRIVER_TYPE				driverType;
		D3D_FEATURE_LEVEL			featureLevel;
		Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain = nullptr;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> renderTargetView = nullptr;
		Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer = nullptr;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> immediateContext = nullptr;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> deviceContexts[COMMANDLIST_COUNT] = {};
		Microsoft::WRL::ComPtr<ID3D11CommandList> commandLists[COMMANDLIST_COUNT] = {};
		Microsoft::WRL::ComPtr<ID3DUserDefinedAnnotation> userDefinedAnnotations[COMMANDLIST_COUNT] = {};
	
		uint32_t	stencilRef[COMMANDLIST_COUNT];
		XMFLOAT4	blendFactor[COMMANDLIST_COUNT];

		Microsoft::WRL::ComPtr<ID3D11VertexShader> prev_vs[COMMANDLIST_COUNT] = {};
		Microsoft::WRL::ComPtr<ID3D11PixelShader> prev_ps[COMMANDLIST_COUNT] = {};
		Microsoft::WRL::ComPtr<ID3D11HullShader> prev_hs[COMMANDLIST_COUNT] = {};
		Microsoft::WRL::ComPtr<ID3D11DomainShader> prev_ds[COMMANDLIST_COUNT] = {};
		Microsoft::WRL::ComPtr<ID3D11GeometryShader> prev_gs[COMMANDLIST_COUNT] = {};
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> prev_cs[COMMANDLIST_COUNT] = {};
		XMFLOAT4 prev_blendfactor[COMMANDLIST_COUNT] = {};
		uint32_t prev_samplemask[COMMANDLIST_COUNT] = {};
		Microsoft::WRL::ComPtr <ID3D11BlendState> prev_bs[COMMANDLIST_COUNT] = {};
		Microsoft::WRL::ComPtr <ID3D11RasterizerState> prev_rs[COMMANDLIST_COUNT] = {};
		uint32_t prev_stencilRef[COMMANDLIST_COUNT] = {};
		Microsoft::WRL::ComPtr <ID3D11DepthStencilState> prev_dss[COMMANDLIST_COUNT] = {};
		Microsoft::WRL::ComPtr <ID3D11InputLayout> prev_il[COMMANDLIST_COUNT] = {};
		PRIMITIVETOPOLOGY prev_pt[COMMANDLIST_COUNT] = {};

		ID3D11UnorderedAccessView* raster_uavs[COMMANDLIST_COUNT][8] = {};
		uint8_t raster_uavs_slot[COMMANDLIST_COUNT] = {};
		uint8_t raster_uavs_count[COMMANDLIST_COUNT] = {};
		void validate_raster_uavs(CommandList cmd);

		struct GPUAllocator
		{
			GPUBuffer buffer;
			size_t byteOffset = 0;
			uint64_t residentFrame = 0;
			bool dirty = false;
		};
		
		GPUAllocator frame_allocators[COMMANDLIST_COUNT];

		void commit_allocations(CommandList cmd);

		void CreateBackBufferResources();

		std::atomic<uint8_t> commandlist_count{ 0 };
		wiContainers::ThreadSafeRingBuffer<CommandList, COMMANDLIST_COUNT> free_commandlists;
		wiContainers::ThreadSafeRingBuffer<CommandList, COMMANDLIST_COUNT> active_commandlists;

	public:
		GraphicsDevice_DX11(asPlatform::window_type window, bool fullscreen = false, bool debuglayer = false);
		virtual ~GraphicsDevice_DX11();

		virtual bool CreateBuffer(const GPUBufferDesc* pDesc, const SubresourceData* pInitialData, GPUBuffer* pBuffer) override;
		virtual bool CreateTexture(const TextureDesc* pDesc, const SubresourceData* pInitialData, Texture* pTexture) override;
		virtual bool CreateInputLayout(const VertexLayoutDesc* pInputElementDescs, uint32_t NumElements, const ShaderByteCode* shaderCode, VertexLayout* pInputLayout) override;
		virtual bool CreateVertexShader(const void* pShaderBytecode, size_t BytecodeLength, VertexShader* pVertexShader) override;
		virtual bool CreatePixelShader(const void* pShaderBytecode, size_t BytecodeLength, PixelShader* pPixelShader) override;
		virtual bool CreateGeometryShader(const void* pShaderBytecode, size_t BytecodeLength, GeometryShader* pGeometryShader) override;
		virtual bool CreateHullShader(const void* pShaderBytecode, size_t BytecodeLength, HullShader* pHullShader) override;
		virtual bool CreateDomainShader(const void* pShaderBytecode, size_t BytecodeLength, DomainShader* pDomainShader) override;
		virtual bool CreateComputeShader(const void* pShaderBytecode, size_t BytecodeLength, ComputeShader* pComputeShader) override;
		virtual bool CreateBlendState(const BlendStateDesc* pBlendStateDesc, BlendState* pBlendState) override;
		virtual bool CreateDepthStencilState(const DepthStencilStateDesc* pDepthStencilStateDesc, DepthStencilState* pDepthStencilState) override;
		virtual bool CreateRasterizerState(const RasterizerStateDesc* pRasterizerStateDesc, RasterizerState* pRasterizerState) override;
		virtual bool CreateSamplerState(const SamplerDesc* pSamplerDesc, Sampler* pSamplerState) override;
		virtual bool CreateQuery(const GPUQueryDesc* pDesc, GPUQuery* pQuery) override;
		virtual bool CreatePipelineState(const PipelineStateDesc* pDesc, PipelineState* pso) override;
		virtual bool CreateRenderPass(const RenderPassDesc* pDesc, RenderPass* renderpass) override;

		virtual int CreateSubresource(Texture* texture, SUBRESOURCE_TYPE type, uint32_t firstSlice, uint32_t sliceCount, uint32_t firstMip, uint32_t mipCount) override;

		virtual void DestroyResource(GPUResource* pResource) override;
		virtual void DestroyBuffer(GPUBuffer* pBuffer) override;
		virtual void DestroyTexture(Texture* pTexture) override;
		virtual void DestroyInputLayout(VertexLayout* pInputLayout) override;
		virtual void DestroyVertexShader(VertexShader* pVertexShader) override;
		virtual void DestroyPixelShader(PixelShader* pPixelShader) override;
		virtual void DestroyGeometryShader(GeometryShader* pGeometryShader) override;
		virtual void DestroyHullShader(HullShader* pHullShader) override;
		virtual void DestroyDomainShader(DomainShader* pDomainShader) override;
		virtual void DestroyComputeShader(ComputeShader* pComputeShader) override;
		virtual void DestroyBlendState(BlendState* pBlendState) override;
		virtual void DestroyDepthStencilState(DepthStencilState* pDepthStencilState) override;
		virtual void DestroyRasterizerState(RasterizerState* pRasterizerState) override;
		virtual void DestroySamplerState(Sampler* pSamplerState) override;
		virtual void DestroyQuery(GPUQuery* pQuery) override;
		virtual void DestroyPipelineState(PipelineState* pso) override;
		virtual void DestroyRenderPass(RenderPass* renderpass) override;

		virtual bool DownloadResource(const GPUResource* resourceToDownload, const GPUResource* resourceDest, void* dataDest) override;

		virtual void SetName(GPUResource* pResource, const std::string& name) override;

		virtual void PresentBegin(CommandList cmd) override;
		virtual void PresentEnd(CommandList cmd) override;

		virtual void WaitForGPU() override;

		virtual CommandList BeginCommandList() override;

		virtual void SetResolution(int width, int height) override;

		virtual Texture GetBackBuffer() override;

		///////////////Thread-sensitive////////////////////////

		virtual void RenderPassBegin(const RenderPass* renderpass, CommandList cmd) override;
		virtual void RenderPassEnd(CommandList cmd) override;
		virtual void BindScissorRects(uint32_t numRects, const Rect* rects, CommandList cmd) override;
		virtual void BindViewports(uint32_t NumViewports, const Viewport* pViewports, CommandList cmd) override;
		virtual void BindResource(SHADERSTAGE stage, const GPUResource* resource, uint32_t slot, CommandList cmd, int subresource = -1) override;
		virtual void BindResources(SHADERSTAGE stage, const GPUResource* const* resources, uint32_t slot, uint32_t count, CommandList cmd) override;
		virtual void BindUAV(SHADERSTAGE stage, const GPUResource* resource, uint32_t slot, CommandList cmd, int subresource = -1) override;
		virtual void BindUAVs(SHADERSTAGE stage, const GPUResource* const* resources, uint32_t slot, uint32_t count, CommandList cmd) override;
		virtual void UnbindResources(uint32_t slot, uint32_t num, CommandList cmd) override;
		virtual void UnbindUAVs(uint32_t slot, uint32_t num, CommandList cmd) override;
		virtual void BindSampler(SHADERSTAGE stage, const Sampler* sampler, uint32_t slot, CommandList cmd) override;
		virtual void BindConstantBuffer(SHADERSTAGE stage, const GPUBuffer* buffer, uint32_t slot, CommandList cmd) override;
		virtual void BindVertexBuffers(const GPUBuffer* const* vertexBuffers, uint32_t slot, uint32_t count, const uint32_t* strides, const uint32_t* offsets, CommandList cmd) override;
		virtual void BindIndexBuffer(const GPUBuffer* indexBuffer, const INDEXBUFFER_FORMAT format, uint32_t offset, CommandList cmd) override;
		virtual void BindStencilRef(uint32_t value, CommandList cmd) override;
		virtual void BindBlendFactor(float r, float g, float b, float a, CommandList cmd) override;
		virtual void BindPipelineState(const PipelineState* pso, CommandList cmd) override;
		virtual void BindComputeShader(const ComputeShader* cs, CommandList cmd) override;
		virtual void Draw(uint32_t vertexCount, uint32_t startVertexLocation, CommandList cmd) override;
		virtual void DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation, uint32_t baseVertexLocation, CommandList cmd) override;
		virtual void DrawInstanced(uint32_t vertexCount, uint32_t instanceCount, uint32_t startVertexLocation, uint32_t startInstanceLocation, CommandList cmd) override;
		virtual void DrawIndexedInstanced(uint32_t indexCount, uint32_t instanceCount, uint32_t startIndexLocation, uint32_t baseVertexLocation, uint32_t startInstanceLocation, CommandList cmd) override;
		virtual void DrawInstancedIndirect(const GPUBuffer* args, uint32_t args_offset, CommandList cmd) override;
		virtual void DrawIndexedInstancedIndirect(const GPUBuffer* args, uint32_t args_offset, CommandList cmd) override;
		virtual void Dispatch(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ, CommandList cmd) override;
		virtual void DispatchIndirect(const GPUBuffer* args, uint32_t args_offset, CommandList cmd) override;
		virtual void CopyResource(const GPUResource* pDst, const GPUResource* pSrc, CommandList cmd) override;
		virtual void CopyTexture2D_Region(const Texture* pDst, uint32_t dstMip, uint32_t dstX, uint32_t dstY, const Texture* pSrc, uint32_t srcMip, CommandList cmd) override;
		virtual void MSAAResolve(const Texture* pDst, const Texture* pSrc, CommandList cmd) override;
		virtual void UpdateBuffer(const GPUBuffer* buffer, const void* data, CommandList cmd, int dataSize = -1) override;
		virtual void QueryBegin(const GPUQuery* query, CommandList cmd) override;
		virtual void QueryEnd(const GPUQuery* query, CommandList cmd) override;
		virtual bool QueryRead(const GPUQuery* query, GPUQueryResult* result) override;
		virtual void Barrier(const GPUBarrier* barriers, uint32_t numBarriers, CommandList cmd) override {}

		virtual GPUAllocation AllocateGPU(size_t dataSize, CommandList cmd) override;

		virtual void EventBegin(const std::string& name, CommandList cmd) override;
		virtual void EventEnd(CommandList cmd) override;
		virtual void SetMarker(const std::string& name, CommandList cmd) override;


	};
}

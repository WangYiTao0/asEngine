#pragma once
#include "CommonInclude.h"
#include "Graphics/API/asGraphicsDevice.h"
#include "System/asPlatform.h"
#include "Helpers/asSpinLock.h"
#include "Helpers/asContainers.h"
#include "Graphics/API/asGraphicsDevice_SharedInternals.h"

#include "Utility/D3D12MemAlloc.h"

#include <dxgi1_4.h>
#include <d3d12.h>

#include <unordered_map>
#include <deque>
#include <atomic>
#include <mutex>


namespace as
{
	namespace asGraphics
	{

		class GraphicsDevice_DX12 : public GraphicsDevice
		{
		private:
			ID3D12Device* device = nullptr;
			ID3D12CommandQueue* directQueue = nullptr;
			ID3D12Fence* frameFence = nullptr;
			HANDLE						frameFenceEvent;
			D3D12MA::Allocator* allocator = nullptr;

			ID3D12CommandQueue* copyQueue = nullptr;
			ID3D12CommandAllocator* copyAllocator = nullptr;
			ID3D12CommandList* copyCommandList = nullptr;
			ID3D12Fence* copyFence = nullptr;
			HANDLE						copyFenceEvent;
			UINT64						copyFenceValue;
			asSpinLock					copyQueueLock;

			ID3D12RootSignature* graphicsRootSig = nullptr;
			ID3D12RootSignature* computeRootSig = nullptr;

			ID3D12CommandSignature* dispatchIndirectCommandSignature = nullptr;
			ID3D12CommandSignature* drawInstancedIndirectCommandSignature = nullptr;
			ID3D12CommandSignature* drawIndexedInstancedIndirectCommandSignature = nullptr;

			ID3D12QueryHeap* querypool_timestamp = nullptr;
			ID3D12QueryHeap* querypool_occlusion = nullptr;
			static const size_t timestamp_query_count = 1024;
			static const size_t occlusion_query_count = 1024;
			asContainers::ThreadSafeRingBuffer<uint32_t, timestamp_query_count> free_timestampqueries;
			asContainers::ThreadSafeRingBuffer<uint32_t, occlusion_query_count> free_occlusionqueries;
			ID3D12Resource* querypool_timestamp_readback = nullptr;
			ID3D12Resource* querypool_occlusion_readback = nullptr;
			D3D12MA::Allocation* allocation_querypool_timestamp_readback = nullptr;
			D3D12MA::Allocation* allocation_querypool_occlusion_readback = nullptr;

			struct DescriptorAllocator
			{
				ID3D12DescriptorHeap* heap = nullptr;
				size_t					heap_begin;
				uint32_t				itemCount;
				uint32_t					maxCount;
				uint32_t					itemSize;
				bool* itemsAlive = nullptr;
				uint32_t				lastAlloc;
				asSpinLock				lock;

				DescriptorAllocator(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t maxCount);
				~DescriptorAllocator();

				size_t allocate();
				void clear();
				void free(asCPUHandle descriptorHandle);
			};
			DescriptorAllocator* RTAllocator = nullptr;
			DescriptorAllocator* DSAllocator = nullptr;
			DescriptorAllocator* ResourceAllocator = nullptr;
			DescriptorAllocator* SamplerAllocator = nullptr;

			ID3D12DescriptorHeap* null_resource_heap_CPU = nullptr;
			ID3D12DescriptorHeap* null_sampler_heap_CPU = nullptr;
			D3D12_CPU_DESCRIPTOR_HANDLE null_resource_heap_cpu_start = {};
			D3D12_CPU_DESCRIPTOR_HANDLE null_sampler_heap_cpu_start = {};
			uint32_t resource_descriptor_size = 0;
			uint32_t sampler_descriptor_size = 0;

			struct FrameResources
			{
				ID3D12Resource* backBuffer = nullptr;
				D3D12_CPU_DESCRIPTOR_HANDLE		backBufferRTV = {};
				ID3D12CommandAllocator* commandAllocators[COMMANDLIST_COUNT] = {};
				ID3D12CommandList* commandLists[COMMANDLIST_COUNT] = {};

				struct DescriptorTableFrameAllocator
				{
					GraphicsDevice_DX12* device = nullptr;
					struct DescriptorHeap
					{
						D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
						ID3D12DescriptorHeap* heap_GPU = nullptr;
						D3D12_CPU_DESCRIPTOR_HANDLE start_cpu = {};
						D3D12_GPU_DESCRIPTOR_HANDLE start_gpu = {};
						uint32_t ringOffset = 0;
					};
					std::vector<DescriptorHeap> heaps_resource;
					std::vector<DescriptorHeap> heaps_sampler;
					size_t currentheap_resource = 0;
					size_t currentheap_sampler = 0;
					bool heaps_bound = false;

					struct Table
					{
						const GPUBuffer* CBV[GPU_RESOURCE_HEAP_CBV_COUNT];
						const GPUResource* SRV[GPU_RESOURCE_HEAP_SRV_COUNT];
						int SRV_index[GPU_RESOURCE_HEAP_SRV_COUNT];
						const GPUResource* UAV[GPU_RESOURCE_HEAP_UAV_COUNT];
						int UAV_index[GPU_RESOURCE_HEAP_UAV_COUNT];
						const Sampler* SAM[GPU_SAMPLER_HEAP_COUNT];

						bool dirty_resources;
						bool dirty_samplers;

						void reset()
						{
							memset(CBV, 0, sizeof(CBV));
							memset(SRV, 0, sizeof(SRV));
							memset(SRV_index, -1, sizeof(SRV_index));
							memset(UAV, 0, sizeof(UAV));
							memset(UAV_index, -1, sizeof(UAV_index));
							memset(SAM, 0, sizeof(SAM));
							dirty_resources = true;
							dirty_samplers = true;
						}

					} tables[SHADERSTAGE_COUNT];

					DescriptorTableFrameAllocator(GraphicsDevice_DX12* device);
					~DescriptorTableFrameAllocator();

					void reset();
					void validate(CommandList cmd);
					void create_or_bind_heaps_on_demand(CommandList cmd);
				};
				DescriptorTableFrameAllocator* descriptors[COMMANDLIST_COUNT] = {};

				struct ResourceFrameAllocator
				{
					GraphicsDevice_DX12* device = nullptr;
					GPUBuffer				buffer;
					D3D12MA::Allocation* allocation = nullptr;
					uint8_t* dataBegin = nullptr;
					uint8_t* dataCur = nullptr;
					uint8_t* dataEnd = nullptr;

					ResourceFrameAllocator(GraphicsDevice_DX12* device, size_t size);
					~ResourceFrameAllocator();

					uint8_t* allocate(size_t dataSize, size_t alignment);
					void clear();
					uint64_t calculateOffset(uint8_t* address);
				};
				ResourceFrameAllocator* resourceBuffer[COMMANDLIST_COUNT] = {};
			};
			FrameResources frames[BACKBUFFER_COUNT];
			FrameResources& GetFrameResources() { return frames[GetFrameCount() % BACKBUFFER_COUNT]; }
			inline ID3D12GraphicsCommandList4* GetDirectCommandList(CommandList cmd) { return static_cast<ID3D12GraphicsCommandList4*>(GetFrameResources().commandLists[cmd]); }

			struct DynamicResourceState
			{
				GPUAllocation allocation;
				bool binding[SHADERSTAGE_COUNT] = {};
			};
			std::unordered_map<const GPUBuffer*, DynamicResourceState> dynamic_constantbuffers[COMMANDLIST_COUNT];

			struct UploadBuffer
			{
				GraphicsDevice_DX12* device = nullptr;
				ID3D12Resource* resource = nullptr;
				D3D12MA::Allocation* allocation = nullptr;
				uint8_t* dataBegin = nullptr;
				uint8_t* dataCur = nullptr;
				uint8_t* dataEnd = nullptr;
				asSpinLock				lock;

				UploadBuffer(GraphicsDevice_DX12* device, size_t size);
				~UploadBuffer();

				uint8_t* allocate(size_t dataSize, size_t alignment);
				void clear();
				uint64_t calculateOffset(uint8_t* address);
			};
			UploadBuffer* bufferUploader = nullptr;
			UploadBuffer* textureUploader = nullptr;

			IDXGISwapChain3* swapChain = nullptr;

			PRIMITIVETOPOLOGY prev_pt[COMMANDLIST_COUNT] = {};

			std::unordered_map<size_t, ID3D12PipelineState*> pipelines_global;
			std::vector<std::pair<size_t, ID3D12PipelineState*>> pipelines_worker[COMMANDLIST_COUNT];
			size_t prev_pipeline_hash[COMMANDLIST_COUNT] = {};
			const RenderPass* active_renderpass[COMMANDLIST_COUNT] = {};

			std::atomic<uint8_t> commandlist_count{ 0 };
			asContainers::ThreadSafeRingBuffer<CommandList, COMMANDLIST_COUNT> free_commandlists;
			asContainers::ThreadSafeRingBuffer<CommandList, COMMANDLIST_COUNT> active_commandlists;

			struct DestroyItem
			{
				enum TYPE
				{
					RESOURCE,
					RESOURCEVIEW,
					RENDERTARGETVIEW,
					DEPTHSTENCILVIEW,
					SAMPLER,
					PIPELINE,
					QUERY_TIMESTAMP,
					QUERY_OCCLUSION,
				} type;
				uint64_t frame;
				asCPUHandle handle;
			};
			std::deque<DestroyItem> destroyer;
			std::mutex destroylocker;
			inline void DeferredDestroy(const DestroyItem& item)
			{
				destroylocker.lock();
				destroyer.push_back(item);
				destroylocker.unlock();
			}
			std::unordered_map<asCPUHandle, D3D12MA::Allocation*> mem_allocations;

		public:
			GraphicsDevice_DX12(asPlatform::window_type asndow, bool fullscreen = false, bool debuglayer = false);
			virtual ~GraphicsDevice_DX12();

			bool CreateBuffer(const GPUBufferDesc* pDesc, const SubresourceData* pInitialData, GPUBuffer* pBuffer) override;
			bool CreateTexture(const TextureDesc* pDesc, const SubresourceData* pInitialData, Texture* pTexture) override;
			bool CreateInputLayout(const VertexLayoutDesc* pInputElementDescs, uint32_t NumElements, const ShaderByteCode* shaderCode, VertexLayout* pInputLayout) override;
			bool CreateShader(SHADERSTAGE stage, const void* pShaderBytecode, size_t BytecodeLength, Shader* pShader) override;
			bool CreateBlendState(const BlendStateDesc* pBlendStateDesc, BlendState* pBlendState) override;
			bool CreateDepthStencilState(const DepthStencilStateDesc* pDepthStencilStateDesc, DepthStencilState* pDepthStencilState) override;
			bool CreateRasterizerState(const RasterizerStateDesc* pRasterizerStateDesc, RasterizerState* pRasterizerState) override;
			bool CreateSamplerState(const SamplerDesc* pSamplerDesc, Sampler* pSamplerState) override;
			bool CreateQuery(const GPUQueryDesc* pDesc, GPUQuery* pQuery) override;
			bool CreatePipelineState(const PipelineStateDesc* pDesc, PipelineState* pso) override;
			bool CreateRenderPass(const RenderPassDesc* pDesc, RenderPass* renderpass) override;

			int CreateSubresource(Texture* texture, SUBRESOURCE_TYPE type, uint32_t firstSlice, uint32_t sliceCount, uint32_t firstMip, uint32_t mipCount) override;

			void DestroyResource(GPUResource* pResource) override;
			void DestroyBuffer(GPUBuffer* pBuffer) override;
			void DestroyTexture(Texture* pTexture) override;
			void DestroyInputLayout(VertexLayout* pInputLayout) override;
			void DestroyShader(Shader* pShader) override;
			void DestroyBlendState(BlendState* pBlendState) override;
			void DestroyDepthStencilState(DepthStencilState* pDepthStencilState) override;
			void DestroyRasterizerState(RasterizerState* pRasterizerState) override;
			void DestroySamplerState(Sampler* pSamplerState) override;
			void DestroyQuery(GPUQuery* pQuery) override;
			void DestroyPipelineState(PipelineState* pso) override;
			void DestroyRenderPass(RenderPass* renderpass) override;

			bool DownloadResource(const GPUResource* resourceToDownload, const GPUResource* resourceDest, void* dataDest) override;

			void SetName(GPUResource* pResource, const std::string& name) override;

			void PresentBegin(CommandList cmd) override;
			void PresentEnd(CommandList cmd) override;

			virtual CommandList BeginCommandList() override;

			void WaitForGPU() override;
			void ClearPipelineStateCache() override;

			void SetResolution(int asdth, int height) override;

			Texture GetBackBuffer() override;

			///////////////Thread-sensitive////////////////////////

			void RenderPassBegin(const RenderPass* renderpass, CommandList cmd) override;
			void RenderPassEnd(CommandList cmd) override;
			void BindScissorRects(uint32_t numRects, const Rect* rects, CommandList cmd) override;
			void BindViewports(uint32_t NumViewports, const Viewport* pViewports, CommandList cmd) override;
			void BindResource(SHADERSTAGE stage, const GPUResource* resource, uint32_t slot, CommandList cmd, int subresource = -1) override;
			void BindResources(SHADERSTAGE stage, const GPUResource* const* resources, uint32_t slot, uint32_t count, CommandList cmd) override;
			void BindUAV(SHADERSTAGE stage, const GPUResource* resource, uint32_t slot, CommandList cmd, int subresource = -1) override;
			void BindUAVs(SHADERSTAGE stage, const GPUResource* const* resources, uint32_t slot, uint32_t count, CommandList cmd) override;
			void UnbindResources(uint32_t slot, uint32_t num, CommandList cmd) override;
			void UnbindUAVs(uint32_t slot, uint32_t num, CommandList cmd) override;
			void BindSampler(SHADERSTAGE stage, const Sampler* sampler, uint32_t slot, CommandList cmd) override;
			void BindConstantBuffer(SHADERSTAGE stage, const GPUBuffer* buffer, uint32_t slot, CommandList cmd) override;
			void BindVertexBuffers(const GPUBuffer* const* vertexBuffers, uint32_t slot, uint32_t count, const uint32_t* strides, const uint32_t* offsets, CommandList cmd) override;
			void BindIndexBuffer(const GPUBuffer* indexBuffer, const INDEXBUFFER_FORMAT format, uint32_t offset, CommandList cmd) override;
			void BindStencilRef(uint32_t value, CommandList cmd) override;
			void BindBlendFactor(float r, float g, float b, float a, CommandList cmd) override;
			void BindPipelineState(const PipelineState* pso, CommandList cmd) override;
			void BindComputeShader(const Shader* cs, CommandList cmd) override;
			void Draw(uint32_t vertexCount, uint32_t startVertexLocation, CommandList cmd) override;
			void DrawIndexed(uint32_t indexCount, uint32_t startIndexLocation, uint32_t baseVertexLocation, CommandList cmd) override;
			void DrawInstanced(uint32_t vertexCount, uint32_t instanceCount, uint32_t startVertexLocation, uint32_t startInstanceLocation, CommandList cmd) override;
			void DrawIndexedInstanced(uint32_t indexCount, uint32_t instanceCount, uint32_t startIndexLocation, uint32_t baseVertexLocation, uint32_t startInstanceLocation, CommandList cmd) override;
			void DrawInstancedIndirect(const GPUBuffer* args, uint32_t args_offset, CommandList cmd) override;
			void DrawIndexedInstancedIndirect(const GPUBuffer* args, uint32_t args_offset, CommandList cmd) override;
			void Dispatch(uint32_t threadGroupCountX, uint32_t threadGroupCountY, uint32_t threadGroupCountZ, CommandList cmd) override;
			void DispatchIndirect(const GPUBuffer* args, uint32_t args_offset, CommandList cmd) override;
			void CopyResource(const GPUResource* pDst, const GPUResource* pSrc, CommandList cmd) override;
			void CopyTexture2D_Region(const Texture* pDst, uint32_t dstMip, uint32_t dstX, uint32_t dstY, const Texture* pSrc, uint32_t srcMip, CommandList cmd) override;
			void MSAAResolve(const Texture* pDst, const Texture* pSrc, CommandList cmd) override;
			void UpdateBuffer(const GPUBuffer* buffer, const void* data, CommandList cmd, int dataSize = -1) override;
			void QueryBegin(const GPUQuery* query, CommandList cmd) override;
			void QueryEnd(const GPUQuery* query, CommandList cmd) override;
			bool QueryRead(const GPUQuery* query, GPUQueryResult* result) override;
			void Barrier(const GPUBarrier* barriers, uint32_t numBarriers, CommandList cmd) override;

			GPUAllocation AllocateGPU(size_t dataSize, CommandList cmd) override;

			void EventBegin(const std::string& name, CommandList cmd) override;
			void EventEnd(CommandList cmd) override;
			void SetMarker(const std::string& name, CommandList cmd) override;

		};

	}
}

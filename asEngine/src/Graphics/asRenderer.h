#pragma once
#include "CommonInclude.h"
#include "asEnums.h"
#include "API/asGraphicsDevice.h"
#include "System\asScene_Dec1.h"
#include "System\asECS.h"

#include <memory>

struct RAY;
namespace as
{

	namespace asRenderer
	{
		inline uint32_t CombineStencilrefs(STENCILREF engineStencilRef, uint8_t userStencilRef)
		{
			return (userStencilRef << 4) | static_cast<uint8_t>(engineStencilRef);
		}

		const asGraphics::Sampler* GetSampler(int slot);
		const asGraphics::Shader* GetVertexShader(VSTYPES id);
		const asGraphics::Shader* GetHullShader(HSTYPES id);
		const asGraphics::Shader* GetDomainShader(DSTYPES id);
		const asGraphics::Shader* GetGeometryShader(GSTYPES id);
		const asGraphics::Shader* GetPixelShader(PSTYPES id);
		const asGraphics::Shader* GetComputeShader(CSTYPES id);
		const asGraphics::VertexLayout* GetVertexLayout(VLTYPES id);
		const asGraphics::RasterizerState* GetRasterizerState(RSTYPES id);
		const asGraphics::DepthStencilState* GetDepthStencilState(DSSTYPES id);
		const asGraphics::BlendState* GetBlendState(BSTYPES id);
		const asGraphics::GPUBuffer* GetConstantBuffer(CBTYPES id);
		const asGraphics::Texture* GetTexture(TEXTYPES id);

		void ModifySampler(const asGraphics::SamplerDesc& desc, int slot);


		void Initialize();

		// Clears the global scene and the associated renderable resources
		void ClearWorld();

		// Set the main graphics device globally:
		void SetDevice(std::shared_ptr<asGraphics::GraphicsDevice> newDevice);
		// Retrieve the main graphics device:
		asGraphics::GraphicsDevice* GetDevice();

		// Returns the shader loading directory
		const std::string& GetShaderPath();
		// Sets the shader loading directory
		void SetShaderPath(const std::string& path);
		// Reload shaders
		void ReloadShaders();

		bool LoadShader(asGraphics::SHADERSTAGE stage, asGraphics::Shader& shader, const std::string& filename);

		// Returns the main camera that is currently being used in rendering (and also for post processing)
		asScene::CameraComponent& GetCamera();
		// Returns the previous frame's camera that is currently being used in rendering to reproject
		asScene::CameraComponent& GetPrevCamera();
		// Returns the planar reflection camera that is currently being used in rendering
		asScene::CameraComponent& GetRefCamera();
		// Attach camera to entity for the current frame
		void AttachCamera(asECS::Entity entity);

		// Updates the main scene, performs frustum culling for main camera and other tasks that are only done once per frame. Specify layerMask to only include specific entities in the render frame.
		void UpdatePerFrameData(float dt, uint32_t layerMask = ~0);
		// Updates the GPU state according to the previously called UpatePerFrameData()
		void UpdateRenderData(asGraphics::CommandList cmd);

		// Binds all common constant buffers and samplers that may be used in all shaders
		void BindCommonResources(asGraphics::CommandList cmd);
		// Updates the per frame constant buffer (need to call at least once per frame)
		void UpdateFrameCB(asGraphics::CommandList cmd);
		// Updates the per camera constant buffer need to call for each different camera that is used when calling DrawScene() and the like
		void UpdateCameraCB(const asScene::CameraComponent& camera, asGraphics::CommandList cmd);
		// Set a global clipping plane state that is available to use in any shader (access as float4 g_xClipPlane)
		void SetClipPlane(const XMFLOAT4& clipPlane, asGraphics::CommandList cmd);
		// Set a global alpha reference value that can be used by any shaders to perform alpha-testing (access as float g_xAlphaRef)
		void SetAlphaRef(float alphaRef, asGraphics::CommandList cmd);
		// Resets the global shader alpha reference value to float g_xAlphaRef = 0.75f
		inline void ResetAlphaRef(asGraphics::CommandList cmd) { SetAlphaRef(0.75f, cmd); }

		// Draw skydome centered to camera.
		void DrawSky(asGraphics::CommandList cmd);
		// A black skydome will be draw with only the sun being visible on it
		void DrawSun(asGraphics::CommandList cmd);
		// Draw the world from a camera. You must call UpdateCameraCB() at least once in this frame prior to this
		void DrawScene(const asScene::CameraComponent& camera, bool tessellation, asGraphics::CommandList cmd, RENDERPASS renderPass, bool grass, bool occlusionCulling);
		// Draw the transparent world from a camera. You must call UpdateCameraCB() at least once in this frame prior to this
		void DrawScene_Transparent(const asScene::CameraComponent& camera, const asGraphics::Texture& lineardepth, RENDERPASS renderPass, asGraphics::CommandList cmd, bool grass, bool occlusionCulling);
		// Draw shadow maps for each visible light that has associated shadow maps
		void DrawShadowmaps(const asScene::CameraComponent& camera, asGraphics::CommandList cmd, uint32_t layerMask = ~0);
		// Draw debug world. You must also enable what parts to draw, eg. SetToDrawGridHelper, etc, see implementation for details what can be enabled.
		void DrawDebugWorld(const asScene::CameraComponent& camera, asGraphics::CommandList cmd);
		// Draw Soft offscreen particles. Linear depth should be already readable (see BindDepthTextures())
		void DrawSoftParticles(
			const asScene::CameraComponent& camera,
			const asGraphics::Texture& lineardepth,
			bool distortion,
			asGraphics::CommandList cmd
		);
		// Draw deferred lights. Gbuffer and depth textures should already be readable (see BindGBufferTextures(), BindDepthTextures())
		void DrawDeferredLights(
			const asScene::CameraComponent& camera,
			const asGraphics::Texture& depthbuffer,
			const asGraphics::Texture& gbuffer0,
			const asGraphics::Texture& gbuffer1,
			const asGraphics::Texture& gbuffer2,
			asGraphics::CommandList cmd
		);
		// Draw simple light visualizer geometries
		void DrawLightVisualizers(
			const asScene::CameraComponent& camera,
			asGraphics::CommandList cmd
		);
		// Draw volumetric light scattering effects
		void DrawVolumeLights(
			const asScene::CameraComponent& camera,
			const asGraphics::Texture& depthbuffer,
			asGraphics::CommandList cmd
		);
		// Draw Lens Flares for lights that have them enabled
		void DrawLensFlares(
			const asScene::CameraComponent& camera,
			const asGraphics::Texture& depthbuffer,
			asGraphics::CommandList cmd
		);
		// Draw deferred decals
		void DrawDeferredDecals(
			const asScene::CameraComponent& camera,
			const asGraphics::Texture& depthbuffer,
			asGraphics::CommandList cmd
		);
		// Call once per frame to re-render out of date environment probes
		void RefreshEnvProbes(asGraphics::CommandList cmd);
		// Call once per frame to re-render out of date impostors
		void RefreshImpostors(asGraphics::CommandList cmd);
		// Call once per frame to repack out of date decals in the atlas
		void RefreshDecalAtlas(asGraphics::CommandList cmd);
		// Call once per frame to repack out of date lightmaps in the atlas
		void RefreshLightmapAtlas(asGraphics::CommandList cmd);
		// Voxelize the scene into a voxel grid 3D texture
		void VoxelRadiance(asGraphics::CommandList cmd);
		// Compute light grid tiles for tiled rendering paths
		//	If you specify lightbuffers (diffuse, specular), then a tiled deferred lighting will be computed as well. Otherwise, only the light grid gets computed
		void ComputeTiledLightCulling(
			const asGraphics::Texture& depthbuffer,
			asGraphics::CommandList cmd,
			const asGraphics::Texture* gbuffer0 = nullptr,
			const asGraphics::Texture* gbuffer1 = nullptr,
			const asGraphics::Texture* gbuffer2 = nullptr,
			const asGraphics::Texture* lightbuffer_diffuse = nullptr,
			const asGraphics::Texture* lightbuffer_specular = nullptr
		);
		// Run a compute shader that will resolve a MSAA depth buffer to a single-sample texture
		void ResolveMSAADepthBuffer(const asGraphics::Texture& dst, const asGraphics::Texture& src, asGraphics::CommandList cmd);
		void DownsampleDepthBuffer(const asGraphics::Texture& src, asGraphics::CommandList cmd);
		// Compute the luminance for the source image and return the texture containing the luminance value in pixel [0,0]
		const asGraphics::Texture* ComputeLuminance(const asGraphics::Texture& sourceImage, asGraphics::CommandList cmd);

		void DeferredComposition(
			const asGraphics::Texture& gbuffer0,
			const asGraphics::Texture& gbuffer1,
			const asGraphics::Texture& gbuffer2,
			const asGraphics::Texture& lightmap_diffuse,
			const asGraphics::Texture& lightmap_specular,
			const asGraphics::Texture& ao,
			const asGraphics::Texture& lineardepth,
			asGraphics::CommandList cmd);


		void Postprocess_Blur_Gaussian(
			const asGraphics::Texture& input,
			const asGraphics::Texture& temp,
			const asGraphics::Texture& output,
			asGraphics::CommandList cmd,
			float amountX = 1.0f,
			float amountY = 1.0f,
			float mip = 0.0f
		);
		void Postprocess_Blur_Bilateral(
			const asGraphics::Texture& input,
			const asGraphics::Texture& lineardepth,
			const asGraphics::Texture& temp,
			const asGraphics::Texture& output,
			asGraphics::CommandList cmd,
			float amountX = 1.0f,
			float amountY = 1.0f,
			float depth_threshold = 1.0f,
			float mip = 0.0f
		);
		void Postprocess_SSAO(
			const asGraphics::Texture& depthbuffer,
			const asGraphics::Texture& lineardepth,
			const asGraphics::Texture& lineardepth_minmax,
			const asGraphics::Texture& temp,
			const asGraphics::Texture& output,
			asGraphics::CommandList cmd,
			float range = 1.0f,
			uint32_t samplecount = 16,
			float blur = 2.3f,
			float power = 2.0f
		);
		void Postprocess_SSR(
			const asGraphics::Texture& input,
			const asGraphics::Texture& depthbuffer,
			const asGraphics::Texture& lineardepth_minmax,
			const asGraphics::Texture& gbuffer1,
			const asGraphics::Texture& output,
			asGraphics::CommandList cmd
		);
		void Postprocess_SSS(
			const asGraphics::Texture& lineardepth,
			const asGraphics::Texture& gbuffer0,
			const asGraphics::RenderPass& input_output_lightbuffer_diffuse,
			const asGraphics::RenderPass& input_output_temp1,
			const asGraphics::RenderPass& input_output_temp2,
			asGraphics::CommandList cmd
		);
		void Postprocess_LightShafts(
			const asGraphics::Texture& input,
			const asGraphics::Texture& output,
			asGraphics::CommandList cmd,
			const XMFLOAT2& center
		);
		void Postprocess_DepthOfField(
			const asGraphics::Texture& input,
			const asGraphics::Texture& output,
			const asGraphics::Texture& lineardepth,
			const asGraphics::Texture& lineardepth_minmax,
			asGraphics::CommandList cmd,
			float focus = 10.0f,
			float scale = 1.0f,
			float aspect = 1.0f,
			float max_coc = 18.0f
		);
		void Postprocess_Outline(
			const asGraphics::Texture& input,
			const asGraphics::Texture& output,
			asGraphics::CommandList cmd,
			float threshold = 0.1f,
			float thickness = 1.0f,
			const XMFLOAT4& color = XMFLOAT4(0, 0, 0, 1)
		);
		void Postprocess_MotionBlur(
			const asGraphics::Texture& input,
			const asGraphics::Texture& velocity,
			const asGraphics::Texture& lineardepth,
			const asGraphics::Texture& output,
			asGraphics::CommandList cmd,
			float strength = 100.0f
		);
		void Postprocess_Bloom(
			const asGraphics::Texture& input,
			const asGraphics::Texture& bloom,
			const asGraphics::Texture& output,
			asGraphics::CommandList cmd,
			float threshold = 1.0f
		);
		void Postprocess_FXAA(
			const asGraphics::Texture& input,
			const asGraphics::Texture& output,
			asGraphics::CommandList cmd
		);
		void Postprocess_TemporalAA(
			const asGraphics::Texture& input_current,
			const asGraphics::Texture& input_history,
			const asGraphics::Texture& velocity,
			const asGraphics::Texture& lineardepth,
			const asGraphics::Texture& output,
			asGraphics::CommandList cmd
		);
		void Postprocess_Colorgrade(
			const asGraphics::Texture& input,
			const asGraphics::Texture& lookuptable,
			const asGraphics::Texture& output,
			asGraphics::CommandList cmd
		);
		void Postprocess_Lineardepth(
			const asGraphics::Texture& input,
			const asGraphics::Texture& output_fullres,
			const asGraphics::Texture& output_minmax,
			asGraphics::CommandList cmd
		);
		void Postprocess_Sharpen(
			const asGraphics::Texture& input,
			const asGraphics::Texture& output,
			asGraphics::CommandList cmd,
			float amount = 1.0f
		);
		void Postprocess_Tonemap(
			const asGraphics::Texture& input,
			const asGraphics::Texture& input_luminance,
			const asGraphics::Texture& input_distortion,
			const asGraphics::Texture& output,
			asGraphics::CommandList cmd,
			float exposure
		);
		void Postprocess_Chromatic_Aberration(
			const asGraphics::Texture& input,
			const asGraphics::Texture& output,
			asGraphics::CommandList cmd,
			float amount = 1.0f
		);

		// Build the scene BVH on GPU that can be used by ray traced rendering
		void BuildSceneBVH(asGraphics::CommandList cmd);

		struct RayBuffers
		{
			uint32_t rayCapacity = 0;
			asGraphics::GPUBuffer rayBuffer[2];
			asGraphics::GPUBuffer rayIndexBuffer[2];
			asGraphics::GPUBuffer rayCountBuffer[2];
			asGraphics::GPUBuffer raySortBuffer;
			void Create(asGraphics::GraphicsDevice* device, uint32_t newRayCapacity);
		};
		// Generate rays for every pixel of the internal resolution
		RayBuffers* GenerateScreenRayBuffers(const asScene::CameraComponent& camera, asGraphics::CommandList cmd);
		// Render the scene with ray tracing. You provide the ray buffer, where each ray maps to one pixel of the result testure
		void RayTraceScene(
			const RayBuffers* rayBuffers,
			const asGraphics::Texture* result,
			int accumulation_sample,
			asGraphics::CommandList cmd
		);
		// Render the scene BVH with ray tracing to the screen
		void RayTraceSceneBVH(asGraphics::CommandList cmd);

		// Render occluders against a depth buffer
		void OcclusionCulling_Render(asGraphics::CommandList cmd);
		// Read the occlusion culling results of the previous call to OcclusionCulling_Render. This must be done on the main thread!
		void OcclusionCulling_Read();
		// Issue end-of frame operations
		void EndFrame();


		enum MIPGENFILTER
		{
			MIPGENFILTER_POINT,
			MIPGENFILTER_LINEAR,
			MIPGENFILTER_GAUSSIAN,
			MIPGENFILTER_BICUBIC,
		};
		void GenerateMipChain(const asGraphics::Texture& texture, MIPGENFILTER filter, asGraphics::CommandList cmd, int arrayIndex = -1);

		enum BORDEREXPANDSTYLE
		{
			BORDEREXPAND_DISABLE,
			BORDEREXPAND_WRAP,
			BORDEREXPAND_CLAMP,
		};
		// Performs copy operation even between different texture formats
		//	Can also expand border region according to desired sampler func
		void CopyTexture2D(
			const asGraphics::Texture& dst, uint32_t DstMIP, uint32_t DstX, uint32_t DstY,
			const asGraphics::Texture& src, uint32_t SrcMIP,
			asGraphics::CommandList cmd,
			BORDEREXPANDSTYLE borderExpand = BORDEREXPAND_DISABLE);

		// Assign texture slots to out of date environemnt probes
		void ManageEnvProbes();
		// Invalidate out of date impostors
		void ManageImpostors();
		// New decals will be packed into a texture atlas
		void ManageDecalAtlas();
		// New lightmapped objects will be packed into global lightmap atlas
		void ManageLightmapAtlas();

		void PutWaterRipple(const std::string& image, const XMFLOAT3& pos);
		void ManageWaterRipples();
		void DrawWaterRipples(asGraphics::CommandList cmd);



		// Set any param to -1 if don't want to modify
		void SetShadowProps2D(int resolution, int count, int softShadowQuality);
		// Set any param to -1 if don't want to modify
		void SetShadowPropsCube(int resolution, int count);

		// Returns the resolution that is used for all spotlight and directional light shadow maps
		int GetShadowRes2D();
		// Returns the resolution that is used for all pointlight and area light shadow maps
		int GetShadowResCube();



		void SetResolutionScale(float value);
		float GetResolutionScale();
		void SetTransparentShadowsEnabled(float value);
		float GetTransparentShadowsEnabled();
		XMUINT2 GetInternalResolution();
		bool ResolutionChanged();
		void SetGamma(float value);
		float GetGamma();
		void SetWireRender(bool value);
		bool IsWireRender();
		void SetToDrawDebugBoneLines(bool param);
		bool GetToDrawDebugBoneLines();
		void SetToDrawDebugPartitionTree(bool param);
		bool GetToDrawDebugPartitionTree();
		bool GetToDrawDebugEnvProbes();
		void SetToDrawDebugEnvProbes(bool value);
		void SetToDrawDebugEmitters(bool param);
		bool GetToDrawDebugEmitters();
		void SetToDrawDebugForceFields(bool param);
		bool GetToDrawDebugForceFields();
		void SetToDrawDebugCameras(bool param);
		bool GetToDrawDebugCameras();
		bool GetToDrawGridHelper();
		void SetToDrawGridHelper(bool value);
		bool GetToDrawVoxelHelper();
		void SetToDrawVoxelHelper(bool value);
		void SetDebugLightCulling(bool enabled);
		bool GetDebugLightCulling();
		void SetAdvancedLightCulling(bool enabled);
		bool GetAdvancedLightCulling();
		void SetAlphaCompositionEnabled(bool enabled);
		bool GetAlphaCompositionEnabled();
		void SetOcclusionCullingEnabled(bool enabled);
		bool GetOcclusionCullingEnabled();
		void SetLDSSkinningEnabled(bool enabled);
		bool GetLDSSkinningEnabled();
		void SetTemporalAAEnabled(bool enabled);
		bool GetTemporalAAEnabled();
		void SetTemporalAADebugEnabled(bool enabled);
		bool GetTemporalAADebugEnabled();
		void SetFreezeCullingCameraEnabled(bool enabled);
		bool GetFreezeCullingCameraEnabled();
		void SetVoxelRadianceEnabled(bool enabled);
		bool GetVoxelRadianceEnabled();
		void SetVoxelRadianceSecondaryBounceEnabled(bool enabled);
		bool GetVoxelRadianceSecondaryBounceEnabled();
		void SetVoxelRadianceReflectionsEnabled(bool enabled);
		bool GetVoxelRadianceReflectionsEnabled();
		void SetVoxelRadianceVoxelSize(float value);
		float GetVoxelRadianceVoxelSize();
		void SetVoxelRadianceMaxDistance(float value);
		float GetVoxelRadianceMaxDistance();
		int GetVoxelRadianceResolution();
		void SetVoxelRadianceNumCones(int value);
		int GetVoxelRadianceNumCones();
		float GetVoxelRadianceRayStepSize();
		void SetVoxelRadianceRayStepSize(float value);
		void SetAdvancedRefractionsEnabled(bool value);
		bool GetAdvancedRefractionsEnabled();
		bool IsRequestedReflectionRendering();
		const XMFLOAT4& GetWaterPlane();
		void SetGameSpeed(float value);
		float GetGameSpeed();
		void OceanRegenerate(); // regeenrates ocean if it is already created
		void InvalidateBVH(); // invalidates scene bvh so if something wants to use it, it will recompute and validate it
		void SetRaytraceBounceCount(uint32_t bounces);
		uint32_t GetRaytraceBounceCount();
		void SetRaytraceDebugBVHVisualizerEnabled(bool value);
		bool GetRaytraceDebugBVHVisualizerEnabled();

		const asGraphics::Texture* GetGlobalLightmap();

		// Gets pick ray according to the current screen resolution and pointer coordinates. Can be used as input into RayIntersectWorld()
		RAY GetPickRay(long cursorX, long cursorY);


		// Add box to render in next frame. It will be rendered in DrawDebugWorld()
		void AddRenderableBox(const XMFLOAT4X4& boxMatrix, const XMFLOAT4& color = XMFLOAT4(1, 1, 1, 1));

		struct RenderableLine
		{
			XMFLOAT3 start = XMFLOAT3(0, 0, 0);
			XMFLOAT3 end = XMFLOAT3(0, 0, 0);
			XMFLOAT4 color = XMFLOAT4(1, 1, 1, 1);
		};
		// Add line to render in the next frame. It will be rendered in DrawDebugWorld()
		void AddRenderableLine(const RenderableLine& line);

		struct RenderablePoint
		{
			XMFLOAT3 position = XMFLOAT3(0, 0, 0);
			float size = 1.0f;
			XMFLOAT4 color = XMFLOAT4(1, 1, 1, 1);
		};
		void AddRenderablePoint(const RenderablePoint& point);

		// Add a texture that should be mipmapped whenever it is feasible to do so
		void AddDeferredMIPGen(const asGraphics::Texture* tex);

		struct CustomShader
		{
			std::string name;

			struct Pass
			{
				uint32_t renderTypeFlags = RENDERTYPE_TRANSPARENT;
				asGraphics::PipelineState* pso = nullptr;
			};
			Pass passes[RENDERPASS_COUNT] = {};
		};
		// Registers a custom shader that can be set to materials. 
		//	Returns the ID of the custom shader that can be used with MaterialComponent::SetCustomShaderID()
		int RegisterCustomShader(const CustomShader& customShader);
		const std::vector<CustomShader>& GetCustomShaders();

		// Helper utility to manage async GPU query readback from the CPU
		//	GPUQueryRing<latency> here latency specifies the ring size of queries and 
		//	directly correlates with the frame latency between Issue (GPU) <-> Read back (CPU)
		template<int latency>
		struct GPUQueryRing
		{
			asGraphics::GPUQuery queries[latency];
			int id = 0;
			bool active[latency] = {};

			// Creates a number of queries in the async ring
			void Create(asGraphics::GraphicsDevice* device, const asGraphics::GPUQueryDesc* desc)
			{
				for (int i = 0; i < latency; ++i)
				{
					device->CreateQuery(desc, &queries[i]);
					active[i] = false;
					id = 0;
				}
			}

			// Returns the current query suitable for GPU execution and marks it as active
			//	Use this with GraphicsDevice::QueryBegin() and GraphicsDevice::QueryEnd()
			inline asGraphics::GPUQuery* Get_GPU()
			{
				active[id] = true;
				return &queries[id];
			}

			// Returns the current query suitable for CPU readback and marks it as inactive
			//	It will return nullptr if none of the queries are suitable for readback yet
			//	Use this with GraphicsDevice::QueryRead(). Only call once per frame per QueryRing instance!
			inline asGraphics::GPUQuery* Get_CPU()
			{
				id = (id + 1) % latency;
				if (!active[id])
				{
					return nullptr;
				}
				active[id] = false;
				return &queries[id];
			}
		};

	};
}


#pragma once
#include "aspch.h"

#include "Graphics\asEnums.h"
#include "Helpers\asIntersect.h"
#include "Graphics\asEmittedParticle.h"
#include "Graphics\asHairParticle.h"
#include "Graphics\GPUMapping\ShaderInterop_Renderer.h"

#include "System/asJobSystem.h"
#include "Audio/asAudio.h"
#include "Graphics\asRenderer.h"
#include "Helpers\asResourceManager.h"

#include "System\asECS.h"
#include "System\asScene_Dec1.h"


namespace as
{
	class asArchive;

	namespace asScene
	{
		struct NameComponent
		{
			std::string name;

			inline void operator=(const std::string& str) { name = str; }
			inline void operator=(std::string&& str) { name = std::move(str); }
			inline bool operator==(const std::string& str) const { return name.compare(str) == 0; }

			void Serialize(asArchive& archive, uint32_t seed = 0);
		};

		struct LayerComponent
		{
			uint32_t layerMask = ~0;

			inline uint32_t GetLayerMask() const { return layerMask; }

			void Serialize(asArchive& archive, uint32_t seed = 0);
		};

		struct TransformComponent
		{
			enum FLAGS
			{
				EMPTY = 0,
				DIRTY = 1 << 0,
			};
			uint32_t _flags = DIRTY;

			XMFLOAT3 scale_local = XMFLOAT3(1, 1, 1);
			XMFLOAT4 rotation_local = XMFLOAT4(0, 0, 0, 1);
			XMFLOAT3 translation_local = XMFLOAT3(0, 0, 0);

			// Non-serialized attributes:
			XMFLOAT4X4 world = IDENTITYMATRIX;

			inline void SetDirty(bool value = true) { if (value) { _flags |= DIRTY; } else { _flags &= ~DIRTY; } }
			inline bool IsDirty() const { return _flags & DIRTY; }

			XMFLOAT3 GetPosition() const;
			XMFLOAT4 GetRotation() const;
			XMFLOAT3 GetScale() const;
			XMVECTOR GetPositionV() const;
			XMVECTOR GetRotationV() const;
			XMVECTOR GetScaleV() const;
			void UpdateTransform();
			void UpdateTransform_Parented(const TransformComponent& parent, const XMFLOAT4X4& inverseParentBindMatrix = IDENTITYMATRIX);
			void ApplyTransform();
			void ClearTransform();
			void Translate(const XMFLOAT3& value);
			void RotateRollPitchYaw(const XMFLOAT3& value);
			void Rotate(const XMFLOAT4& quaternion);
			void Scale(const XMFLOAT3& value);
			void MatrixTransform(const XMFLOAT4X4& matrix);
			void MatrixTransform(const XMMATRIX& matrix);
			void Lerp(const TransformComponent& a, const TransformComponent& b, float t);
			void CatmullRom(const TransformComponent& a, const TransformComponent& b, const TransformComponent& c, const TransformComponent& d, float t);

			void Serialize(asArchive& archive, uint32_t seed = 0);
		};

		struct PreviousFrameTransformComponent
		{
			// Non-serialized attributes:
			XMFLOAT4X4 world_prev;

			void Serialize(asArchive& archive, uint32_t seed = 0);
		};

		struct HierarchyComponent
		{
			asECS::Entity parentID = asECS::INVALID_ENTITY;
			uint32_t layerMask_bind; // saved child layermask at the time of binding
			XMFLOAT4X4 world_parent_inverse_bind; // saved parent inverse worldmatrix at the time of binding

			void Serialize(asArchive& archive, uint32_t seed = 0);
		};

		struct MaterialComponent
		{
			enum FLAGS
			{
				EMPTY = 0,
				DIRTY = 1 << 0,
				CAST_SHADOW = 1 << 1,
				PLANAR_REFLECTION = 1 << 2,
				WATER = 1 << 3,
				FLIP_NORMALMAP = 1 << 4,
				USE_VERTEXCOLORS = 1 << 5,
				SPECULAR_GLOSSINESS_WORKFLOW = 1 << 6,
				OCCLUSION_PRIMARY = 1 << 7,
				OCCLUSION_SECONDARY = 1 << 8,
			};
			uint32_t _flags = DIRTY | CAST_SHADOW;

			STENCILREF engineStencilRef = STENCILREF_DEFAULT;
			uint8_t userStencilRef = 0;
			BLENDMODE userBlendMode = BLENDMODE_OPAQUE;

			XMFLOAT4 baseColor = XMFLOAT4(1, 1, 1, 1);
			XMFLOAT4 emissiveColor = XMFLOAT4(1, 1, 1, 0);
			XMFLOAT4 texMulAdd = XMFLOAT4(1, 1, 0, 0);
			float roughness = 0.2f;
			float reflectance = 0.02f;
			float metalness = 0.0f;
			float refractionIndex = 0.0f;
			float subsurfaceScattering = 0.0f;
			float normalMapStrength = 1.0f;
			float parallaxOcclusionMapping = 0.0f;
			float displacementMapping = 0.0f;

			float alphaRef = 1.0f;

			XMFLOAT2 texAnimDirection = XMFLOAT2(0, 0);
			float texAnimFrameRate = 0.0f;
			float texAnimElapsedTime = 0.0f;

			std::string baseColorMapName;
			std::string surfaceMapName;
			std::string normalMapName;
			std::string displacementMapName;
			std::string emissiveMapName;
			std::string occlusionMapName;

			uint32_t uvset_baseColorMap = 0;
			uint32_t uvset_surfaceMap = 0;
			uint32_t uvset_normalMap = 0;
			uint32_t uvset_displacementMap = 0;
			uint32_t uvset_emissiveMap = 0;
			uint32_t uvset_occlusionMap = 0;

			// Non-serialized attributes:
			std::shared_ptr<asResource> baseColorMap;
			std::shared_ptr<asResource> surfaceMap;
			std::shared_ptr<asResource> normalMap;
			std::shared_ptr<asResource> displacementMap;
			std::shared_ptr<asResource> emissiveMap;
			std::shared_ptr<asResource> occlusionMap;
			std::unique_ptr<asGraphics::GPUBuffer> constantBuffer;

			int customShaderID = -1; // for now, this is not serialized; need to consider actual proper use case first

			// User stencil value can be in range [0, 15]
			inline void SetUserStencilRef(uint8_t value)
			{
				assert(value < 16);
				userStencilRef = value & 0x0F;
			}
			inline uint32_t GetStencilRef() const
			{
				return asRenderer::CombineStencilrefs(engineStencilRef, userStencilRef);
			}

			const asGraphics::Texture* GetBaseColorMap() const;
			const asGraphics::Texture* GetNormalMap() const;
			const asGraphics::Texture* GetSurfaceMap() const;
			const asGraphics::Texture* GetDisplacementMap() const;
			const asGraphics::Texture* GetEmissiveMap() const;
			const asGraphics::Texture* GetOcclusionMap() const;

			inline float GetOpacity() const { return baseColor.w; }
			inline float GetEmissiveStrength() const { return emissiveColor.w; }
			inline int GetCustomShaderID() const { return customShaderID; }

			inline void SetDirty(bool value = true) { if (value) { _flags |= DIRTY; } else { _flags &= ~DIRTY; } }
			inline bool IsDirty() const { return _flags & DIRTY; }

			inline void SetCastShadow(bool value) { if (value) { _flags |= CAST_SHADOW; } else { _flags &= ~CAST_SHADOW; } }
			inline void SetPlanarReflections(bool value) { if (value) { _flags |= PLANAR_REFLECTION; } else { _flags &= ~PLANAR_REFLECTION; } }
			inline void SetWater(bool value) { if (value) { _flags |= WATER; } else { _flags &= ~WATER; } }
			inline void SetOcclusionEnabled_Primary(bool value) { SetDirty(); if (value) { _flags |= OCCLUSION_PRIMARY; } else { _flags &= ~OCCLUSION_PRIMARY; } }
			inline void SetOcclusionEnabled_Secondary(bool value) { SetDirty(); if (value) { _flags |= OCCLUSION_SECONDARY; } else { _flags &= ~OCCLUSION_SECONDARY; } }

			inline bool IsTransparent() const { return userBlendMode != BLENDMODE_OPAQUE || IsCustomShader() || IsWater(); }
			inline BLENDMODE GetBlendMode() const { if (userBlendMode == BLENDMODE_OPAQUE && IsTransparent()) { return BLENDMODE_ALPHA; } else return userBlendMode; }
			inline bool IsWater() const { return _flags & WATER; }
			inline bool HasPlanarReflection() const { return (_flags & PLANAR_REFLECTION) || IsWater(); }
			inline bool IsCastingShadow() const { return _flags & CAST_SHADOW; }
			inline bool IsAlphaTestEnabled() const { return alphaRef <= 1.0f - 1.0f / 256.0f; }
			inline bool IsFlipNormalMap() const { return _flags & FLIP_NORMALMAP; }
			inline bool IsUsingVertexColors() const { return _flags & USE_VERTEXCOLORS; }
			inline bool IsUsingSpecularGlossinessWorkflow() const { return _flags & SPECULAR_GLOSSINESS_WORKFLOW; }
			inline bool IsOcclusionEnabled_Primary() const { return _flags & OCCLUSION_PRIMARY; }
			inline bool IsOcclusionEnabled_Secondary() const { return _flags & OCCLUSION_SECONDARY; }
			inline bool IsCustomShader() const { return customShaderID >= 0; }

			inline void SetBaseColor(const XMFLOAT4& value) { SetDirty(); baseColor = value; }
			inline void SetEmissiveColor(const XMFLOAT4& value) { SetDirty(); emissiveColor = value; }
			inline void SetRoughness(float value) { SetDirty(); roughness = value; }
			inline void SetReflectance(float value) { SetDirty(); reflectance = value; }
			inline void SetMetalness(float value) { SetDirty(); metalness = value; }
			inline void SetEmissiveStrength(float value) { SetDirty(); emissiveColor.w = value; }
			inline void SetRefractionIndex(float value) { SetDirty(); refractionIndex = value; }
			inline void SetSubsurfaceScattering(float value) { SetDirty(); subsurfaceScattering = value; }
			inline void SetNormalMapStrength(float value) { SetDirty(); normalMapStrength = value; }
			inline void SetParallaxOcclusionMapping(float value) { SetDirty(); parallaxOcclusionMapping = value; }
			inline void SetDisplacementMapping(float value) { SetDirty(); displacementMapping = value; }
			inline void SetOpacity(float value) { SetDirty(); baseColor.w = value; }
			inline void SetAlphaRef(float value) { SetDirty();  alphaRef = value; }
			inline void SetFlipNormalMap(bool value) { SetDirty(); if (value) { _flags |= FLIP_NORMALMAP; } else { _flags &= ~FLIP_NORMALMAP; } }
			inline void SetUseVertexColors(bool value) { SetDirty(); if (value) { _flags |= USE_VERTEXCOLORS; } else { _flags &= ~USE_VERTEXCOLORS; } }
			inline void SetUseSpecularGlossinessWorkflow(bool value) { SetDirty(); if (value) { _flags |= SPECULAR_GLOSSINESS_WORKFLOW; } else { _flags &= ~SPECULAR_GLOSSINESS_WORKFLOW; } }
			inline void SetCustomShaderID(int id) { customShaderID = id; }
			inline void DisableCustomShader() { customShaderID = -1; }
			inline void SetUVSet_BaseColorMap(uint32_t value) { uvset_baseColorMap = value; SetDirty(); }
			inline void SetUVSet_NormalMap(uint32_t value) { uvset_normalMap = value; SetDirty(); }
			inline void SetUVSet_SurfaceMap(uint32_t value) { uvset_surfaceMap = value; SetDirty(); }
			inline void SetUVSet_DisplacementMap(uint32_t value) { uvset_displacementMap = value; SetDirty(); }
			inline void SetUVSet_EmissiveMap(uint32_t value) { uvset_emissiveMap = value; SetDirty(); }
			inline void SetUVSet_OcclusionMap(uint32_t value) { uvset_occlusionMap = value; SetDirty(); }

			ShaderMaterial CreateShaderMaterial() const;

			void Serialize(asArchive& archive, uint32_t seed = 0);
		};

		struct MeshComponent
		{
			enum FLAGS
			{
				EMPTY = 0,
				RENDERABLE = 1 << 0,
				DOUBLE_SIDED = 1 << 1,
				DYNAMIC = 1 << 2,
			};
			uint32_t _flags = RENDERABLE;

			std::vector<XMFLOAT3>		vertex_positions;
			std::vector<XMFLOAT3>		vertex_normals;
			std::vector<XMFLOAT2>		vertex_uvset_0;
			std::vector<XMFLOAT2>		vertex_uvset_1;
			std::vector<XMUINT4>		vertex_boneindices;
			std::vector<XMFLOAT4>		vertex_boneweights;
			std::vector<XMFLOAT2>		vertex_atlas;
			std::vector<uint32_t>		vertex_colors;
			std::vector<uint32_t>		indices;

			struct MeshSubset
			{
				asECS::Entity materialID = asECS::INVALID_ENTITY;
				uint32_t indexOffset = 0;
				uint32_t indexCount = 0;
			};
			std::vector<MeshSubset>		subsets;

			float tessellationFactor = 0.0f;
			asECS::Entity armatureID = asECS::INVALID_ENTITY;

			// Non-serialized attributes:
			AABB aabb;
			std::unique_ptr<asGraphics::GPUBuffer>	indexBuffer;
			std::unique_ptr<asGraphics::GPUBuffer>	vertexBuffer_POS;
			std::unique_ptr<asGraphics::GPUBuffer>	vertexBuffer_UV0;
			std::unique_ptr<asGraphics::GPUBuffer>	vertexBuffer_UV1;
			std::unique_ptr<asGraphics::GPUBuffer>	vertexBuffer_BON;
			std::unique_ptr<asGraphics::GPUBuffer>	vertexBuffer_COL;
			std::unique_ptr<asGraphics::GPUBuffer>	vertexBuffer_ATL;
			std::unique_ptr<asGraphics::GPUBuffer>	vertexBuffer_PRE;
			std::unique_ptr<asGraphics::GPUBuffer>	streamoutBuffer_POS;


			inline void SetRenderable(bool value) { if (value) { _flags |= RENDERABLE; } else { _flags &= ~RENDERABLE; } }
			inline void SetDoubleSided(bool value) { if (value) { _flags |= DOUBLE_SIDED; } else { _flags &= ~DOUBLE_SIDED; } }
			inline void SetDynamic(bool value) { if (value) { _flags |= DYNAMIC; } else { _flags &= ~DYNAMIC; } }

			inline bool IsRenderable() const { return _flags & RENDERABLE; }
			inline bool IsDoubleSided() const { return _flags & DOUBLE_SIDED; }
			inline bool IsDynamic() const { return _flags & DYNAMIC; }

			inline float GetTessellationFactor() const { return tessellationFactor; }
			inline asGraphics::INDEXBUFFER_FORMAT GetIndexFormat() const { return vertex_positions.size() > 65535 ? asGraphics::INDEXFORMAT_32BIT : asGraphics::INDEXFORMAT_16BIT; }
			inline bool IsSkinned() const { return armatureID != asECS::INVALID_ENTITY; }

			void CreateRenderData();
			void ComputeNormals(bool smooth);
			void FlipCulling();
			void FlipNormals();
			void Recenter();
			void RecenterToBottom();

			void Serialize(asArchive& archive, uint32_t seed = 0);


			struct Vertex_POS
			{
				XMFLOAT3 pos = XMFLOAT3(0.0f, 0.0f, 0.0f);
				uint32_t normal_subsetIndex = 0;

				void FromFULL(const XMFLOAT3& _pos, const XMFLOAT3& _nor, uint32_t subsetIndex)
				{
					pos.x = _pos.x;
					pos.y = _pos.y;
					pos.z = _pos.z;
					MakeFromParams(_nor, subsetIndex);
				}
				inline XMVECTOR LoadPOS() const
				{
					return XMLoadFloat3(&pos);
				}
				inline XMVECTOR LoadNOR() const
				{
					return XMLoadFloat3(&GetNor_FULL());
				}
				inline void MakeFromParams(const XMFLOAT3& normal)
				{
					normal_subsetIndex = normal_subsetIndex & 0xFF000000; // reset only the normals

					normal_subsetIndex |= (uint32_t)((normal.x * 0.5f + 0.5f) * 255.0f) << 0;
					normal_subsetIndex |= (uint32_t)((normal.y * 0.5f + 0.5f) * 255.0f) << 8;
					normal_subsetIndex |= (uint32_t)((normal.z * 0.5f + 0.5f) * 255.0f) << 16;
				}
				inline void MakeFromParams(const XMFLOAT3& normal, uint32_t subsetIndex)
				{
					assert(subsetIndex < 256); // subsetIndex is packed onto 8 bits

					normal_subsetIndex = 0;

					normal_subsetIndex |= (uint32_t)((normal.x * 0.5f + 0.5f) * 255.0f) << 0;
					normal_subsetIndex |= (uint32_t)((normal.y * 0.5f + 0.5f) * 255.0f) << 8;
					normal_subsetIndex |= (uint32_t)((normal.z * 0.5f + 0.5f) * 255.0f) << 16;
					normal_subsetIndex |= (subsetIndex & 0x000000FF) << 24;
				}
				inline XMFLOAT3 GetNor_FULL() const
				{
					XMFLOAT3 nor_FULL(0, 0, 0);

					nor_FULL.x = (float)((normal_subsetIndex >> 0) & 0x000000FF) / 255.0f * 2.0f - 1.0f;
					nor_FULL.y = (float)((normal_subsetIndex >> 8) & 0x000000FF) / 255.0f * 2.0f - 1.0f;
					nor_FULL.z = (float)((normal_subsetIndex >> 16) & 0x000000FF) / 255.0f * 2.0f - 1.0f;

					return nor_FULL;
				}
				inline uint32_t GetMaterialIndex() const
				{
					return (normal_subsetIndex >> 24) & 0x000000FF;
				}

				static const asGraphics::FORMAT FORMAT = asGraphics::FORMAT::FORMAT_R32G32B32A32_FLOAT;
			};
			struct Vertex_TEX
			{
				XMHALF2 tex = XMHALF2(0.0f, 0.0f);

				void FromFULL(const XMFLOAT2& texcoords)
				{
					tex = XMHALF2(texcoords.x, texcoords.y);
				}

				static const asGraphics::FORMAT FORMAT = asGraphics::FORMAT::FORMAT_R16G16_FLOAT;
			};
			struct Vertex_BON
			{
				uint64_t ind = 0;
				uint64_t wei = 0;

				void FromFULL(const XMUINT4& boneIndices, const XMFLOAT4& boneWeights)
				{
					ind = 0;
					wei = 0;

					ind |= (uint64_t)boneIndices.x << 0;
					ind |= (uint64_t)boneIndices.y << 16;
					ind |= (uint64_t)boneIndices.z << 32;
					ind |= (uint64_t)boneIndices.w << 48;

					wei |= (uint64_t)(boneWeights.x * 65535.0f) << 0;
					wei |= (uint64_t)(boneWeights.y * 65535.0f) << 16;
					wei |= (uint64_t)(boneWeights.z * 65535.0f) << 32;
					wei |= (uint64_t)(boneWeights.w * 65535.0f) << 48;
				}
				inline XMUINT4 GetInd_FULL() const
				{
					XMUINT4 ind_FULL(0, 0, 0, 0);

					ind_FULL.x = ((ind >> 0) & 0x0000FFFF);
					ind_FULL.y = ((ind >> 16) & 0x0000FFFF);
					ind_FULL.z = ((ind >> 32) & 0x0000FFFF);
					ind_FULL.w = ((ind >> 48) & 0x0000FFFF);

					return ind_FULL;
				}
				inline XMFLOAT4 GetWei_FULL() const
				{
					XMFLOAT4 wei_FULL(0, 0, 0, 0);

					wei_FULL.x = (float)((wei >> 0) & 0x0000FFFF) / 65535.0f;
					wei_FULL.y = (float)((wei >> 16) & 0x0000FFFF) / 65535.0f;
					wei_FULL.z = (float)((wei >> 32) & 0x0000FFFF) / 65535.0f;
					wei_FULL.w = (float)((wei >> 48) & 0x0000FFFF) / 65535.0f;

					return wei_FULL;
				}
			};
			struct Vertex_COL
			{
				uint32_t color = 0;
				static const asGraphics::FORMAT FORMAT = asGraphics::FORMAT::FORMAT_R8G8B8A8_UNORM;
			};

		};

		struct ImpostorComponent
		{
			enum FLAGS
			{
				EMPTY = 0,
				DIRTY = 1 << 0,
			};
			uint32_t _flags = DIRTY;

			float swapInDistance = 100.0f;

			// Non-serialized attributes:
			AABB aabb;
			float fadeThresholdRadius;
			std::vector<XMFLOAT4X4> instanceMatrices;

			inline void SetDirty(bool value = true) { if (value) { _flags |= DIRTY; } else { _flags &= ~DIRTY; } }
			inline bool IsDirty() const { return _flags & DIRTY; }

			void Serialize(asArchive& archive, uint32_t seed = 0);
		};

		struct ObjectComponent
		{
			enum FLAGS
			{
				EMPTY = 0,
				RENDERABLE = 1 << 0,
				CAST_SHADOW = 1 << 1,
				DYNAMIC = 1 << 2,
				IMPOSTOR_PLACEMENT = 1 << 3,
				REQUEST_PLANAR_REFLECTION = 1 << 4,
				LIGHTMAP_RENDER_REQUEST = 1 << 5,
			};
			uint32_t _flags = RENDERABLE | CAST_SHADOW;

			asECS::Entity meshID = asECS::INVALID_ENTITY;
			uint32_t cascadeMask = 0; // which shadow cascades to skip (0: skip none, 1: skip first, etc...)
			uint32_t rendertypeMask = 0;
			XMFLOAT4 color = XMFLOAT4(1, 1, 1, 1);

			uint32_t lightmapWidth = 0;
			uint32_t lightmapHeight = 0;
			std::vector<uint8_t> lightmapTextureData;

			uint8_t userStencilRef = 0;

			// Non-serialized attributes:

			XMFLOAT4 globalLightMapMulAdd = XMFLOAT4(0, 0, 0, 0);
			std::unique_ptr<asGraphics::Texture> lightmap;
			std::unique_ptr<asGraphics::RenderPass> renderpass_lightmap_clear;
			std::unique_ptr<asGraphics::RenderPass> renderpass_lightmap_accumulate;
			uint32_t lightmapIterationCount = 0;

			XMFLOAT3 center = XMFLOAT3(0, 0, 0);
			float impostorFadeThresholdRadius;
			float impostorSwapDistance;

			// these will only be valid for a single frame:
			int transform_index = -1;
			int prev_transform_index = -1;

			// occlusion result history bitfield (32 bit->32 frame history)
			uint32_t occlusionHistory = ~0;
			// occlusion query pool index
			int occlusionQueryID = -1;

			inline bool IsOccluded() const
			{
				// Perform a conservative occlusion test:
				// If it is visible in any frames in the history, it is determined visible in this frame
				// But if all queries failed in the history, it is occluded.
				// If it pops up for a frame after occluded, it is visible again for some frames
				return ((occlusionQueryID >= 0) && (occlusionHistory & 0xFFFFFFFF) == 0);
			}

			inline void SetRenderable(bool value) { if (value) { _flags |= RENDERABLE; } else { _flags &= ~RENDERABLE; } }
			inline void SetCastShadow(bool value) { if (value) { _flags |= CAST_SHADOW; } else { _flags &= ~CAST_SHADOW; } }
			inline void SetDynamic(bool value) { if (value) { _flags |= DYNAMIC; } else { _flags &= ~DYNAMIC; } }
			inline void SetImpostorPlacement(bool value) { if (value) { _flags |= IMPOSTOR_PLACEMENT; } else { _flags &= ~IMPOSTOR_PLACEMENT; } }
			inline void SetRequestPlanarReflection(bool value) { if (value) { _flags |= REQUEST_PLANAR_REFLECTION; } else { _flags &= ~REQUEST_PLANAR_REFLECTION; } }
			inline void SetLightmapRenderRequest(bool value) { if (value) { _flags |= LIGHTMAP_RENDER_REQUEST; } else { _flags &= ~LIGHTMAP_RENDER_REQUEST; } }

			inline bool IsRenderable() const { return _flags & RENDERABLE; }
			inline bool IsCastingShadow() const { return _flags & CAST_SHADOW; }
			inline bool IsDynamic() const { return _flags & DYNAMIC; }
			inline bool IsImpostorPlacement() const { return _flags & IMPOSTOR_PLACEMENT; }
			inline bool IsRequestPlanarReflection() const { return _flags & REQUEST_PLANAR_REFLECTION; }
			inline bool IsLightmapRenderRequested() const { return _flags & LIGHTMAP_RENDER_REQUEST; }

			inline float GetTransparency() const { return 1 - color.w; }
			inline uint32_t GetRenderTypes() const { return rendertypeMask; }

			// User stencil value can be in range [0, 15]
			//	Values greater than 0 can be used to override userStencilRef of MaterialComponent
			inline void SetUserStencilRef(uint8_t value)
			{
				assert(value < 16);
				userStencilRef = value & 0x0F;
			}

			void ClearLightmap();
			void SaveLightmap();
			asGraphics::FORMAT GetLightmapFormat();

			void Serialize(asArchive& archive, uint32_t seed = 0);
		};

		struct RigidBodyPhysicsComponent
		{
			enum FLAGS
			{
				EMPTY = 0,
				DISABLE_DEACTIVATION = 1 << 0,
				KINEMATIC = 1 << 1,
			};
			uint32_t _flags = EMPTY;

			enum CollisionShape
			{
				BOX,
				SPHERE,
				CAPSULE,
				CONVEX_HULL,
				TRIANGLE_MESH,
				ENUM_FORCE_UINT32 = 0xFFFFFFFF
			};
			CollisionShape shape;
			float mass = 1.0f;
			float friction = 1.0f;
			float restitution = 1.0f;
			float damping = 1.0f;

			// Non-serialized attributes:
			void* physicsobject = nullptr;

			inline void SetDisableDeactivation(bool value) { if (value) { _flags |= DISABLE_DEACTIVATION; } else { _flags &= ~DISABLE_DEACTIVATION; } }
			inline void SetKinematic(bool value) { if (value) { _flags |= KINEMATIC; } else { _flags &= ~KINEMATIC; } }

			inline bool IsDisableDeactivation() const { return _flags & DISABLE_DEACTIVATION; }
			inline bool IsKinematic() const { return _flags & KINEMATIC; }

			void Serialize(asArchive& archive, uint32_t seed = 0);
		};

		struct SoftBodyPhysicsComponent
		{
			enum FLAGS
			{
				EMPTY = 0,
				SAFE_TO_REGISTER = 1 << 0,
				DISABLE_DEACTIVATION = 1 << 1,
			};
			uint32_t _flags = DISABLE_DEACTIVATION;

			float mass = 1.0f;
			float friction = 1.0f;
			std::vector<uint32_t> physicsToGraphicsVertexMapping; // maps graphics vertex index to physics vertex index of the same position
			std::vector<uint32_t> graphicsToPhysicsVertexMapping; // maps a physics vertex index to first graphics vertex index of the same position
			std::vector<float> weights; // weight per physics vertex controlling the mass. (0: disable weight (no physics, only animation), 1: default weight)
			std::vector<XMFLOAT3> restPose; // unsimulated vertex positions

			// Non-serialized attributes:
			void* physicsobject = nullptr;
			XMFLOAT4X4 worldMatrix = IDENTITYMATRIX;

			inline void SetDisableDeactivation(bool value) { if (value) { _flags |= DISABLE_DEACTIVATION; } else { _flags &= ~DISABLE_DEACTIVATION; } }

			inline bool IsDisableDeactivation() const { return _flags & DISABLE_DEACTIVATION; }

			// Create physics represenation of graphics mesh
			void CreateFromMesh(const MeshComponent& mesh);

			void Serialize(asArchive& archive, uint32_t seed = 0);
		};

		struct ArmatureComponent
		{
			enum FLAGS
			{
				EMPTY = 0,
			};
			uint32_t _flags = EMPTY;

			std::vector<asECS::Entity> boneCollection;
			std::vector<XMFLOAT4X4> inverseBindMatrices;

			// Non-serialized attributes:

			GFX_STRUCT ShaderBoneType
			{
				XMFLOAT4A pose0;
				XMFLOAT4A pose1;
				XMFLOAT4A pose2;

				inline void Store(const XMMATRIX& M)
				{
					XMFLOAT4X4 mat;
					XMStoreFloat4x4(&mat, M);
					pose0 = XMFLOAT4A(mat._11, mat._21, mat._31, mat._41);
					pose1 = XMFLOAT4A(mat._12, mat._22, mat._32, mat._42);
					pose2 = XMFLOAT4A(mat._13, mat._23, mat._33, mat._43);
				}
				inline XMMATRIX Load() const
				{
					return XMMATRIX(
						pose0.x, pose1.x, pose2.x, 0,
						pose0.y, pose1.y, pose2.y, 0,
						pose0.z, pose1.z, pose2.z, 0,
						pose0.w, pose1.w, pose2.w, 1
					);
				}

				ALIGN_16
			};
			std::vector<ShaderBoneType> boneData;
			std::unique_ptr<asGraphics::GPUBuffer> boneBuffer;

			void Serialize(asArchive& archive, uint32_t seed = 0);
		};

		struct LightComponent
		{
			enum FLAGS
			{
				EMPTY = 0,
				CAST_SHADOW = 1 << 0,
				VOLUMETRICS = 1 << 1,
				VISUALIZER = 1 << 2,
				LIGHTMAPONLY_STATIC = 1 << 3,
			};
			uint32_t _flags = EMPTY;
			XMFLOAT3 color = XMFLOAT3(1, 1, 1);

			enum LightType
			{
				DIRECTIONAL = ENTITY_TYPE_DIRECTIONALLIGHT,
				POINT = ENTITY_TYPE_POINTLIGHT,
				SPOT = ENTITY_TYPE_SPOTLIGHT,
				SPHERE = ENTITY_TYPE_SPHERELIGHT,
				DISC = ENTITY_TYPE_DISCLIGHT,
				RECTANGLE = ENTITY_TYPE_RECTANGLELIGHT,
				TUBE = ENTITY_TYPE_TUBELIGHT,
				LIGHTTYPE_COUNT,
				ENUM_FORCE_UINT32 = 0xFFFFFFFF,
			};
			LightType type = POINT;
			float energy = 1.0f;
			float range_local = 10.0f;
			float fov = XM_PIDIV4;
			float shadowBias = 0.0001f;
			float radius = 1.0f; // area light
			float width = 1.0f;  // area light
			float height = 1.0f; // area light

			std::vector<std::string> lensFlareNames;

			// Non-serialized attributes:
			XMFLOAT3 position;
			float range_global;
			XMFLOAT3 direction;
			XMFLOAT4 rotation;
			XMFLOAT3 scale;
			XMFLOAT3 front;
			XMFLOAT3 right;
			int shadowMap_index = -1;

			std::vector<std::shared_ptr<asResource>> lensFlareRimTextures;

			inline void SetCastShadow(bool value) { if (value) { _flags |= CAST_SHADOW; } else { _flags &= ~CAST_SHADOW; } }
			inline void SetVolumetricsEnabled(bool value) { if (value) { _flags |= VOLUMETRICS; } else { _flags &= ~VOLUMETRICS; } }
			inline void SetVisualizerEnabled(bool value) { if (value) { _flags |= VISUALIZER; } else { _flags &= ~VISUALIZER; } }
			inline void SetStatic(bool value) { if (value) { _flags |= LIGHTMAPONLY_STATIC; } else { _flags &= ~LIGHTMAPONLY_STATIC; } }

			inline bool IsCastingShadow() const { return _flags & CAST_SHADOW; }
			inline bool IsVolumetricsEnabled() const { return _flags & VOLUMETRICS; }
			inline bool IsVisualizerEnabled() const { return _flags & VISUALIZER; }
			inline bool IsStatic() const { return _flags & LIGHTMAPONLY_STATIC; }

			inline float GetRange() const { return range_global; }

			inline void SetType(LightType val) {
				type = val;
			}
			inline LightType GetType() const { return type; }

			void Serialize(asArchive& archive, uint32_t seed = 0);
		};

		struct CameraComponent
		{
			enum FLAGS
			{
				EMPTY = 0,
				DIRTY = 1 << 0,
			};
			uint32_t _flags = EMPTY;

			float width = 0.0f;
			float height = 0.0f;
			float zNearP = 0.001f;
			float zFarP = 800.0f;
			float fov = XM_PI / 3.0f;

			// Non-serialized attributes:
			XMFLOAT3 Eye = XMFLOAT3(0, 0, 0);
			XMFLOAT3 At = XMFLOAT3(0, 0, 1);
			XMFLOAT3 Up = XMFLOAT3(0, 1, 0);
			XMFLOAT3X3 rotationMatrix;
			XMFLOAT4X4 View, Projection, VP;
			Frustum frustum;
			XMFLOAT4X4 InvView, InvProjection, InvVP;
			XMFLOAT2 jitter;

			void CreatePerspective(float newWidth, float newHeight, float newNear, float newFar, float newFOV = XM_PI / 3.0f);
			void UpdateCamera();
			void TransformCamera(const TransformComponent& transform);
			void Reflect(const XMFLOAT4& plane = XMFLOAT4(0, 1, 0, 0));

			inline XMVECTOR GetEye() const { return XMLoadFloat3(&Eye); }
			inline XMVECTOR GetAt() const { return XMLoadFloat3(&At); }
			inline XMVECTOR GetUp() const { return XMLoadFloat3(&Up); }
			inline XMVECTOR GetRight() const { return XMVector3Cross(GetAt(), GetUp()); }
			inline XMMATRIX GetView() const { return XMLoadFloat4x4(&View); }
			inline XMMATRIX GetInvView() const { return XMLoadFloat4x4(&InvView); }
			inline XMMATRIX GetProjection() const { return XMLoadFloat4x4(&Projection); }
			inline XMMATRIX GetInvProjection() const { return XMLoadFloat4x4(&InvProjection); }
			inline XMMATRIX GetViewProjection() const { return XMLoadFloat4x4(&VP); }
			inline XMMATRIX GetInvViewProjection() const { return XMLoadFloat4x4(&InvVP); }

			inline void SetDirty(bool value = true) { if (value) { _flags |= DIRTY; } else { _flags &= ~DIRTY; } }
			inline bool IsDirty() const { return _flags & DIRTY; }

			void Serialize(asArchive& archive, uint32_t seed = 0);
		};

		struct EnvironmentProbeComponent
		{
			enum FLAGS
			{
				EMPTY = 0,
				DIRTY = 1 << 0,
				REALTIME = 1 << 1,
			};
			uint32_t _flags = DIRTY;

			// Non-serialized attributes:
			int textureIndex = -1;
			XMFLOAT3 position;
			float range;
			XMFLOAT4X4 inverseMatrix;

			inline void SetDirty(bool value = true) { if (value) { _flags |= DIRTY; } else { _flags &= ~DIRTY; } }
			inline void SetRealTime(bool value) { if (value) { _flags |= REALTIME; } else { _flags &= ~REALTIME; } }

			inline bool IsDirty() const { return _flags & DIRTY; }
			inline bool IsRealTime() const { return _flags & REALTIME; }

			void Serialize(asArchive& archive, uint32_t seed = 0);
		};

		struct ForceFieldComponent
		{
			enum FLAGS
			{
				EMPTY = 0,
			};
			uint32_t _flags = EMPTY;

			int type = ENTITY_TYPE_FORCEFIELD_POINT;
			float gravity = 0.0f; // negative = deflector, positive = attractor
			float range_local = 0.0f; // affection range

			// Non-serialized attributes:
			XMFLOAT3 position;
			float range_global;
			XMFLOAT3 direction;

			inline float GetRange() const { return range_global; }

			void Serialize(asArchive& archive, uint32_t seed = 0);
		};

		struct DecalComponent
		{
			enum FLAGS
			{
				EMPTY = 0,
			};
			uint32_t _flags = EMPTY;

			// Non-serialized attributes:
			XMFLOAT4 color;
			float emissive;
			XMFLOAT3 front;
			XMFLOAT3 position;
			float range;
			XMFLOAT4 atlasMulAdd;
			XMFLOAT4X4 world;

			const asGraphics::Texture* texture = nullptr;
			const asGraphics::Texture* normal = nullptr;

			inline float GetOpacity() const { return color.w; }

			void Serialize(asArchive& archive, uint32_t seed = 0);
		};

		struct AnimationComponent
		{
			enum FLAGS
			{
				EMPTY = 0,
				PLAYING = 1 << 0,
				LOOPED = 1 << 1,
			};
			uint32_t _flags = LOOPED;
			float start = 0;
			float end = 0;
			float timer = 0.0f;

			struct AnimationChannel
			{
				enum FLAGS
				{
					EMPTY = 0,
				};
				uint32_t _flags = EMPTY;

				enum Path
				{
					TRANSLATION,
					ROTATION,
					SCALE,
					UNKNOWN,
					TYPE_FORCE_UINT32 = 0xFFFFFFFF
				} path = TRANSLATION;

				asECS::Entity target = asECS::INVALID_ENTITY;
				uint32_t samplerIndex = 0;
			};
			struct AnimationSampler
			{
				enum FLAGS
				{
					EMPTY = 0,
				};
				uint32_t _flags = EMPTY;

				enum Mode
				{
					LINEAR,
					STEP,
					MODE_FORCE_UINT32 = 0xFFFFFFFF
				} mode = LINEAR;

				std::vector<float> keyframe_times;
				std::vector<float> keyframe_data;
			};

			std::vector<AnimationChannel> channels;
			std::vector<AnimationSampler> samplers;

			inline bool IsPlaying() const { return _flags & PLAYING; }
			inline bool IsLooped() const { return _flags & LOOPED; }
			inline float GetLength() const { return end - start; }
			inline bool IsEnded() const { return timer >= end; }

			inline void Play() { _flags |= PLAYING; }
			inline void Pause() { _flags &= ~PLAYING; }
			inline void Stop() { Pause(); timer = 0.0f; }
			inline void SetLooped(bool value = true) { if (value) { _flags |= LOOPED; } else { _flags &= ~LOOPED; } }

			void Serialize(asArchive& archive, uint32_t seed = 0);
		};

		struct WeatherComponent
		{
			enum FLAGS
			{
				EMPTY = 0,
				OCEAN_ENABLED = 1 << 0
			};
			uint32_t _flags = EMPTY;

			inline bool IsOceanEnabled() const { return _flags & OCEAN_ENABLED; }
			inline void SetOceanEnabled(bool value = true) { if (value) { _flags |= OCEAN_ENABLED; } else { _flags &= ~OCEAN_ENABLED; } }

			XMFLOAT3 sunColor = XMFLOAT3(0, 0, 0);
			XMFLOAT3 sunDirection = XMFLOAT3(0, 1, 0);
			//rgba(15, 188, 249, 1.0)

			XMFLOAT3 ambient = XMFLOAT3(0.1f, 0.1f, 0.1f);
			XMFLOAT3 horizon = XMFLOAT3(0.3f, 0.3f, 0.4f);
			XMFLOAT3 zenith = XMFLOAT3(37.0f / 255.0f, 61.0f / 255.0f, 190.0f / 255.0f);
			//XMFLOAT3 horizon = XMFLOAT3(0.0f, 0.0f, 0.0f);
			//XMFLOAT3 zenith = XMFLOAT3(0.0f, 0.0f, 0.0f);
			//XMFLOAT3 ambient = XMFLOAT3(0.2f, 0.2f, 0.2f);
			float fogStart = 100;
			float fogEnd = 1000;
			float fogHeight = 0;
			float cloudiness = 0.0f;
			float cloudScale = 0.0003f;
			float cloudSpeed = 0.1f;
			XMFLOAT3 windDirection = XMFLOAT3(0, 0, 0);
			float windRandomness = 5;
			float windWaveSize = 1;

			struct OceanParameters
			{
				// Must be power of 2.
				int dmap_dim = 512;
				// Typical value is 1000 ~ 2000
				float patch_length = 50.0f;

				// Adjust the time interval for simulation.
				float time_scale = 0.3f;
				// Amplitude for transverse wave. Around 1.0
				float wave_amplitude = 1000.0f;
				// Wind direction. Normalization not required.
				XMFLOAT2 wind_dir = XMFLOAT2(0.8f, 0.6f);
				// Around 100 ~ 1000
				float wind_speed = 600.0f;
				// This value damps out the waves against the wind direction.
				// Smaller value means higher wind dependency.
				float wind_dependency = 0.07f;
				// The amplitude for longitudinal wave. Must be positive.
				float choppy_scale = 1.3f;


				XMFLOAT3 waterColor = XMFLOAT3(0.0f, 3.0f / 255.0f, 31.0f / 255.0f);
				float waterHeight = 0.0f;
				uint32_t surfaceDetail = 4;
				float surfaceDisplacementTolerance = 2;
			};
			OceanParameters oceanParameters;

			std::string skyMapName;
			std::shared_ptr<asResource> skyMap;

			void Serialize(asArchive& archive, uint32_t seed = 0);
		};

		struct SoundComponent
		{
			enum FLAGS
			{
				EMPTY = 0,
				PLAYING = 1 << 0,
				LOOPED = 1 << 1,
			};
			uint32_t _flags = LOOPED;

			std::string filename;
			std::shared_ptr<asResource> soundResource;
			asAudio::SoundInstance soundinstance;
			float volume = 1;

			inline bool IsPlaying() const { return _flags & PLAYING; }
			inline bool IsLooped() const { return _flags & LOOPED; }

			inline void Play() { _flags |= PLAYING; }
			inline void Stop() { _flags &= ~PLAYING; }
			inline void SetLooped(bool value = true) { if (value) { _flags |= LOOPED; } else { _flags &= ~LOOPED; } }

			void Serialize(asArchive& archive, uint32_t seed = 0);
		};

		struct Scene
		{
			asECS::ComponentManager<NameComponent> names;
			asECS::ComponentManager<LayerComponent> layers;
			asECS::ComponentManager<TransformComponent> transforms;
			asECS::ComponentManager<PreviousFrameTransformComponent> prev_transforms;
			asECS::ComponentManager<HierarchyComponent> hierarchy;
			asECS::ComponentManager<MaterialComponent> materials;
			asECS::ComponentManager<MeshComponent> meshes;
			asECS::ComponentManager<ImpostorComponent> impostors;
			asECS::ComponentManager<ObjectComponent> objects;
			asECS::ComponentManager<AABB> aabb_objects;
			asECS::ComponentManager<RigidBodyPhysicsComponent> rigidbodies;
			asECS::ComponentManager<SoftBodyPhysicsComponent> softbodies;
			asECS::ComponentManager<ArmatureComponent> armatures;
			asECS::ComponentManager<LightComponent> lights;
			asECS::ComponentManager<AABB> aabb_lights;
			asECS::ComponentManager<CameraComponent> cameras;
			asECS::ComponentManager<EnvironmentProbeComponent> probes;
			asECS::ComponentManager<AABB> aabb_probes;
			asECS::ComponentManager<ForceFieldComponent> forces;
			asECS::ComponentManager<DecalComponent> decals;
			asECS::ComponentManager<AABB> aabb_decals;
			asECS::ComponentManager<AnimationComponent> animations;
			asECS::ComponentManager<asEmittedParticle> emitters;
			asECS::ComponentManager<asHairParticle> hairs;
			asECS::ComponentManager<WeatherComponent> weathers;
			asECS::ComponentManager<SoundComponent> sounds;

			// Non-serialized attributes:
			AABB bounds;
			XMFLOAT4 waterPlane = XMFLOAT4(0, 1, 0, 0);
			WeatherComponent weather;

			// Update all components by a given timestep (in seconds):
			void Update(float dt);
			// Remove everything from the scene that it owns:
			void Clear();
			// Merge with an other scene.
			void Merge(Scene& other);

			// Removes a specific entity from the scene (if it exists):
			void Entity_Remove(asECS::Entity entity);
			// Finds the first entity by the name (if it exists, otherwise returns INVALID_ENTITY):
			asECS::Entity Entity_FindByName(const std::string& name);
			// Duplicates all of an entity's components and creates a new entity with them:
			asECS::Entity Entity_Duplicate(asECS::Entity entity);
			// Serializes entity and all of its components to archive:
			//	You can specify entity = INVALID_ENTITY when the entity needs to be created from archive
			//	You can specify seed = 0 when the archive is guaranteed to be storing persistent and unique entities
			//	propagateDeepSeed : request that entity references inside components should be seeded as well
			//	Returns either the new entity that was read, or the original entity that was written
			asECS::Entity Entity_Serialize(asArchive& archive, asECS::Entity entity = asECS::INVALID_ENTITY, uint32_t seed = 0, bool propagateSeedDeep = true);

			asECS::Entity Entity_CreateMaterial(
				const std::string& name
			);
			asECS::Entity Entity_CreateObject(
				const std::string& name
			);
			asECS::Entity Entity_CreateMesh(
				const std::string& name
			);
			asECS::Entity Entity_CreateLight(
				const std::string& name,
				const XMFLOAT3& position = XMFLOAT3(0, 0, 0),
				const XMFLOAT3& color = XMFLOAT3(1, 1, 1),
				float energy = 1,
				float range = 10
			);
			asECS::Entity Entity_CreateForce(
				const std::string& name,
				const XMFLOAT3& position = XMFLOAT3(0, 0, 0)
			);
			asECS::Entity Entity_CreateEnvironmentProbe(
				const std::string& name,
				const XMFLOAT3& position = XMFLOAT3(0, 0, 0)
			);
			asECS::Entity Entity_CreateDecal(
				const std::string& name,
				const std::string& textureName,
				const std::string& normalMapName = ""
			);
			asECS::Entity Entity_CreateCamera(
				const std::string& name,
				float width, float height, float nearPlane = 0.01f, float farPlane = 1000.0f, float fov = XM_PIDIV4
			);
			asECS::Entity Entity_CreateEmitter(
				const std::string& name,
				const XMFLOAT3& position = XMFLOAT3(0, 0, 0)
			);
			asECS::Entity Entity_CreateHair(
				const std::string& name,
				const XMFLOAT3& position = XMFLOAT3(0, 0, 0)
			);
			asECS::Entity Entity_CreateSound(
				const std::string& name,
				const std::string& filename,
				const XMFLOAT3& position = XMFLOAT3(0, 0, 0)
			);

			// Attaches an entity to a parent:
			void Component_Attach(asECS::Entity entity, asECS::Entity parent);
			// Detaches the entity from its parent (if it is attached):
			void Component_Detach(asECS::Entity entity);
			// Detaches all children from an entity (if there are any):
			void Component_DetachChildren(asECS::Entity parent);

			void Serialize(asArchive& archive);
		};

		void RunPreviousFrameTransformUpdateSystem(
			asJobSystem::context& ctx,
			const asECS::ComponentManager<TransformComponent>& transforms,
			asECS::ComponentManager<PreviousFrameTransformComponent>& prev_transforms
		);
		void RunAnimationUpdateSystem(
			asJobSystem::context& ctx,
			asECS::ComponentManager<AnimationComponent>& animations,
			asECS::ComponentManager<TransformComponent>& transforms,
			float dt
		);
		void RunTransformUpdateSystem(
			asJobSystem::context& ctx,
			asECS::ComponentManager<TransformComponent>& transforms
		);
		void RunHierarchyUpdateSystem(
			asJobSystem::context& ctx,
			const asECS::ComponentManager<HierarchyComponent>& hierarchy,
			asECS::ComponentManager<TransformComponent>& transforms,
			asECS::ComponentManager<LayerComponent>& layers
		);
		void RunArmatureUpdateSystem(
			asJobSystem::context& ctx,
			const asECS::ComponentManager<TransformComponent>& transforms,
			asECS::ComponentManager<ArmatureComponent>& armatures
		);
		void RunMaterialUpdateSystem(
			asJobSystem::context& ctx,
			asECS::ComponentManager<MaterialComponent>& materials, float dt
		);
		void RunImpostorUpdateSystem(
			asJobSystem::context& ctx,
			asECS::ComponentManager<ImpostorComponent>& impostors
		);
		void RunObjectUpdateSystem(
			asJobSystem::context& ctx,
			const asECS::ComponentManager<PreviousFrameTransformComponent>& prev_transforms,
			const asECS::ComponentManager<TransformComponent>& transforms,
			const asECS::ComponentManager<MeshComponent>& meshes,
			const asECS::ComponentManager<MaterialComponent>& materials,
			asECS::ComponentManager<ObjectComponent>& objects,
			asECS::ComponentManager<AABB>& aabb_objects,
			asECS::ComponentManager<ImpostorComponent>& impostors,
			asECS::ComponentManager<SoftBodyPhysicsComponent>& softbodies,
			AABB& sceneBounds,
			XMFLOAT4& waterPlane
		);
		void RunCameraUpdateSystem(
			asJobSystem::context& ctx,
			const asECS::ComponentManager<TransformComponent>& transforms,
			asECS::ComponentManager<CameraComponent>& cameras
		);
		void RunDecalUpdateSystem(
			asJobSystem::context& ctx,
			const asECS::ComponentManager<TransformComponent>& transforms,
			const asECS::ComponentManager<MaterialComponent>& materials,
			asECS::ComponentManager<AABB>& aabb_decals,
			asECS::ComponentManager<DecalComponent>& decals
		);
		void RunProbeUpdateSystem(
			asJobSystem::context& ctx,
			const asECS::ComponentManager<TransformComponent>& transforms,
			asECS::ComponentManager<AABB>& aabb_probes,
			asECS::ComponentManager<EnvironmentProbeComponent>& probes
		);
		void RunForceUpdateSystem(
			asJobSystem::context& ctx,
			const asECS::ComponentManager<TransformComponent>& transforms,
			asECS::ComponentManager<ForceFieldComponent>& forces
		);
		void RunLightUpdateSystem(
			asJobSystem::context& ctx,
			const asECS::ComponentManager<TransformComponent>& transforms,
			asECS::ComponentManager<AABB>& aabb_lights,
			asECS::ComponentManager<LightComponent>& lights
		);
		void RunParticleUpdateSystem(
			asJobSystem::context& ctx,
			const asECS::ComponentManager<TransformComponent>& transforms,
			const asECS::ComponentManager<MeshComponent>& meshes,
			asECS::ComponentManager<asEmittedParticle>& emitters,
			asECS::ComponentManager<asHairParticle>& hairs,
			float dt
		);
		void RunWeatherUpdateSystem(
			asJobSystem::context& ctx,
			const asECS::ComponentManager<WeatherComponent>& weathers,
			const asECS::ComponentManager<LightComponent>& lights,
			WeatherComponent& weather
		);
		void RunSoundUpdateSystem(
			asJobSystem::context& ctx,
			const asECS::ComponentManager<TransformComponent>& transforms,
			asECS::ComponentManager<SoundComponent>& sounds
		);



		// Helper that manages a global scene
		inline Scene& GetScene()
		{
			static Scene scene;
			return scene;
		}

		// Helper function to open a asScene file and add the contents to the global scene
		//	fileName		:	file path
		//	transformMatrix	:	everything will be transformed by this matrix (optional)
		//	attached		:	everything will be attached to a base entity
		//
		//	returns INVALID_ENTITY if attached argument was false, else it returns the base entity handle
		asECS::Entity LoadModel(const std::string& fileName, const XMMATRIX& transformMatrix = XMMatrixIdentity(), bool attached = false);

		// Helper function to open a asScene file and add the contents to the specified scene. This is thread safe as it doesn't modify global scene
		//	scene			:	the scene that will contain the model
		//	fileName		:	file path
		//	transformMatrix	:	everything will be transformed by this matrix (optional)
		//	attached		:	everything will be attached to a base entity
		//
		//	returns INVALID_ENTITY if attached argument was false, else it returns the base entity handle
		asECS::Entity LoadModel(Scene& scene, const std::string& fileName, const XMMATRIX& transformMatrix = XMMatrixIdentity(), bool attached = false);

		struct PickResult
		{
			asECS::Entity entity = asECS::INVALID_ENTITY;
			XMFLOAT3 position = XMFLOAT3(0, 0, 0);
			XMFLOAT3 normal = XMFLOAT3(0, 0, 0);
			float distance = FLT_MAX;
			int subsetIndex = -1;
			int vertexID0 = -1;
			int vertexID1 = -1;
			int vertexID2 = -1;
			XMFLOAT4X4 orientation = IDENTITYMATRIX;

			bool operator==(const PickResult& other)
			{
				return entity == other.entity;
			}
		};
		// Given a ray, finds the closest intersection point against all mesh instances
		//	ray				:	the incoming ray that will be traced
		//	renderTypeMask	:	filter based on render type
		//	layerMask		:	filter based on layer
		//	scene			:	the scene that will be traced against the ray
		PickResult Pick(const RAY& ray, uint32_t renderTypeMask = RENDERTYPE_OPAQUE, uint32_t layerMask = ~0, const Scene& scene = GetScene());
	}
}
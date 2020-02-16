#ifndef AS_SHADER_GLOBALS_HF
#define AS_SHADER_GLOBALS_HF

#include "Graphics/GPUMapping/ShaderInterop.h"
#include "Graphics/GPUMapping/ShaderInterop_Renderer.h"

TEXTURE2D(texture_depth, float, TEXSLOT_DEPTH)
TEXTURE2D(texture_lineardepth, float, TEXSLOT_LINEARDEPTH)
TEXTURE2D(texture_gbuffer0, float4, TEXSLOT_GBUFFER0)
TEXTURE2D(texture_gbuffer1, float4, TEXSLOT_GBUFFER1)
TEXTURE2D(texture_gbuffer2, float4, TEXSLOT_GBUFFER2)
TEXTURECUBE(texture_globalenvmap, float4, TEXSLOT_GLOBALENVMAP)
TEXTURE2D(texture_globallightmap, float4, TEXSLOT_GLOBALLIGHTMAP)
TEXTURECUBEARRAY(texture_envmaparray, float4, TEXSLOT_ENVMAPARRAY)
TEXTURE2D(texture_decalatlas, float4, TEXSLOT_DECALATLAS)
TEXTURE2DARRAY(texture_shadowarray_2d, float, TEXSLOT_SHADOWARRAY_2D)
TEXTURECUBEARRAY(texture_shadowarray_cube, float, TEXSLOT_SHADOWARRAY_CUBE)
TEXTURE2DARRAY(texture_shadowarray_transparent, float4, TEXSLOT_SHADOWARRAY_TRANSPARENT)
TEXTURE3D(texture_voxelradiance, float4, TEXSLOT_VOXELRADIANCE)

STRUCTUREDBUFFER(EntityTiles, uint, SBSLOT_ENTITYTILES);
STRUCTUREDBUFFER(EntityArray, ShaderEntity, SBSLOT_ENTITYARRAY);
STRUCTUREDBUFFER(MatrixArray, float4x4, SBSLOT_MATRIXARRAY);

// Ondemand textures are 2d textures and declared in shader globals, these can be used independently in any shader:
TEXTURE2D(texture_0, float4, TEXSLOT_ONDEMAND0)
TEXTURE2D(texture_1, float4, TEXSLOT_ONDEMAND1)
TEXTURE2D(texture_2, float4, TEXSLOT_ONDEMAND2)
TEXTURE2D(texture_3, float4, TEXSLOT_ONDEMAND3)
TEXTURE2D(texture_4, float4, TEXSLOT_ONDEMAND4)
TEXTURE2D(texture_5, float4, TEXSLOT_ONDEMAND5)
TEXTURE2D(texture_6, float4, TEXSLOT_ONDEMAND6)
TEXTURE2D(texture_7, float4, TEXSLOT_ONDEMAND7)
TEXTURE2D(texture_8, float4, TEXSLOT_ONDEMAND8)
TEXTURE2D(texture_9, float4, TEXSLOT_ONDEMAND9)

SAMPLERSTATE(sampler_linear_clamp, SSLOT_LINEAR_CLAMP)
SAMPLERSTATE(sampler_linear_wrap, SSLOT_LINEAR_WRAP)
SAMPLERSTATE(sampler_linear_mirror, SSLOT_LINEAR_MIRROR)
SAMPLERSTATE(sampler_point_clamp, SSLOT_POINT_CLAMP)
SAMPLERSTATE(sampler_point_wrap, SSLOT_POINT_WRAP)
SAMPLERSTATE(sampler_point_mirror, SSLOT_POINT_MIRROR)
SAMPLERSTATE(sampler_aniso_clamp, SSLOT_ANISO_CLAMP)
SAMPLERSTATE(sampler_aniso_wrap, SSLOT_ANISO_WRAP)
SAMPLERSTATE(sampler_aniso_mirror, SSLOT_ANISO_MIRROR)
SAMPLERCOMPARISONSTATE(sampler_cmp_depth, SSLOT_CMP_DEPTH)
SAMPLERSTATE(sampler_objectshader, SSLOT_OBJECTSHADER)

static const float		PI = 3.14159265358979323846;
static const float	 SQRT2 = 1.41421356237309504880;//sqrt2

//gauss 
static const float gaussWeight0 = 1.0f;
static const float gaussWeight1 = 0.9f;
static const float gaussWeight2 = 0.55f;
static const float gaussWeight3 = 0.18f;
static const float gaussWeight4 = 0.1f;

static const float gaussNormalization = 1.0f /
(gaussWeight0 + 2.0f * (gaussWeight1 + gaussWeight2 + gaussWeight3 + gaussWeight4));
static const float gaussianWeightsNormalized[9] = {
	gaussWeight4 * gaussNormalization,
	gaussWeight3 * gaussNormalization,
	gaussWeight2 * gaussNormalization,
	gaussWeight1 * gaussNormalization,
	gaussWeight0 * gaussNormalization,
	gaussWeight1 * gaussNormalization,
	gaussWeight2 * gaussNormalization,
	gaussWeight3 * gaussNormalization,
	gaussWeight4 * gaussNormalization,
};

#endif //AS_SHADER_GLOBALS_HF
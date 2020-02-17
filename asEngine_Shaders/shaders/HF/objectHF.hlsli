#ifndef AS_OBJECTSHADER_HF
#define AS_OBJECTSHADER_HF

#if (defined(TILEDFORWARD) || defined(FORWARD)) && !defined(TRANSPARENT)
#define DISABLE_ALPHATEST
#endif

#ifdef TRANSPARENT
#define DISABLE_TRANSPARENT_SHADOWMAP
#endif


#ifdef PLANARREFLECTION
#define DISABLE_ENVMAPS
#endif


#define LIGHTMAP_QUALITY_BICUBIC


#include "../HF/globals.hlsli"
#include "../HF/objectInputLayoutHF.hlsli"
#include "../HF/brdf.hlsli"
#include "../HF/lightingHF.hlsli"

// DEFINITIONS
//////////////////

// These are bound by wiRenderer (based on Material):
TEXTURE2D(texture_basecolormap, float4, TEXSLOT_RENDERER_BASECOLORMAP);			// rgb: baseColor, a: opacity
TEXTURE2D(texture_normalmap, float3, TEXSLOT_RENDERER_NORMALMAP);				// rgb: normal
TEXTURE2D(texture_surfacemap, float4, TEXSLOT_RENDERER_SURFACEMAP);				// r: occlusion, g: roughness, b: metallic, a: reflectance
TEXTURE2D(texture_displacementmap, float, TEXSLOT_RENDERER_DISPLACEMENTMAP);	// r: heightmap
TEXTURE2D(texture_emissivemap, float4, TEXSLOT_RENDERER_EMISSIVEMAP);			// rgba: emissive
TEXTURE2D(texture_occlusionmap, float, TEXSLOT_RENDERER_OCCLUSIONMAP);			// r: occlusion

// These are bound by RenderPath (based on Render Path):
TEXTURE2D(texture_reflection, float4, TEXSLOT_RENDERPATH_REFLECTION);		// rgba: scene color from reflected camera angle
TEXTURE2D(texture_refraction, float4, TEXSLOT_RENDERPATH_REFRACTION);		// rgba: scene color from primary camera angle
TEXTURE2D(texture_waterriples, float4, TEXSLOT_RENDERPATH_WATERRIPPLES);	// rgb: snorm8 water ripple normal map
TEXTURE2D(texture_ssao, float, TEXSLOT_RENDERPATH_SSAO);					// r: screen space ambient occlusion
TEXTURE2D(texture_ssr, float4, TEXSLOT_RENDERPATH_SSR);						// rgb: screen space ray-traced reflections, a: reflection blend based on ray hit or miss


struct PixelInputType_Simple
{
	float4 pos								: SV_POSITION;
	float  clip								: SV_ClipDistance0;
	float4 color							: COLOR;
	float4 uvsets							: UVSETS;
};
struct PixelInputType
{
	float4 pos								: SV_POSITION;
	float  clip								: SV_ClipDistance0;
	float4 color							: COLOR;
	float4 uvsets							: UVSETS;
	float2 atl								: ATLAS;
	float3 nor								: NORMAL;
	float3 pos3D							: WORLDPOSITION;
	float4 pos2D							: SCREENPOSITION;
	float4 pos2DPrev						: SCREENPOSITIONPREV;
};

struct GBUFFEROutputType
{
	float4 g0	: SV_Target0;		// texture_gbuffer0
	float4 g1	: SV_Target1;		// texture_gbuffer1
	float4 g2	: SV_Target2;		// texture_gbuffer2
	float4 diffuse	: SV_Target3;
	float4 specular	: SV_Target4;
};
inline GBUFFEROutputType CreateGbuffer(in float4 color, in Surface surface, in float2 velocity, in Lighting lighting)
{
	LightingPart combined_lighting = CombineLighting(surface, lighting);

	GBUFFEROutputType Out;
	Out.g0 = float4(color.rgb, surface.sss);														/*FORMAT_R8G8B8A8_UNORM*/
	Out.g1 = float4(encodeNormal(surface.N), velocity);												/*FORMAT_R16G16B16A16_FLOAT*/
	Out.g2 = float4(surface.occlusion, surface.roughness, surface.metalness, surface.reflectance);	/*FORMAT_R8G8B8A8_UNORM*/
	Out.diffuse = float4(combined_lighting.diffuse, 1);												/*FORMAT_R11G11B10_FLOAT*/
	Out.specular = float4(combined_lighting.specular, 1);											/*FORMAT_R11G11B10_FLOAT*/
	return Out;
}

struct GBUFFEROutputType_Thin
{
	float4 g0	: SV_Target0;		// texture_gbuffer0
	float4 g1	: SV_Target1;		// texture_gbuffer1
};
inline GBUFFEROutputType_Thin CreateGbuffer_Thin(in float4 color, in Surface surface, in float2 velocity)
{
	GBUFFEROutputType_Thin Out;
	Out.g0 = color;																		/*FORMAT_R11G11B10_FLOAT*/
	Out.g1 = float4(encodeNormal(surface.N), velocity);									/*FORMAT_R16G16B16A16_FLOAT*/
	return Out;
}


// METHODS
////////////

inline void ApplyEmissive(in Surface surface, inout Lighting lighting)
{
	lighting.direct.specular += surface.emissiveColor.rgb * surface.emissiveColor.a;
}

inline void LightMapping(in float2 ATLAS, inout Lighting lighting)
{
	if (any(ATLAS))
	{
#ifdef LIGHTMAP_QUALITY_BICUBIC
		float4 lightmap = SampleTextureCatmullRom(texture_globallightmap, ATLAS);
#else
		float4 lightmap = texture_globallightmap.SampleLevel(sampler_linear_clamp, ATLAS, 0);
#endif // LIGHTMAP_QUALITY_BICUBIC
		lighting.indirect.diffuse = lerp(lighting.indirect.diffuse, lightmap.rgb, lightmap.a);
	}
}

inline void NormalMapping(in float2 UV, in float3 V, inout float3 N, in float3x3 TBN, inout float3 bumpColor)
{
	float3 normalMap = texture_normalmap.Sample(sampler_objectshader, UV).rgb;
	bumpColor = normalMap.rgb * 2 - 1;
	bumpColor.g *= g_xMaterial.normalMapFlip;
	N = normalize(lerp(N, mul(bumpColor, TBN), g_xMaterial.normalMapStrength));
	bumpColor *= g_xMaterial.normalMapStrength;
}

inline float3 PlanarReflection(in Surface surface, in float2 bumpColor)
{
	float4 reflectionUV = mul(g_xFrame_MainCamera_ReflVP, float4(surface.P, 1));
	reflectionUV.xy /= reflectionUV.w;
	reflectionUV.xy = reflectionUV.xy * float2(0.5f, -0.5f) + 0.5f;
	return texture_reflection.SampleLevel(sampler_linear_clamp, reflectionUV.xy + bumpColor*g_xMaterial.normalMapStrength, 0).rgb;
}

#define NUM_PARALLAX_OCCLUSION_STEPS 32
inline void ParallaxOcclusionMapping(inout float4 uvsets, in float3 V, in float3x3 TBN)
{
	V = mul(TBN, V);
	float layerHeight = 1.0 / NUM_PARALLAX_OCCLUSION_STEPS;
	float curLayerHeight = 0;
	float2 dtex = g_xMaterial.parallaxOcclusionMapping * V.xy / NUM_PARALLAX_OCCLUSION_STEPS;
	float2 originalTextureCoords = g_xMaterial.uvset_displacementMap == 0 ? uvsets.xy : uvsets.zw;
	float2 currentTextureCoords = originalTextureCoords;
	float2 derivX = ddx_coarse(currentTextureCoords);
	float2 derivY = ddy_coarse(currentTextureCoords);
	float heightFromTexture = 1 - texture_displacementmap.SampleGrad(sampler_linear_wrap, currentTextureCoords, derivX, derivY).r;
	uint iter = 0;
	[loop]
	while (heightFromTexture > curLayerHeight && iter < NUM_PARALLAX_OCCLUSION_STEPS)
	{
		curLayerHeight += layerHeight;
		currentTextureCoords -= dtex;
		heightFromTexture = 1 - texture_displacementmap.SampleGrad(sampler_linear_wrap, currentTextureCoords, derivX, derivY).r;
		iter++;
	}
	float2 prevTCoords = currentTextureCoords + dtex;
	float nextH = heightFromTexture - curLayerHeight;
	float prevH = 1 - texture_displacementmap.SampleGrad(sampler_linear_wrap, prevTCoords, derivX, derivY).r - curLayerHeight + layerHeight;
	float weight = nextH / (nextH - prevH);
	float2 finalTextureCoords = prevTCoords * weight + currentTextureCoords * (1.0 - weight);
	float2 difference = finalTextureCoords - originalTextureCoords;
	uvsets += difference.xyxy;
}

inline void Refraction(in float2 ScreenCoord, inout Surface surface, inout float4 color, inout Lighting lighting)
{
	if (g_xMaterial.refractionIndex > 0)
	{
		float2 size;
		float mipLevels;
		texture_refraction.GetDimensions(0, size.x, size.y, mipLevels);
		const float sampled_mip = ((g_xFrame_Options & OPTION_BIT_ADVANCEDREFRACTIONS_ENABLED) ? surface.roughness * mipLevels : 0);
		const float2 normal2D = mul((float3x3)g_xCamera_View, surface.N.xyz).xy;
		float2 perturbatedRefrTexCoords = ScreenCoord.xy + normal2D * g_xMaterial.refractionIndex;
		float4 refractiveColor = texture_refraction.SampleLevel(sampler_linear_clamp, perturbatedRefrTexCoords, sampled_mip);
		surface.albedo.rgb = lerp(refractiveColor.rgb, surface.albedo.rgb, color.a);
		lighting.direct.diffuse = lerp(1, lighting.direct.diffuse, color.a);
		color.a = 1;
	}
}

inline void ForwardLighting(inout Surface surface, inout Lighting lighting)
{
#ifndef DISABLE_DECALS
	[branch]
	if (xForwardDecalMask != 0)
	{
		// decals are enabled, loop through them first:
		float4 decalAccumulation = 0;
		const float3 P_dx = ddx_coarse(surface.P);
		const float3 P_dy = ddy_coarse(surface.P);

		uint bucket_bits = xForwardDecalMask;

		[loop]
		while (bucket_bits != 0)
		{
			// Retrieve global entity index from local bucket, then remove bit from local bucket:
			const uint bucket_bit_index = firstbitlow(bucket_bits);
			const uint entity_index = bucket_bit_index;
			bucket_bits ^= 1 << bucket_bit_index;

			[branch]
			if (decalAccumulation.a < 1)
			{
				ShaderEntity decal = EntityArray[g_xFrame_DecalArrayOffset + entity_index];

				const float4x4 decalProjection = MatrixArray[decal.userdata];
				const float3 clipSpacePos = mul(decalProjection, float4(surface.P, 1)).xyz;
				const float3 uvw = clipSpacePos.xyz*float3(0.5f, -0.5f, 0.5f) + 0.5f;
				[branch]
				if (is_saturated(uvw))
				{
					// mipmapping needs to be performed by hand:
					const float2 decalDX = mul(P_dx, (float3x3)decalProjection).xy * decal.texMulAdd.xy;
					const float2 decalDY = mul(P_dy, (float3x3)decalProjection).xy * decal.texMulAdd.xy;
					float4 decalColor = texture_decalatlas.SampleGrad(sampler_linear_clamp, uvw.xy*decal.texMulAdd.xy + decal.texMulAdd.zw, decalDX, decalDY);
					// blend out if close to cube Z:
					float edgeBlend = 1 - pow(saturate(abs(clipSpacePos.z)), 8);
					decalColor.a *= edgeBlend;
					decalColor *= decal.GetColor();
					// apply emissive:
					lighting.direct.specular += max(0, decalColor.rgb * decal.GetEmissive() * edgeBlend);
					// perform manual blending of decals:
					//  NOTE: they are sorted top-to-bottom, but blending is performed bottom-to-top
					decalAccumulation.rgb = (1 - decalAccumulation.a) * (decalColor.a*decalColor.rgb) + decalAccumulation.rgb;
					decalAccumulation.a = decalColor.a + (1 - decalColor.a) * decalAccumulation.a;
					[branch]
					if (decalAccumulation.a >= 1.0)
					{
						// force exit:
						break;
					}
				}
			}
			else
			{
				// force exit:
				break;
			}

		}

		surface.albedo.rgb = lerp(surface.albedo.rgb, decalAccumulation.rgb, decalAccumulation.a);
	}
#endif // DISABLE_DECALS


#ifndef DISABLE_ENVMAPS
	// Apply environment maps:
	float4 envmapAccumulation = 0;
	const float envMapMIP = surface.roughness * g_xFrame_EnvProbeMipCount;

#ifndef ENVMAPRENDERING
#ifndef DISABLE_LOCALENVPMAPS
	[branch]
	if (xForwardEnvProbeMask != 0)
	{
		uint bucket_bits = xForwardEnvProbeMask;

		[loop]
		while (bucket_bits != 0)
		{
			// Retrieve global entity index from local bucket, then remove bit from local bucket:
			const uint bucket_bit_index = firstbitlow(bucket_bits);
			const uint entity_index = bucket_bit_index;
			bucket_bits ^= 1 << bucket_bit_index;

			[branch]
			if (envmapAccumulation.a < 1)
			{
				ShaderEntity probe = EntityArray[g_xFrame_EnvProbeArrayOffset + entity_index];

				const float4x4 probeProjection = MatrixArray[probe.userdata];
				const float3 clipSpacePos = mul(probeProjection, float4(surface.P, 1)).xyz;
				const float3 uvw = clipSpacePos.xyz*float3(0.5f, -0.5f, 0.5f) + 0.5f;
				[branch]
				if (is_saturated(uvw))
				{
					const float4 envmapColor = EnvironmentReflection_Local(surface, probe, probeProjection, clipSpacePos, envMapMIP);
					// perform manual blending of probes:
					//  NOTE: they are sorted top-to-bottom, but blending is performed bottom-to-top
					envmapAccumulation.rgb = (1 - envmapAccumulation.a) * (envmapColor.a * envmapColor.rgb) + envmapAccumulation.rgb;
					envmapAccumulation.a = envmapColor.a + (1 - envmapColor.a) * envmapAccumulation.a;
					[branch]
					if (envmapAccumulation.a >= 1.0)
					{
						// force exit:
						break;
					}
				}
			}
			else
			{
				// force exit:
				break;
			}

		}
	}
#endif // DISABLE_LOCALENVPMAPS
#endif //ENVMAPRENDERING

	// Apply global envmap where there is no local envmap information:
	[branch]
	if (envmapAccumulation.a < 0.99f)
	{
		envmapAccumulation.rgb = lerp(EnvironmentReflection_Global(surface, envMapMIP), envmapAccumulation.rgb, envmapAccumulation.a);
	}
	lighting.indirect.specular += max(0, envmapAccumulation.rgb);
#endif // DISABLE_ENVMAPS

#ifndef DISABLE_VOXELGI
	VoxelGI(surface, lighting);
#endif //DISABLE_VOXELGI

	[branch]
	if (any(xForwardLightMask))
	{
		// Loop through light buckets for the draw call:
		const uint first_item = 0;
		const uint last_item = first_item + g_xFrame_LightArrayCount - 1;
		const uint first_bucket = first_item / 32;
		const uint last_bucket = min(last_item / 32, 1); // only 2 buckets max (uint2) for forward pass!
		[loop]
		for (uint bucket = first_bucket; bucket <= last_bucket; ++bucket)
		{
			uint bucket_bits = xForwardLightMask[bucket];

			[loop]
			while (bucket_bits != 0)
			{
				// Retrieve global entity index from local bucket, then remove bit from local bucket:
				const uint bucket_bit_index = firstbitlow(bucket_bits);
				const uint entity_index = bucket * 32 + bucket_bit_index;
				bucket_bits ^= 1 << bucket_bit_index;

				ShaderEntity light = EntityArray[g_xFrame_LightArrayOffset + entity_index];

				if (light.GetFlags() & ENTITY_FLAG_LIGHT_STATIC)
				{
					continue; // static lights will be skipped (they are used in lightmap baking)
				}

				switch (light.GetType())
				{
				case ENTITY_TYPE_DIRECTIONALLIGHT:
				{
					DirectionalLight(light, surface, lighting);
				}
				break;
				case ENTITY_TYPE_POINTLIGHT:
				{
					PointLight(light, surface, lighting);
				}
				break;
				case ENTITY_TYPE_SPOTLIGHT:
				{
					SpotLight(light, surface, lighting);
				}
				break;
				case ENTITY_TYPE_SPHERELIGHT:
				{
					SphereLight(light, surface, lighting);
				}
				break;
				case ENTITY_TYPE_DISCLIGHT:
				{
					DiscLight(light, surface, lighting);
				}
				break;
				case ENTITY_TYPE_RECTANGLELIGHT:
				{
					RectangleLight(light, surface, lighting);
				}
				break;
				case ENTITY_TYPE_TUBELIGHT:
				{
					TubeLight(light, surface, lighting);
				}
				break;
				}
			}
		}
	}

}

inline void TiledLighting(in float2 pixel, inout Surface surface, inout Lighting lighting)
{
	const uint2 tileIndex = uint2(floor(pixel / TILED_CULLING_BLOCKSIZE));
	const uint flatTileIndex = flatten2D(tileIndex, g_xFrame_EntityCullingTileCount.xy) * SHADER_ENTITY_TILE_BUCKET_COUNT;


#ifndef DISABLE_DECALS
	[branch]
	if (g_xFrame_DecalArrayCount > 0)
	{
		// decals are enabled, loop through them first:
		float4 decalAccumulation = 0;
		const float3 P_dx = ddx_coarse(surface.P);
		const float3 P_dy = ddy_coarse(surface.P);

		// Loop through decal buckets in the tile:
		const uint first_item = g_xFrame_DecalArrayOffset;
		const uint last_item = first_item + g_xFrame_DecalArrayCount - 1;
		const uint first_bucket = first_item / 32;
		const uint last_bucket = min(last_item / 32, max(0, SHADER_ENTITY_TILE_BUCKET_COUNT - 1));
		[loop]
		for (uint bucket = first_bucket; bucket <= last_bucket; ++bucket)
		{
			uint bucket_bits = EntityTiles[flatTileIndex + bucket];

			// This is the wave scalarizer from Improved Culling - Siggraph 2017 [Drobot]:
			bucket_bits = WaveReadLaneFirst(WaveActiveBitOr(bucket_bits));

			[loop]
			while (bucket_bits != 0)
			{
				// Retrieve global entity index from local bucket, then remove bit from local bucket:
				const uint bucket_bit_index = firstbitlow(bucket_bits);
				const uint entity_index = bucket * 32 + bucket_bit_index;
				bucket_bits ^= 1 << bucket_bit_index;

				[branch]
				if (entity_index >= first_item && entity_index <= last_item && decalAccumulation.a < 1)
				{
					ShaderEntity decal = EntityArray[entity_index];

					const float4x4 decalProjection = MatrixArray[decal.userdata];
					const float3 clipSpacePos = mul(decalProjection, float4(surface.P, 1)).xyz;
					const float3 uvw = clipSpacePos.xyz*float3(0.5f, -0.5f, 0.5f) + 0.5f;
					[branch]
					if (is_saturated(uvw))
					{
						// mipmapping needs to be performed by hand:
						const float2 decalDX = mul(P_dx, (float3x3)decalProjection).xy * decal.texMulAdd.xy;
						const float2 decalDY = mul(P_dy, (float3x3)decalProjection).xy * decal.texMulAdd.xy;
						float4 decalColor = texture_decalatlas.SampleGrad(sampler_linear_clamp, uvw.xy*decal.texMulAdd.xy + decal.texMulAdd.zw, decalDX, decalDY);
						// blend out if close to cube Z:
						float edgeBlend = 1 - pow(saturate(abs(clipSpacePos.z)), 8);
						decalColor.a *= edgeBlend;
						decalColor *= decal.GetColor();
						// apply emissive:
						lighting.direct.specular += max(0, decalColor.rgb * decal.GetEmissive() * edgeBlend);
						// perform manual blending of decals:
						//  NOTE: they are sorted top-to-bottom, but blending is performed bottom-to-top
						decalAccumulation.rgb = (1 - decalAccumulation.a) * (decalColor.a*decalColor.rgb) + decalAccumulation.rgb;
						decalAccumulation.a = decalColor.a + (1 - decalColor.a) * decalAccumulation.a;
						[branch]
						if (decalAccumulation.a >= 1.0)
						{
							// force exit:
							bucket = SHADER_ENTITY_TILE_BUCKET_COUNT;
							break;
						}
					}
				}
				else if (entity_index > last_item)
				{
					// force exit:
					bucket = SHADER_ENTITY_TILE_BUCKET_COUNT;
					break;
				}

			}
		}

		surface.albedo.rgb = lerp(surface.albedo.rgb, decalAccumulation.rgb, decalAccumulation.a);
	}
#endif // DISABLE_DECALS


#ifndef DISABLE_ENVMAPS
	// Apply environment maps:
	float4 envmapAccumulation = 0;
	const float envMapMIP = surface.roughness * g_xFrame_EnvProbeMipCount;

#ifndef DISABLE_LOCALENVPMAPS
	[branch]
	if (g_xFrame_EnvProbeArrayCount > 0)
	{
		// Loop through envmap buckets in the tile:
		const uint first_item = g_xFrame_EnvProbeArrayOffset;
		const uint last_item = first_item + g_xFrame_EnvProbeArrayCount - 1;
		const uint first_bucket = first_item / 32;
		const uint last_bucket = min(last_item / 32, max(0, SHADER_ENTITY_TILE_BUCKET_COUNT - 1));
		[loop]
		for (uint bucket = first_bucket; bucket <= last_bucket; ++bucket)
		{
			uint bucket_bits = EntityTiles[flatTileIndex + bucket];

			// Bucket scalarizer - Siggraph 2017 - Improved Culling [Michal Drobot]:
			bucket_bits = WaveReadLaneFirst(WaveActiveBitOr(bucket_bits));

			[loop]
			while (bucket_bits != 0)
			{
				// Retrieve global entity index from local bucket, then remove bit from local bucket:
				const uint bucket_bit_index = firstbitlow(bucket_bits);
				const uint entity_index = bucket * 32 + bucket_bit_index;
				bucket_bits ^= 1 << bucket_bit_index;

				[branch]
				if (entity_index >= first_item && entity_index <= last_item && envmapAccumulation.a < 1)
				{
					ShaderEntity probe = EntityArray[entity_index];

					const float4x4 probeProjection = MatrixArray[probe.userdata];
					const float3 clipSpacePos = mul(probeProjection, float4(surface.P, 1)).xyz;
					const float3 uvw = clipSpacePos.xyz*float3(0.5f, -0.5f, 0.5f) + 0.5f;
					[branch]
					if (is_saturated(uvw))
					{
						const float4 envmapColor = EnvironmentReflection_Local(surface, probe, probeProjection, clipSpacePos, envMapMIP);
						// perform manual blending of probes:
						//  NOTE: they are sorted top-to-bottom, but blending is performed bottom-to-top
						envmapAccumulation.rgb = (1 - envmapAccumulation.a) * (envmapColor.a * envmapColor.rgb) + envmapAccumulation.rgb;
						envmapAccumulation.a = envmapColor.a + (1 - envmapColor.a) * envmapAccumulation.a;
						[branch]
						if (envmapAccumulation.a >= 1.0)
						{
							// force exit:
							bucket = SHADER_ENTITY_TILE_BUCKET_COUNT;
							break;
						}
					}
				}
				else if (entity_index > last_item)
				{
					// force exit:
					bucket = SHADER_ENTITY_TILE_BUCKET_COUNT;
					break;
				}

			}
		}
	}
#endif // DISABLE_LOCALENVPMAPS
	
	// Apply global envmap where there is no local envmap information:
	[branch]
	if (envmapAccumulation.a < 0.99f)
	{
		envmapAccumulation.rgb = lerp(EnvironmentReflection_Global(surface, envMapMIP), envmapAccumulation.rgb, envmapAccumulation.a);
	}
	lighting.indirect.specular += max(0, envmapAccumulation.rgb);
#endif // DISABLE_ENVMAPS

#ifndef DISABLE_VOXELGI
	VoxelGI(surface, lighting);
#endif //DISABLE_VOXELGI

	[branch]
	if (g_xFrame_LightArrayCount > 0)
	{
		// Loop through light buckets in the tile:
		const uint first_item = g_xFrame_LightArrayOffset;
		const uint last_item = first_item + g_xFrame_LightArrayCount - 1;
		const uint first_bucket = first_item / 32;
		const uint last_bucket = min(last_item / 32, max(0, SHADER_ENTITY_TILE_BUCKET_COUNT - 1));
		[loop]
		for (uint bucket = first_bucket; bucket <= last_bucket; ++bucket)
		{
			uint bucket_bits = EntityTiles[flatTileIndex + bucket];

			// Bucket scalarizer - Siggraph 2017 - Improved Culling [Michal Drobot]:
			bucket_bits = WaveReadLaneFirst(WaveActiveBitOr(bucket_bits));

			[loop]
			while (bucket_bits != 0)
			{
				// Retrieve global entity index from local bucket, then remove bit from local bucket:
				const uint bucket_bit_index = firstbitlow(bucket_bits);
				const uint entity_index = bucket * 32 + bucket_bit_index;
				bucket_bits ^= 1 << bucket_bit_index;

				// Check if it is a light and process:
				[branch]
				if (entity_index >= first_item && entity_index <= last_item)
				{
					ShaderEntity light = EntityArray[entity_index];

					if (light.GetFlags() & ENTITY_FLAG_LIGHT_STATIC)
					{
						continue; // static lights will be skipped (they are used in lightmap baking)
					}

					switch (light.GetType())
					{
					case ENTITY_TYPE_DIRECTIONALLIGHT:
					{
						DirectionalLight(light, surface, lighting);
					}
					break;
					case ENTITY_TYPE_POINTLIGHT:
					{
						PointLight(light, surface, lighting);
					}
					break;
					case ENTITY_TYPE_SPOTLIGHT:
					{
						SpotLight(light, surface, lighting);
					}
					break;
					case ENTITY_TYPE_SPHERELIGHT:
					{
						SphereLight(light, surface, lighting);
					}
					break;
					case ENTITY_TYPE_DISCLIGHT:
					{
						DiscLight(light, surface, lighting);
					}
					break;
					case ENTITY_TYPE_RECTANGLELIGHT:
					{
						RectangleLight(light, surface, lighting);
					}
					break;
					case ENTITY_TYPE_TUBELIGHT:
					{
						TubeLight(light, surface, lighting);
					}
					break;
					}
				}
				else if (entity_index > last_item)
				{
					// force exit:
					bucket = SHADER_ENTITY_TILE_BUCKET_COUNT;
					break;
				}

			}
		}
	}

}

inline void ApplyLighting(in Surface surface, in Lighting lighting, inout float4 color)
{
	LightingPart combined_lighting = CombineLighting(surface, lighting);
	color.rgb = surface.albedo * combined_lighting.diffuse + combined_lighting.specular;
}

inline void ApplyFog(in float dist, inout float4 color)
{
	color.rgb = lerp(color.rgb, GetHorizonColor(), GetFog(dist));
}


// OBJECT SHADER PROTOTYPE
///////////////////////////

#if defined(COMPILE_OBJECTSHADER_PS)

// Possible switches:
//	ALPHATESTONLY		-	assemble object shader for depth only rendering + alpha test
//	TEXTUREONLY			-	assemble object shader for rendering only with base textures, no lighting
//	DEFERRED			-	assemble object shader for deferred rendering
//	FORWARD				-	assemble object shader for forward rendering
//	TILEDFORWARD		-	assemble object shader for tiled forward rendering
//	TRANSPARENT			-	assemble object shader for forward or tile forward transparent rendering
//	ENVMAPRENDERING		-	modify object shader for envmap rendering
//	NORMALMAP			-	include normal mapping computation
//	PLANARREFLECTION	-	include planar reflection sampling
//	POM					-	include parallax occlusion mapping computation
//	WATER				-	include specialized water shader code
//	BLACKOUT			-	include specialized blackout shader code

#if defined(ALPHATESTONLY) || defined(TEXTUREONLY)
#define SIMPLE_INPUT
#endif // APLHATESTONLY

#ifdef SIMPLE_INPUT
#define PIXELINPUT PixelInputType_Simple
#else
#define PIXELINPUT PixelInputType
#endif // SIMPLE_INPUT

#ifdef PLANARREFLECTION
#define DISABLE_ENVMAPS
#define DISABLE_VOXELGI
#endif // PLANARREFLECTION


// entry point:
#if defined(ALPHATESTONLY)
void main(PIXELINPUT input)
#elif defined(TEXTUREONLY)
float4 main(PIXELINPUT input) : SV_TARGET
#elif defined(TRANSPARENT)
float4 main(PIXELINPUT input) : SV_TARGET
#elif defined(ENVMAPRENDERING)
float4 main(PSIn_EnvmapRendering input) : SV_TARGET
#elif defined(DEFERRED)
GBUFFEROutputType main(PIXELINPUT input)
#elif defined(FORWARD)
GBUFFEROutputType_Thin main(PIXELINPUT input)
#elif defined(TILEDFORWARD)
[earlydepthstencil]
GBUFFEROutputType_Thin main(PIXELINPUT input)
#endif // ALPHATESTONLY



// shader base:
{
	float2 pixel = input.pos.xy;

#ifndef DISABLE_ALPHATEST
#ifndef TRANSPARENT
#ifndef ENVMAPRENDERING
	// apply dithering:
	clip(dither(pixel + GetTemporalAASampleRotation()) - (1 - input.color.a));
#endif // DISABLE_ALPHATEST
#endif // TRANSPARENT
#endif // ENVMAPRENDERING

	Surface surface;

#ifndef SIMPLE_INPUT
	surface.P = input.pos3D;
	surface.V = g_xCamera_CamPos - surface.P;
	float dist = length(surface.V);
	surface.V /= dist;
	surface.N = normalize(input.nor);

	float3x3 TBN = compute_tangent_frame(surface.N, surface.P, input.uvsets.xy);
#endif // SIMPLE_INPUT

#ifdef POM
	ParallaxOcclusionMapping(input.uvsets, surface.V, TBN);
#endif // POM

	float4 color;
	[branch]
	if (g_xMaterial.uvset_baseColorMap >= 0)
	{
		const float2 UV_baseColorMap = g_xMaterial.uvset_baseColorMap == 0 ? input.uvsets.xy : input.uvsets.zw;
		color = texture_basecolormap.Sample(sampler_objectshader, UV_baseColorMap);
		color.rgb = DEGAMMA(color.rgb);
	}
	else
	{
		color = 1;
	}
	color *= input.color;
	ALPHATEST(color.a);

#ifndef SIMPLE_INPUT
	float3 bumpColor = 0;
	float opacity = color.a;
	float depth = input.pos.z;
#ifndef ENVMAPRENDERING
	const float lineardepth = input.pos2D.w;
	input.pos2D.xy /= input.pos2D.w;
	input.pos2DPrev.xy /= input.pos2DPrev.w;

	const float2 ScreenCoord = input.pos2D.xy * float2(0.5f, -0.5f) + 0.5f;
	const float2 velocity = ((input.pos2DPrev.xy - g_xFrame_TemporalAAJitterPrev) - (input.pos2D.xy - g_xFrame_TemporalAAJitter)) * float2(0.5f, -0.5f);
	const float2 ReprojectedScreenCoord = ScreenCoord + velocity;

	const float sampled_lineardepth = texture_lineardepth.SampleLevel(sampler_point_clamp, ScreenCoord, 0) * g_xCamera_ZFarP;
	const float depth_difference = sampled_lineardepth - lineardepth;
#endif // ENVMAPRENDERING
#endif // SIMPLE_INPUT

#ifdef NORMALMAP
	const float2 UV_normalMap = g_xMaterial.uvset_normalMap == 0 ? input.uvsets.xy : input.uvsets.zw;
	NormalMapping(UV_normalMap, surface.P, surface.N, TBN, bumpColor);
#endif // NORMALMAP

	// Surface map:
	float4 surface_occlusion_roughness_metallic_reflectance;
	[branch]
	if (g_xMaterial.uvset_surfaceMap >= 0)
	{
		const float2 UV_surfaceMap = g_xMaterial.uvset_surfaceMap == 0 ? input.uvsets.xy : input.uvsets.zw;
		surface_occlusion_roughness_metallic_reflectance = texture_surfacemap.Sample(sampler_objectshader, UV_surfaceMap);
		if (g_xMaterial.occlusion_primary == 0)
		{
			surface_occlusion_roughness_metallic_reflectance.r = 1;
		}
		if (g_xMaterial.specularGlossinessWorkflow)
		{
			ConvertToSpecularGlossiness(surface_occlusion_roughness_metallic_reflectance);
		}
	}
	else
	{
		surface_occlusion_roughness_metallic_reflectance = 1;
	}

	// Emissive map:
	float4 emissiveColor = g_xMaterial.emissiveColor;
	[branch]
	if (emissiveColor.a > 0 && g_xMaterial.uvset_emissiveMap >= 0)
	{
		const float2 UV_emissiveMap = g_xMaterial.uvset_emissiveMap == 0 ? input.uvsets.xy : input.uvsets.zw;
		float4 emissiveMap = texture_emissivemap.Sample(sampler_objectshader, UV_emissiveMap);
		emissiveMap.rgb = DEGAMMA(emissiveMap.rgb);
		emissiveColor *= emissiveMap;
	}

	// Secondary occlusion map:
	[branch]
	if (g_xMaterial.occlusion_secondary && g_xMaterial.uvset_occlusionMap >= 0)
	{
		const float2 UV_occlusionMap = g_xMaterial.uvset_occlusionMap == 0 ? input.uvsets.xy : input.uvsets.zw;
		surface_occlusion_roughness_metallic_reflectance.r *= texture_occlusionmap.Sample(sampler_objectshader, UV_occlusionMap).r;
	}

#ifndef DEFERRED
#ifndef SIMPLE_INPUT
#ifndef ENVMAPRENDERING
#ifndef TRANSPARENT
	const float ssao = texture_ssao.SampleLevel(sampler_linear_clamp, ScreenCoord, 0).r;
	surface_occlusion_roughness_metallic_reflectance.r *= ssao;
#endif // TRANSPARENT
#endif // ENVMAPRENDERING
#endif // SIMPLE_INPUT
#endif


	surface = CreateSurface(
		surface.P, 
		surface.N, 
		surface.V, 
		color,
		surface_occlusion_roughness_metallic_reflectance.r,
		g_xMaterial.roughness * surface_occlusion_roughness_metallic_reflectance.g,
		g_xMaterial.metalness * surface_occlusion_roughness_metallic_reflectance.b,
		g_xMaterial.reflectance * surface_occlusion_roughness_metallic_reflectance.a,
		emissiveColor,
		g_xMaterial.subsurfaceScattering
	);

	Lighting lighting = CreateLighting(0, 0, GetAmbient(surface.N), 0);

#ifndef SIMPLE_INPUT


#ifdef WATER
	color.a = 1;

	//NORMALMAP
	float2 bumpColor0 = 0;
	float2 bumpColor1 = 0;
	float2 bumpColor2 = 0;
	[branch]
	if (g_xMaterial.uvset_normalMap >= 0)
	{
		const float2 UV_normalMap = g_xMaterial.uvset_normalMap == 0 ? input.uvsets.xy : input.uvsets.zw;
		bumpColor0 = 2.0f * texture_normalmap.Sample(sampler_objectshader, UV_normalMap - g_xMaterial.texMulAdd.ww).rg - 1.0f;
		bumpColor1 = 2.0f * texture_normalmap.Sample(sampler_objectshader, UV_normalMap + g_xMaterial.texMulAdd.zw).rg - 1.0f;
	}
	bumpColor2 = texture_waterriples.SampleLevel(sampler_objectshader, ScreenCoord, 0).rg;
	bumpColor = float3(bumpColor0 + bumpColor1 + bumpColor2, 1)  * g_xMaterial.refractionIndex;
	surface.N = normalize(lerp(surface.N, mul(normalize(bumpColor), TBN), g_xMaterial.normalMapStrength));
	bumpColor *= g_xMaterial.normalMapStrength;

	//REFLECTION
	float4 reflectionUV = mul(g_xFrame_MainCamera_ReflVP, float4(surface.P, 1));
	reflectionUV.xy /= reflectionUV.w;
	reflectionUV.xy = reflectionUV.xy * float2(0.5f, -0.5f) + 0.5f;
	float4 reflectiveColor = texture_reflection.SampleLevel(sampler_linear_mirror, reflectionUV.xy + bumpColor.rg, 0);


	//REFRACTION 
	float2 perturbatedRefrTexCoords = ScreenCoord.xy + bumpColor.rg;
	float3 refractiveColor = texture_refraction.SampleLevel(sampler_linear_mirror, perturbatedRefrTexCoords, 0).rgb;
	float mod = saturate(0.05f * depth_difference);
	refractiveColor = lerp(refractiveColor, surface.baseColor.rgb, mod).rgb;

	//FRESNEL TERM
	float3 fresnelTerm = F_Fresnel(surface.f0, surface.NdotV);
	surface.albedo.rgb = lerp(refractiveColor, reflectiveColor.rgb, fresnelTerm);
#endif // WATER



	LightMapping(input.atl, lighting);

	ApplyEmissive(surface, lighting);

#ifdef PLANARREFLECTION
	lighting.indirect.specular += PlanarReflection(surface, bumpColor.rg);
#endif



#ifndef DEFERRED


#ifdef FORWARD
	ForwardLighting(surface, lighting);
#endif // FORWARD

#ifdef TILEDFORWARD
	TiledLighting(pixel, surface, lighting);
#endif // TILEDFORWARD


#ifndef WATER
#ifndef ENVMAPRENDERING
#ifdef TRANSPARENT
	Refraction(ScreenCoord, surface, color, lighting);
#else
	float4 ssr = texture_ssr.SampleLevel(sampler_linear_clamp, ReprojectedScreenCoord, surface.roughness * 5);
	lighting.indirect.specular = lerp(lighting.indirect.specular, ssr.rgb, ssr.a);
#endif // TRANSPARENT
#endif // ENVMAPRENDERING
#endif // WATER

	ApplyLighting(surface, lighting, color);

#ifdef WATER
	// SOFT EDGE
	float fade = saturate(0.3 * depth_difference);
	color.a *= fade;
#endif // WATER

	ApplyFog(dist, color);


#endif // DEFERRED

#endif // SIMPLE_INPUT


#ifdef TEXTUREONLY
	color.rgb += surface.emissiveColor.rgb * surface.emissiveColor.a;
#endif // TEXTUREONLY


#ifdef BLACKOUT
	color = float4(0, 0, 0, 1);
#endif

	color = max(0, color);


	// end point:
#if defined(TRANSPARENT) || defined(TEXTUREONLY) || defined(ENVMAPRENDERING)
	return color;
#else
#if defined(DEFERRED)	
	return CreateGbuffer(color, surface, velocity, lighting);
#elif defined(FORWARD) || defined(TILEDFORWARD)
	return CreateGbuffer_Thin(color, surface, velocity);
#endif // DEFERRED
#endif // TRANSPARENT

}


#endif // COMPILE_OBJECTSHADER_PS



#endif // AS_OBJECTSHADER_HF


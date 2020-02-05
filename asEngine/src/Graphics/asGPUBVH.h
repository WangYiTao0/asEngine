#pragma once
//Bounding volume hierarchy
//Thinking Parallel, Part III: Tree Construction on the GPU

#include "CommonInclude.h"
#include "Graphics\API\asGraphicsDevice.h"
#include "System\asScene_Dec1.h"
#include "Helpers\asRectPacker.h"
#include "Graphics\GPUMapping\ShaderInterop_Renderer.h"

#include <vector>
#include <unordered_map>
#include <unordered_set>

class asGPUBVH
{
private:
	//scene BVH intersection resources
	asGraphics::GPUBuffer bvhNodeBuffer;
	asGraphics::GPUBuffer bvhParentBuffer;
	asGraphics::GPUBuffer bvhFlagBuffer;
	asGraphics::GPUBuffer primitiveCounterBuffer;
	asGraphics::GPUBuffer primitiveIDBuffer;
	asGraphics::GPUBuffer primitiveBuffer;
	asGraphics::GPUBuffer primitiveDataBuffer;
	asGraphics::GPUBuffer primitiveMortonBuffer;
	uint32_t primitiveCapacity = 0;
	uint32_t primitiveCount = 0;

	// Scene material resources:
	asGraphics::GPUBuffer globalMaterialBuffer;
	asGraphics::Texture globalMaterialAtlas;
	std::vector<ShaderMaterial> materialArray;
	std::unordered_map<const asGraphics::Texture*, asRectPacker::rect_xywh> storedTextures;
	std::unordered_set<const asGraphics::Texture*> sceneTextures;
	void UpdateGlobalMaterialResources(const asScene::Scene& scene, asGraphics::CommandList cmd);

public:
	void Build(const asScene::Scene& scene, asGraphics::CommandList cmd);
	void Bind(asGraphics::SHADERSTAGE stage, asGraphics::CommandList cmd) const;

	static void LoadShaders();

};
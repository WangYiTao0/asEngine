#pragma once
//Bounding volume hierarchy
//Thinking Parallel, Part III: Tree Construction on the GPU


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
	as::asGraphics::GPUBuffer bvhNodeBuffer;
	as::asGraphics::GPUBuffer bvhParentBuffer;
	as::asGraphics::GPUBuffer bvhFlagBuffer;
	as::asGraphics::GPUBuffer primitiveCounterBuffer;
	as::asGraphics::GPUBuffer primitiveIDBuffer;
	as::asGraphics::GPUBuffer primitiveBuffer;
	as::asGraphics::GPUBuffer primitiveDataBuffer;
	as::asGraphics::GPUBuffer primitiveMortonBuffer;
	uint32_t primitiveCapacity = 0;
	uint32_t primitiveCount = 0;

	// Scene material resources:
	as::asGraphics::GPUBuffer globalMaterialBuffer;
	as::asGraphics::Texture globalMaterialAtlas;
	std::vector<ShaderMaterial> materialArray;
	std::unordered_map<const as::asGraphics::Texture*, as::asRectPacker::rect_xywh> storedTextures;
	std::unordered_set<const as::asGraphics::Texture*> sceneTextures;
	void UpdateGlobalMaterialResources(const as::asScene::Scene& scene, as::asGraphics::CommandList cmd);

public:
	void Build(const as::asScene::Scene& scene, as::asGraphics::CommandList cmd);
	void Bind(as::asGraphics::SHADERSTAGE stage, as::asGraphics::CommandList cmd) const;

	static void LoadShaders();

};
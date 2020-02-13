#pragma once
#include <string>
namespace as
{
	struct asScene::Scene;

	void ImportModel_OBJ(const std::string& fileName, asScene::Scene& scene);
	void ImportModel_GLTF(const std::string& fileName, asScene::Scene& scene);
}


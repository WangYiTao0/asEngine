#pragma once
#include "System\asECS.h"
#include "System\asScene_Dec1.h"
#include "System\asJobSystem.h"

namespace as
{
	namespace asPhysicsEngine
	{
		void Initialize();

		bool IsEnabled();
		void SetEnable(bool value);
		void RunPhysicsUpdateSystem(
			asJobSystem::context& ctx,
			const asScene::WeatherComponent& weather,
			const asECS::ComponentManager<asScene::ArmatureComponent>& armature,
			asECS::ComponentManager<asScene::TransformComponent>& transforms,
			asECS::ComponentManager<asScene::MeshComponent>& meshes,
			asECS::ComponentManager<asScene::ObjectComponent>& objects,
			asECS::ComponentManager<asScene::RigidBodyPhysicsComponent>& rigidbodies,
			asECS::ComponentManager<asScene::SoftBodyPhysicsComponent>& softbodies,
			float dt
		);
	}
}
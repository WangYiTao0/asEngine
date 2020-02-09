#pragma once
#include "CommonInclude.h"
#include "API/asGraphicsDevice.h"
#include "asEnums.h"
#include "System/asECS.h"
#include "System/asScene_Dec1.h"
#include "Helpers/asIntersect.h"

#include <memory>

class asArchive;

namespace as
{
	namespace asScene
	{

		class asHairParticle
		{
		private:
			std::unique_ptr<asGraphics::GPUBuffer> cb;
			std::unique_ptr<asGraphics::GPUBuffer> particleBuffer;
			std::unique_ptr<asGraphics::GPUBuffer> simulationBuffer;
		public:

			void UpdateCPU(const TransformComponent& transform, const MeshComponent& mesh, float dt);
			void UpdateGPU(const MeshComponent& mesh, const MaterialComponent& material, asGraphics::CommandList cmd) const;
			void Draw(const CameraComponent& camera, const MaterialComponent& material, RENDERPASS renderPass, bool transparent, asGraphics::CommandList cmd) const;

			enum FLAGS
			{
				EMPTY = 0,
				REGENERATE_FRAME = 1 << 0,
			};
			uint32_t _flags = EMPTY;

			asECS::Entity meshID = asECS::INVALID_ENTITY;

			uint32_t strandCount = 0;
			uint32_t segmentCount = 1;
			uint32_t randomSeed = 1;
			float length = 1.0f;
			float stiffness = 10.0f;
			float randomness = 0.2f;
			float viewDistance = 200;

			// Non-serialized attributes:
			XMFLOAT4X4 world;
			XMFLOAT4X4 worldPrev;
			AABB aabb;

			void Serialize(asArchive& archive, uint32_t seed = 0);

			static void LoadShaders();
			static void Initialize();
		};

	}
}
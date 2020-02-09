#include "aspch.h"
#include "asInitializer.h"
#include "asEngine.h"

#include <sstream>
namespace as
{
	namespace asInitializer
	{
		bool initializationStarted = false;
		asJobSystem::context ctx;

		void InitializeComponentsImmediate()
		{
			InitializeComponentsAsync();
			asJobSystem::Wait(ctx);
		}
		void InitializeComponentsAsync()
		{
			initializationStarted = true;

			std::stringstream ss;
			ss << std::endl << "[asInitializer] Initializing Wicked Engine, please wait..." << std::endl;
			//ss << "Version: " << asVersion::GetVersionString() << std::endl;
			asBackLog::post(ss.str().c_str());

			asJobSystem::Initialize();

			asJobSystem::Execute(ctx, [] { asFont::Initialize(); });
			asJobSystem::Execute(ctx, [] { asImage::Initialize(); });
			//asJobSystem::Execute(ctx, [] { asInput::Initialize(); });
			//asJobSystem::Execute(ctx, [] { asRenderer::Initialize(); asWidget::LoadShaders(); });
			asJobSystem::Execute(ctx, [] { asAudio::Initialize(); });
			//asJobSystem::Execute(ctx, [] { asNetwork::Initialize(); });
			asJobSystem::Execute(ctx, [] { asTextureHelper::Initialize(); });
			asJobSystem::Execute(ctx, [] { asScene::asHairParticle::Initialize(); });
			asJobSystem::Execute(ctx, [] { asScene::asEmittedParticle::Initialize(); });
			asJobSystem::Execute(ctx, [] { asOcean::Initialize(); });
			asJobSystem::Execute(ctx, [] { asGPUSortLib::LoadShaders(); });
			asJobSystem::Execute(ctx, [] { asGPUBVH::LoadShaders(); });
			asJobSystem::Execute(ctx, [] { asPhysicsEngine::Initialize(); });

		}

		bool IsInitializeFinished()
		{
			return initializationStarted && !asJobSystem::IsBusy(ctx);
		}
	}
}
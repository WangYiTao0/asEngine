#pragma once

// NOTE:
// The purpose of this file is to expose all engine features.
// It should be included in the engine's implementing project not the engine itself!
// It should be included in the precompiled header if available.


#include "CommonInclude.h"

// High-level interface:
#include "High_Level_Interface/RenderPath.h"
#include "High_Level_Interface/RenderPath2D.h"
#include "High_Level_Interface/RenderPath3D.h"
#include "High_Level_Interface/RenderPath3D_Forward.h"
#include "High_Level_Interface/RenderPath3D_Deferred.h"
#include "High_Level_Interface/RenderPath3D_TiledForward.h"
#include "High_Level_Interface/RenderPath3D_TiledDeferred.h"
#include "High_Level_Interface/RenderPath3D_PathTracing.h"
#include "High_Level_Interface/LoadingScreen.h"
#include "High_Level_Interface/MainComponent.h"

// Engine-level systems
//#include "asVersion.h"
#include "Tools/asBackLog.h"
#include "Helpers/asIntersect.h"
#include "Graphics/asImage.h"
#include "Graphics/asSprite.h"
#include "Graphics/asFont.h"
#include "System/asScene.h"
#include "Graphics/asEmittedParticle.h"
#include "Graphics/asHairParticle.h"
#include "Graphics/asRenderer.h"
#include "Helpers/asMath.h"
#include "Audio/asAudio.h"
#include "Helpers/asResourceManager.h"
#include "Helpers/asTimer.h"
#include "Helpers/asHelper.h"
//#include "asInput.h"
//#include "asRawInput.h"
//#include "asXInput.h"
#include "Graphics/asTextureHelper.h"
#include "Helpers/asRandom.h"
#include "Helpers/asColors.h"
#include "Physics/asPhysicsEngine.h"
#include "Graphics/asEnums.h"
#include "System/asInitializer.h"
//#include "asLua.h"
//#include "asLuna.h"
#include "Graphics/API/asGraphicsDevice.h"
//#include "asGUI.h"
//#include "asWidget.h"
#include "Helpers/asHashString.h"
#include "Helpers/asArchive.h"
#include "Helpers/asSpinLock.h"
#include "Helpers/asRectPacker.h"
#include "Tools/asProfiler.h"
#include "Graphics/asOcean.h"
#include "Helpers/asStartupArguments.h"
#include "Graphics/asGPUBVH.h"
#include "Graphics/asGPUSortLib.h"
#include "System/asJobSystem.h"
//#include "asNetwork.h"


// For use by asEngine applications
#include "Graphics\asFont.h"
#include "Core/Application.h"
#include "Core/Log.h"

//#ifdef _WIN32
//#ifdef WINSTORE_SUPPORT
//#pragma comment(lib,"WickedEngine_UWP.lib")
//#else
//#pragma comment(lib,"WickedEngine_Windows.lib")
//#endif // WINSTORE_SUPPORT
//#endif // _WIN32
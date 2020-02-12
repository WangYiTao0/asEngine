#pragma once
#include "CommonInclude.h"
#include "Graphics/API/asGraphicsDevice.h"
#include "Audio/asAudio.h"
#include "asHashString.h"

#include <memory>
#include <mutex>
#include <unordered_map>

struct asResource
{
	union 
	{
		const void* data = nullptr;
		const as::asGraphics::Texture* texture;
		const as::asAudio::Sound* sound;
	};

	enum DATA_TYPE
	{
		EMPTY,
		IMAGE,
		SOUND,
	}type = EMPTY;

	~asResource();
};

namespace as
{
	namespace asResourceManager
	{
		// Load a resource
		std::shared_ptr<asResource> Load(const asHashString& name);
		// Check if a resource is currently loaded
		bool Contains(const asHashString& name);
		// Register a pre-created resource
		std::shared_ptr<asResource> Register(const asHashString& name, void* data, asResource::DATA_TYPE data_type);
		// Invalidate all resources
		void Clear();
	}
}
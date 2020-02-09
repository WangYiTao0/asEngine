#pragma once

#include "CommonInclude.h"
#include "Graphics/API/asGraphicsDevice.h"
#include "Helpers/asColors.h"

namespace as
{
	namespace asTextureHelper
	{
		void Initialize();
		const asGraphics::Texture* getRandom64x64();
		const asGraphics::Texture* getColorGradeDefault();
		const asGraphics::Texture* getNormalMapDefault();
		const asGraphics::Texture* getBlackCubeMap();

		const asGraphics::Texture* getWhite();
		const asGraphics::Texture* getBlack();
		const asGraphics::Texture* getTransparent();
		const asGraphics::Texture* getColor(asColor color);

		bool CreateTexture(asGraphics::Texture& texture, const uint8_t* data, uint32_t width, uint32_t height, asGraphics::FORMAT format = asGraphics::FORMAT_R8G8B8A8_UNORM);
	};
}
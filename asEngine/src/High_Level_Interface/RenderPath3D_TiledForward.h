#pragma once
#include "RenderPath3D_Forward.h"
namespace as
{
	class RenderPath3D_TiledForward :
		public RenderPath3D_Forward
	{
	private:
		void Render() const override;
	};
}


#pragma once

#include "Graphics/API/asGraphicsDevice.h"
#include "Graphics\asFFTGenerator.h"
#include "System\asScene.h"

#include <vector>

class asOcean
{
public:
	asOcean(const as::asScene::WeatherComponent& weather);

	void UpdateDisplacementMap(const as::asScene::WeatherComponent& weather, float time, as::asGraphics::CommandList cmd) const;
	void Render(const as::asScene::CameraComponent& camera, const as::asScene::WeatherComponent& weather, float time, as::asGraphics::CommandList cmd) const;

	const as::asGraphics::Texture* getDisplacementMap() const;
	const as::asGraphics::Texture* getGradientMap() const;

	static void Initialize();
	static void LoadShaders();

protected:
	as::asGraphics::Texture displacementMap;		// (RGBA32F)
	as::asGraphics::Texture gradientMap;			// (RGBA16F)


	void initHeightMap(const as::asScene::WeatherComponent& weather, XMFLOAT2* out_h0, float* out_omega);


	// Initial height field H(0) generated by Phillips spectrum & Gauss distribution.
	as::asGraphics::GPUBuffer buffer_Float2_H0;

	// Angular frequency
	as::asGraphics::GPUBuffer buffer_Float_Omega;

	// Height field H(t), choppy field Dx(t) and Dy(t) in frequency domain, updated each frame.
	as::asGraphics::GPUBuffer buffer_Float2_Ht;

	// Height & choppy buffer in the space domain, corresponding to H(t), Dx(t) and Dy(t)
	as::asGraphics::GPUBuffer buffer_Float_Dxyz;

	as::asGraphics::GPUBuffer immutableCB;
	as::asGraphics::GPUBuffer perFrameCB;
};
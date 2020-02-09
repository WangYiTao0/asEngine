#pragma once
#include "asImage.h"
#include "API/asGraphicsDevice.h"
#include "Helpers/asResourceManager.h"
#include "Helpers/asRandom.h"

#include <memory>

class asSprite
{
private:
	std::string textureName, maskName;
	std::shared_ptr<asResource> textureResource;
	std::shared_ptr<asResource> maskResource;
public:
	asSprite(const std::string& newTexture = "", const std::string& newMask = "");

	virtual void FixedUpdate(float speed);
	virtual void Update(float dt);
	void Draw(as::asGraphics::CommandList cmd) const;
	void DrawNormal(as::asGraphics::CommandList cmd) const;

	asImageParams params;

	struct Anim
	{
		struct MovingTexAnim
		{
			float speedX = 0; // the speed of texture scrolling animation in horizontal direction
			float speedY = 0; // the speed of texture scrolling animation in vertical direction
		};
		struct DrawRectAnim
		{
			float frameRate = 30; // target frame rate of the spritesheet animation (eg. 30, 60, etc.)
			int frameCount = 1; // how many frames are in the animation in total
			int horizontalFrameCount = 0; // how many horizontal frames there are (optional, use if the spritesheet contains multiple rows)

			float _elapsedTime = 0; // internal use; you don't need to initialize
			int _currentFrame = 0; // internal use; you don't need to initialize
		};
		struct WobbleAnim
		{
			XMFLOAT2 amount = XMFLOAT2(0, 0);	// how much the sprite wobbles in X and Y direction
			float speed = 1; // how fast the sprite wobbles

			float corner_angles[4]; // internal use; you don't need to initialize
			float corner_speeds[4]; // internal use; you don't need to initialize
			float corner_angles2[4]; // internal use; you don't need to initialize
			float corner_speeds2[4]; // internal use; you don't need to initialize
			WobbleAnim()
			{
				for (int i = 0; i < 4; ++i)
				{
					corner_angles[i] = as::asRandom::getRandom(0, 1000) / 1000.0f * XM_2PI;
					corner_speeds[i] = as::asRandom::getRandom(500, 1000) / 1000.0f;
					if (as::asRandom::getRandom(0, 1) == 0)
					{
						corner_speeds[i] *= -1;
					}
					corner_angles2[i] = as::asRandom::getRandom(0, 1000) / 1000.0f * XM_2PI;
					corner_speeds2[i] = as::asRandom::getRandom(500, 1000) / 1000.0f;
					if (as::asRandom::getRandom(0, 1) == 0)
					{
						corner_speeds2[i] *= -1;
					}
				}
			}
		};

		bool repeatable = false;
		XMFLOAT3 vel = XMFLOAT3(0, 0, 0);
		float rot = 0;
		float scaleX = 0;
		float scaleY = 0;
		float opa = 0;
		float fad = 0;
		MovingTexAnim movingTexAnim;
		DrawRectAnim drawRectAnim;
		WobbleAnim wobbleAnim;
	};
	Anim anim;

	const as::asGraphics::Texture* getTexture() { return textureResource->texture; }
	void setTexture(const as::asGraphics::Texture* texture)
	{
		textureResource = std::make_shared<asResource>();
		textureResource->texture = texture;
		textureResource->type = asResource::IMAGE;
	}
};


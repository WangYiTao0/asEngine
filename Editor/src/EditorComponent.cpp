#include "EditorComponent.h"

void EditorComponent::Initialize()
{
	__super::Initialize();

	infoDisplay.active = true;
	infoDisplay.watermark = true;
	infoDisplay.fpsinfo = true;
	infoDisplay.resolution = true;

	ActivatePath(new TestRenderer);
}

TestRenderer::TestRenderer()
{
	//setSSREnabled(false);
	//setSSAOEnabled(false);
	//setReflectionsEnabled(true);
	//setFXAAEnabled(false);

	float screenW = (float)as::asRenderer::GetDevice()->GetScreenWidth();
	float screenH = (float)as::asRenderer::GetDevice()->GetScreenHeight();
}

void TestRenderer::RunJobSystemTest()
{
}

void TestRenderer::RunFontTest()
{
}

void TestRenderer::RunSpriteTest()
{
}

void TestRenderer::RunNetworkTest()
{
}

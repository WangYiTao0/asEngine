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
	setSSREnabled(false);
	setSSAOEnabled(false);
	setReflectionsEnabled(true);
	setFXAAEnabled(false);

	as::asScene::LoadModel("assets/models/bunny.obj"); // Simply load a model into the current global scene
	as::asScene::GetScene(); // Get the current global scene
	as::asRenderer::ClearWorld(); // Delete every model, etc. from the current global scene

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

#include "Test.h"

using namespace as;

void TestComponent::Initialize()
{
	__super::Initialize();
	
	infoDisplay.active = true;
	infoDisplay.watermark = true;
	infoDisplay.fpsinfo = true;
	infoDisplay.resolution = true;

	ActivatePath(new Test3D);
}

Test3D::Test3D()
{
	setSSREnabled(false);
	setSSAOEnabled(false);
	setReflectionsEnabled(true);
	setFXAAEnabled(false);

	float screenW = (float)asRenderer::GetDevice()->GetScreenWidth();
	float screenH = (float)asRenderer::GetDevice()->GetScreenHeight();

	asLabel* label = new asLabel("Label1");
	label->SetText("Wicked Engine Test Framework");
	label->SetSize(XMFLOAT2(200, 15));
	label->SetPos(XMFLOAT2(screenW / 2.f - label->scale.x / 2.f, screenH * 0.95f));
	GetGUI().AddWidget(label);
}

Test::Test()
{
	
}

void Test::OnAttach()
{
	auto& a = as::Application::Get();
	auto hwnd = HWND(a.GetWindow().GetWindow());

	testComponent.SetWindow(hwnd);
	testComponent.Initialize();
}

void Test::OnDetach()
{

}

void Test::OnUpdate(float ts)
{
	testComponent.Run();
}

void Test::OnImGuiRender()
{

}

void Test::OnEvent(as::Event& e)
{

}


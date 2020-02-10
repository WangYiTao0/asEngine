#include "Test.h"

void TestComponent::Initialize()
{
	__super::Initialize();

	infoDisplay.active = true;
	infoDisplay.watermark = true;
	infoDisplay.fpsinfo = true;
	infoDisplay.resolution = true;

	//ActivatePath(new TestsRenderer);
}

Test::Test()
{
	//test.Initialize();
}

void Test::OnAttach()
{

}

void Test::OnDetach()
{

}

void Test::OnUpdate(float ts)
{
	//test.Run();
	//test.Update(ts);
}

void Test::OnImGuiRender()
{

}

void Test::OnEvent(as::Event& e)
{

}

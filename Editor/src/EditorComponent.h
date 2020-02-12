#pragma once
#include <asEngine.h>

class EditorComponent : public as::MainComponent
{
public:
	virtual void Initialize()override;
};

class TestRenderer : public as::RenderPath3D_Deferred
{
public:
	TestRenderer();

	void RunJobSystemTest();
	void RunFontTest();
	void RunSpriteTest();
	void RunNetworkTest();
};
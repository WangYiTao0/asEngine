#pragma once

#include "asEngine.h"

class TestComponent : public as::MainComponent
{
public:
	virtual void Initialize() override;
};

class Test : public as::Layer
{
public:
	Test();
	virtual ~Test() = default;

	virtual void OnAttach() override;
	virtual void OnDetach() override;

	void OnUpdate(float ts) override;
	virtual void OnImGuiRender() override;
	void OnEvent(as::Event & e) override;
private:
	TestComponent test;
};




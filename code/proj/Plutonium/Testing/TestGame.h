#pragma once
#include <Application.h>
#include <Graphics/Vulkan/Shaders/Renderpass.h>

class TestGame
	: public Pu::Application
{
public:
	TestGame(void);
	TestGame(const TestGame&) = delete;
	TestGame(TestGame&&) = delete;

	TestGame& operator =(const TestGame&) = delete;
	TestGame& operator =(TestGame&&) = delete;

protected:
	virtual bool GpuPredicate(const Pu::PhysicalDevice& device) override;
	virtual void Initialize(void);
	virtual void LoadContent(void);
	virtual void UnLoadContent(void) {}
	virtual void Finalize(void);
	virtual void Update(float) {}
	virtual void Render(float);
	virtual void RenderLoad(float) {}

private: 
	Pu::Renderpass *renderpass;
};
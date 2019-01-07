#pragma once
#include <Application.h>

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
	virtual void Initialize(void);
	virtual void LoadContent(void);
	virtual void UnLoadContent(void);
	virtual void Finalize(void);
	virtual void Update(float) {}
	virtual void Render(float, Pu::CommandBuffer &cmdBuffer);
	virtual void RenderLoad(float) {}

private: 
	Pu::Renderpass *renderpass;
	Pu::GraphicsPipeline *pipeline;
	Pu::Buffer *vrtxBuffer, *vrtxStagingBuffer, *imgStagingBuffer;
	Pu::Image *image;
	Pu::ImageView *view;
	Pu::DescriptorSet *descriptor;
};
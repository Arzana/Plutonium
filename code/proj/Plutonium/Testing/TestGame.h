#pragma once
#include <Application.h>
#include <Graphics/Text/Font.h>
#include <Components/FreeCamera.h>
#include <Graphics/Resources/Mesh.h>
#include <Graphics/Text/TextRenderer.h>
#include <Graphics/UI/Items/Label.h>
#include "TransformBlock.h"

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
	virtual void Update(float);
	virtual void Render(float dt, Pu::CommandBuffer &cmdBuffer);

private: 
	Pu::FreeCamera *cam;

	Pu::GraphicsPipeline *pipeline;
	Pu::DepthBuffer *depthBuffer;
	Pu::Buffer *vrtxBuffer;
	Pu::StagingBuffer *vrtxStagingBuffer;
	Pu::Texture2D *image;
	Pu::Mesh *mesh;
	Pu::DescriptorSet *material;
	TransformBlock *transform;

	Pu::Font *font;
	Pu::GuiItemRenderer *uiRenderer;
	Pu::Label *item;
};
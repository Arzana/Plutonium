#pragma once
#include <Application.h>
#include <Graphics/Textures/Texture2D.h>
#include <Graphics/Resources/StagingBuffer.h>
#include <Components/FreeCamera.h>
#include <Graphics/Resources/Mesh.h>
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
	virtual void Render(float, Pu::CommandBuffer &cmdBuffer);

private: 
	Pu::GraphicsPipeline *pipeline;
	Pu::DepthBuffer *depthBuffer;
	Pu::Buffer *vrtxBuffer;
	Pu::StagingBuffer *vrtxStagingBuffer;
	Pu::Texture2D *image;
	Pu::Font *font;
	Pu::FreeCamera *cam;
	Pu::Mesh *mesh;
	TransformBlock *transform;
};
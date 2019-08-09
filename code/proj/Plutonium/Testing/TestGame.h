#pragma once
#include <Application.h>
#include <Content/PumLoader.h>
#include <Graphics/Models/Mesh.h>
#include <Components/FreeCamera.h>
#include <Graphics/Models/Material.h>
#include <Environment/Terrain/Lithosphere.h>
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
	bool remarkDepthBuffer;
	Pu::FreeCamera *cam;
	Pu::WindowMode newMode;

	Pu::Renderpass *renderpass;
	Pu::GraphicsPipeline *pipeline;
	Pu::DepthBuffer *depthBuffer;
	Pu::Buffer *vrtxBuffer;
	Pu::StagingBuffer *vrtxStagingBuffer;
	Pu::Mesh mesh;
	Pu::QueryPool *timestamps;

	Pu::DescriptorPool *descriptorPool;
	TransformBlock *transform;
	Pu::Material *material;

	Pu::Lithosphere *lithosphere;

	void RenderpassPreCreate(Pu::Renderpass&);
	void RenderpassPostCreate(Pu::Renderpass&);
};
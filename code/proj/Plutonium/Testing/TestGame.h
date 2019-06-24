#pragma once
#include <Application.h>
#include <Components/FreeCamera.h>
#include <Content/PumLoader.h>
#include <Graphics/Models/Mesh.h>
#include <Graphics/Diagnostics/DebugRenderer.h>
#include "TransformBlock.h"
#include "MonsterMaterial.h"

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
	Pu::DebugRenderer *debugRenderer;

	Pu::Renderpass *renderpass;
	Pu::GraphicsPipeline *pipeline;
	Pu::DepthBuffer *depthBuffer;
	Pu::Buffer *vrtxBuffer;
	Pu::StagingBuffer *vrtxStagingBuffer;
	Pu::Texture2D *image;
	Pu::Mesh mesh;
	Pu::AABB bb;
	Pu::QueryPool *queryPool;

	Pu::DescriptorPool *descriptorPool;
	TransformBlock *transform;
	MonsterMaterial *material;
	Pu::PumMaterial rawMaterial;
};
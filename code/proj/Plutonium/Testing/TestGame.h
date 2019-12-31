#pragma once
#include <Application.h>
#include <Components/FreeCamera.h>
#include <Graphics/Models/Mesh.h>
#include <Graphics/Models/Material.h>
#include <Graphics/Diagnostics/DebugRenderer.h>
#include "TransformBlock.h"
#include "DirLight.h"

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
	virtual void EnableFeatures(Pu::PhysicalDeviceFeatures &features) override;
	virtual void Initialize(void);
	virtual void LoadContent(void);
	virtual void UnLoadContent(void);
	virtual void Finalize(void);
	virtual void Update(float);
	virtual void Render(float dt, Pu::CommandBuffer &cmdBuffer);

private:
	Pu::FreeCamera *cam;
	bool firstRun, markDepthBuffer;
	Pu::TextureCube *environment;
	Pu::DebugRenderer *dbgRenderer;

	Pu::Renderpass *renderPass;
	Pu::GraphicsPipeline *gfxPipeline;
	Pu::DepthBuffer *depthBuffer;
	Pu::DescriptorPool *descPoolCam;
	Pu::DescriptorPool *descPoolLight;
	Pu::DescriptorPool *descPoolMats;

	Pu::Buffer *vrtxBuffer;
	Pu::StagingBuffer *stagingBuffer;
	Pu::vector<std::tuple<Pu::uint32, Pu::Mesh*>> meshes;
	Pu::vector<Pu::Material*> materials;
	Pu::vector<Pu::Texture2D*> textures;
	Pu::vector<Pu::PumMaterial> stageMaterials;

	Pu::Matrix mdlMtrx;
	TransformBlock *transform;
	DirLight *light;

	void OnAnyKeyDown(const Pu::InputDevice &sender, const Pu::ButtonEventArgs &args);
	void OnSwapchainRecreated(const Pu::GameWindow&);
	void CreateDepthBuffer(void);
	void InitializeRenderpass(Pu::Renderpass&);
	void FinalizeRenderpass(Pu::Renderpass&);
	void CreateGraphicsPipeline(void);
};
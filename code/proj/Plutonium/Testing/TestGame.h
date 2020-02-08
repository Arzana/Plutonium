#pragma once
#include <Application.h>
#include <Graphics/Models/Mesh.h>
#include <Graphics/Models/Material.h>
#include <Graphics/Cameras/FreeCamera.h>
#include <Graphics/Lighting/DirectionalLight.h>
#include <Graphics/Diagnostics/DebugRenderer.h>
#include <Graphics/Lighting/LightProbeRenderer.h>
#include <Graphics/Vulkan/Pipelines/PipelineCache.h>

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
	virtual void Update(float) {}
	virtual void Render(float dt, Pu::CommandBuffer &cmdBuffer);

private:
	Pu::FreeCamera *cam;
	bool firstRun, markDepthBuffer, updateCam;
	Pu::DebugRenderer *dbgRenderer;
	Pu::PipelineCache *cache;

	Pu::LightProbeRenderer *probeRenderer;
	Pu::LightProbe *environment;
	Pu::DescriptorPool *probePool;
	Pu::vector<Pu::DescriptorSet> probeSets;

	Pu::Renderpass *renderPass;
	Pu::GraphicsPipeline *gfxPipeline;
	Pu::DepthBuffer *depthBuffer;
	Pu::DescriptorPool *descPoolCam;
	Pu::DescriptorPool *descPoolLight;
	Pu::DescriptorPool *descPoolMats;

	Pu::Buffer *vrtxBuffer;
	Pu::StagingBuffer *stagingBuffer;
	Pu::vector<std::pair<Pu::uint32, Pu::Mesh*>> meshes;
	Pu::vector<Pu::Material*> materials;
	Pu::vector<Pu::Texture2D*> textures;
	Pu::vector<Pu::PumMaterial> stageMaterials;

	Pu::Matrix mdlMtrx;
	Pu::DirectionalLight *light;

	void OnAnyKeyDown(const Pu::InputDevice &sender, const Pu::ButtonEventArgs &args);
	void OnSwapchainRecreated(const Pu::GameWindow&, const Pu::SwapchainReCreatedEventArgs &args);
	void CreateDepthBuffer(void);
	void InitializeRenderpass(Pu::Renderpass&);
	void FinalizeRenderpass(Pu::Renderpass&);
	void CreateGraphicsPipeline(void);
};
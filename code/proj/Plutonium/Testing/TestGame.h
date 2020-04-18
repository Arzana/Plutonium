#pragma once
#include <Application.h>
#include <Graphics/Models/Model.h>
#include <Graphics/Cameras/FreeCamera.h>
#include <Graphics/Diagnostics/DebugRenderer.h>
#include <Graphics/Lighting/DeferredRenderer.h>
#include <Graphics/Lighting/LightProbeRenderer.h>
#include <Core/Math/HeightMap.h>

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
	void EnableFeatures(const Pu::PhysicalDeviceFeatures &supported, Pu::PhysicalDeviceFeatures &enabeled) final;
	void Initialize(void) final;
	void LoadContent(Pu::AssetFetcher &content) final;
	void UnLoadContent(Pu::AssetFetcher &content) final;
	void Finalize(void) final {}
	void Update(float) final {}
	void Render(float dt, Pu::CommandBuffer &cmdBuffer) final;

private:
	Pu::FreeCamera *cam;
	bool firstRun, markDepthBuffer, updateCam;
	Pu::DebugRenderer *dbgRenderer;

	Pu::DeferredRenderer *renderer;
	Pu::DescriptorPool *descPoolConst;

	Pu::LightProbeRenderer *probeRenderer;
	Pu::LightProbe *environment;
	Pu::TextureCube *skybox;
	Pu::DirectionalLight *lightMain, *lightFill;

	Pu::MeshCollection terrainMesh;
	Pu::Terrain *terrainMat;
	Pu::StagingBuffer *srcBuffer, *heightBuffer;
	Pu::Image *heightImg;
	Pu::Sampler *heightSampler;
	Pu::Texture2D *height, *mask;
	Pu::Texture2DArray *textures;
	Pu::HeightMap heightMap;

	void OnAnyKeyDown(const Pu::InputDevice &sender, const Pu::ButtonEventArgs &args);
	void OnSwapchainRecreated(const Pu::GameWindow&, const Pu::SwapchainReCreatedEventArgs&);
};
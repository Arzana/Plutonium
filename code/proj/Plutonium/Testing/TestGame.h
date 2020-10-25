#pragma once
#include <Application.h>
#include <Graphics/Cameras/FreeCamera.h>
#include <Graphics/Diagnostics/DebugRenderer.h>
#include <Physics/Systems/PhysicalWorld.h>

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
	void EnableFeatures(const Pu::PhysicalDeviceFeatures &supported, Pu::PhysicalDeviceFeatures &enabeled, Pu::vector<const char*> &extensions) final;
	void Initialize(void) final {}
	void LoadContent(Pu::AssetFetcher &content) final;
	void UnLoadContent(Pu::AssetFetcher &content) final;
	void Finalize(void) final {}
	void Update(float) final;
	void Render(float dt, Pu::CommandBuffer &cmdBuffer) final;

private:
	Pu::FreeCamera *camFree;
	bool firstRun, updateCam, spawnToggle;
	bool showProfiler, showCamOpt, showPhysics, showAssets, updateRenderer;
	Pu::DebugRenderer *dbgRenderer;
	const Pu::SurfaceFormat *desiredFormat;
	int vsynchMode;

	Pu::DeferredRenderer *renderer;
	Pu::DescriptorPool *descPoolConst;

	Pu::TextureCube *skybox;
	Pu::DirectionalLight *lightMain, *lightFill;
	Pu::vector<Pu::TerrainChunk*> terrain;
	Pu::PerlinNoise noise;
	Pu::PhysicalWorld *world;

	Pu::DynamicBuffer *permutations;
	Pu::Buffer *perlin;
	Pu::ShaderProgram *compute;
	Pu::ComputePipeline *computePipe;
	Pu::DescriptorPool *descPoolPerlin;
	Pu::DescriptorSetGroup *perlinSet;

	void OnAnyMouseScrolled(const Pu::Mouse&, Pu::int16 value);
	void OnAnyKeyDown(const Pu::InputDevice &sender, const Pu::ButtonEventArgs &args);
	void OnSwapchainRecreated(const Pu::GameWindow&, const Pu::SwapchainReCreatedEventArgs&);
};
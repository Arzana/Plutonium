#pragma once
#include <Application.h>
#include <Graphics/Cameras/FreeCamera.h>
#include <Graphics/Diagnostics/DebugRenderer.h>
#include <Graphics/Lighting/DeferredRenderer.h>
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
	bool GpuPredicate(_In_ const Pu::PhysicalDevice &physicalDevice) final;
	void EnableFeatures(const Pu::PhysicalDeviceFeatures &supported, Pu::PhysicalDeviceFeatures &enabeled) final;
	void Initialize(void) final {}
	void LoadContent(Pu::AssetFetcher &content) final;
	void UnLoadContent(Pu::AssetFetcher &content) final;
	void Finalize(void) final {}
	void Update(float) final {}
	void Render(float dt, Pu::CommandBuffer &cmdBuffer) final;

private:
	Pu::FreeCamera *camFree;
	bool firstRun, updateCam, spawn;
	Pu::DebugRenderer *dbgRenderer;

	Pu::DeferredRenderer *renderer;
	Pu::DescriptorPool *descPoolConst;

	Pu::TextureCube *skybox;
	Pu::DirectionalLight *lightMain, *lightFill;

	Pu::PhysicalWorld *world;
	Pu::Model *modelSphere, *modelPlane;

	Pu::Sphere collider;
	Pu::PhysicalObject spherePrefab;
	Pu::vector<Pu::PhysicsHandle> spheres;
	Pu::vector<Pu::PhysicsHandle> planes;

	void OnAnyKeyDown(const Pu::InputDevice &sender, const Pu::ButtonEventArgs &args);
	void OnAnyKeyUp(const Pu::InputDevice &sender, const Pu::ButtonEventArgs &args);
	void OnSwapchainRecreated(const Pu::GameWindow&, const Pu::SwapchainReCreatedEventArgs&);
};
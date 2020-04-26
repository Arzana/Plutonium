#pragma once
#include <Application.h>
#include <Graphics/Models/Model.h>
#include <Graphics/Cameras/FreeCamera.h>
#include <Graphics/Cameras/FollowCamera.h>
#include <Graphics/Diagnostics/DebugRenderer.h>
#include <Graphics/Lighting/DeferredRenderer.h>
#include <Graphics/Lighting/LightProbeRenderer.h>

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
	void Update(float dt) final;
	void Render(float dt, Pu::CommandBuffer &cmdBuffer) final;

private:
	Pu::FreeCamera *cam;
	bool firstRun, updateCam;
	Pu::DebugRenderer *dbgRenderer;

	Pu::DeferredRenderer *renderer;
	Pu::DescriptorPool *descPoolConst;
	Pu::LightProbeRenderer *probeRenderer;
	Pu::LightProbe *environment;

	Pu::TextureCube *skybox;
	Pu::DirectionalLight *lightMain, *lightFill;
	Pu::Model *ground, *ball;

	Pu::Plane collider;
	Pu::Matrix groundOrien, iI;
	Pu::Quaternion rot;
	Pu::Vector3 pos, vloc, angularVloc;
	Pu::Matrix player;
	float imass, e, time;

	void OnAnyKeyDown(const Pu::InputDevice &sender, const Pu::ButtonEventArgs &args);
	void OnSwapchainRecreated(const Pu::GameWindow&, const Pu::SwapchainReCreatedEventArgs&);
};
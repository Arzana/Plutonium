#pragma once
#include <Application.h>
#include <Graphics/Models/Model.h>
#include <Graphics/Cameras/FreeCamera.h>
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

	Pu::Model *model;
	Pu::Matrix mdlMtrx;
	Pu::DirectionalLight *lightMain, *lightFill;

	void OnAnyKeyDown(const Pu::InputDevice &sender, const Pu::ButtonEventArgs &args);
};
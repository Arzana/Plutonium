#pragma once
#include <Application.h>
#include <Graphics/Models/Model.h>
#include <Graphics/Cameras/FreeCamera.h>
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
	virtual void EnableFeatures(Pu::PhysicalDeviceFeatures &features) override;
	virtual void Initialize(void);
	virtual void LoadContent(void);
	virtual void UnLoadContent(void);
	virtual void Finalize(void) {}
	virtual void Update(float) {}
	virtual void Render(float dt, Pu::CommandBuffer &cmdBuffer);

private:
	Pu::FreeCamera *cam;
	bool firstRun, markDepthBuffer, updateCam;

	Pu::DeferredRenderer *renderer;
	Pu::DescriptorPool *descPoolConst;

	Pu::LightProbeRenderer *probeRenderer;
	Pu::LightProbe *environment;

	Pu::Model *model;
	Pu::Matrix mdlMtrx;
	Pu::DirectionalLight *light;

	void OnAnyKeyDown(const Pu::InputDevice &sender, const Pu::ButtonEventArgs &args);
};
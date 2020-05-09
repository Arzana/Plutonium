#pragma once
#include <Application.h>
#include <Graphics/Cameras/FreeCamera.h>
#include <Graphics/Cameras/FollowCamera.h>
#include <Graphics/Diagnostics/DebugRenderer.h>
#include <Graphics/Lighting/DeferredRenderer.h>
#include <Graphics/Lighting/LightProbeRenderer.h>
#include <Physics/PhysicalWorld.h>
#include <Core/Math/HeightMap.h>
#include <Core/Math/Matrix3.h>

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
	Pu::Camera *camActive;
	Pu::FreeCamera *camFree;
	Pu::FollowCamera *camFollow;
	bool firstRun, updateCam;
	Pu::DebugRenderer *dbgRenderer;

	Pu::DeferredRenderer *renderer;
	Pu::DescriptorPool *descPoolConst;
	Pu::LightProbeRenderer *probeRenderer;
	Pu::LightProbe *environment;

	Pu::TextureCube *skybox;
	Pu::DirectionalLight *lightMain, *lightFill;

	Pu::MeshCollection groundMesh;
	Pu::Terrain *groundMat;
	Pu::HeightMap *collider;
	Pu::Image *perlinImg;
	Pu::Sampler *perlinSampler;
	Pu::Texture2D *perlin, *mask;
	Pu::Texture2DArray *groundTextures;
	Pu::StagingBuffer *groundMeshStagingBuffer, *perlinStagingBuffer;

	Pu::Model *playerModel;
	Pu::Matrix playerWorld;
	Pu::Matrix3 MoI;
	Pu::Vector3 input, vloc, angularVloc;
	float mass, imass, e, time, speed;

	Pu::PhysicalWorld *world;

	void OnAnyKeyDown(const Pu::InputDevice &sender, const Pu::ButtonEventArgs &args);
	void OnAnyKeyUp(const Pu::InputDevice &sender, const Pu::ButtonEventArgs &args);
	void OnMouseScrolled(const Pu::Mouse&, Pu::int16 args);
	void OnSwapchainRecreated(const Pu::GameWindow&, const Pu::SwapchainReCreatedEventArgs&);
};
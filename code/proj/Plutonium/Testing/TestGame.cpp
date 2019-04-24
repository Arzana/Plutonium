#include "TestGame.h"
#include <Input/Keys.h>
#include <Content/GLTFParser.h>
#include <Graphics/VertexLayouts/SkinnedAnimated.h>
#include <Graphics/Textures/DepthBuffer.h>
#include <Core/Diagnostics/CPU.h>
#include <Graphics/UI/Rendering/BasicGuiBackgroundRenderer.h>

using namespace Pu;

TestGame::TestGame(void)
	: Application(L"TestGame", 1920.0f, 1080.0f)
{
	GetInput().AnyKeyboard.KeyDown += [this](const Keyboard&, uint16 key)
	{
		switch (key)
		{
		case (_CrtEnum2Int(Keys::Escape)):
			Exit();
			break;
		case (_CrtEnum2Int(Keys::Up)):
			cam->MoveSpeed++;
			break;
		case (_CrtEnum2Int(Keys::Down)):
			cam->MoveSpeed--;
			break;
		case (_CrtEnum2Int(Keys::M)):
			if (Mouse::IsCursorVisible())
			{
				Mouse::HideCursor();
				cam->Enable();
			}
			else
			{
				Mouse::ShowCursor();
				cam->Disable();
			}
			break;
		}
	};
}

void TestGame::Initialize(void)
{
	/* Make sure that the cursor is hiden and locked if the window is focused. */
	GetWindow().GetNative().OnGainedFocus += [](const NativeWindow &sender)
	{
		Mouse::HideCursor();
		Mouse::LockCursor(sender);
	};

	GetWindow().GetNative().OnLostFocus += [](const NativeWindow&)
	{
		Mouse::ShowCursor();
		Mouse::FreeCursor();
	};

	AddComponent(cam = new FreeCamera(*this, GetInput().AnyKeyboard, GetInput().AnyMouse));
	depthBuffer = new DepthBuffer(GetDevice(), Format::D32_SFLOAT, GetWindow().GetNative().GetSize());

	/* Setup graphics pipeline. */
	pipeline = new GraphicsPipeline(GetDevice(), 2);
	pipeline->PostInitialize += [this](GraphicsPipeline &pipeline)
	{
		/* Set viewport, topology and add the vertex binding. */
		pipeline.SetViewport(GetWindow().GetNative().GetClientBounds());
		pipeline.SetTopology(PrimitiveTopology::TriangleList);
		pipeline.SetDepthStencilMode(true, true, false);
		pipeline.AddVertexBinding(0, sizeof(SkinnedAnimated));
		pipeline.Finalize();

		/* Create the framebuffers and the required uniform block. */
		GetWindow().CreateFrameBuffers(pipeline.GetRenderpass(), { &depthBuffer->GetView() });
		transform = new TransformBlock(GetDevice(), pipeline);
	};

	/* Setup and load render pass. */
	GetContent().FetchRenderpass(*pipeline, { L"{Shaders}SkinnedAnimated.vert", L"{Shaders}SkinnedAnimated.frag" }).OnLinkCompleted += [this](Renderpass &renderpass)
	{
		/* Set description and layout of FragColor. */
		renderpass.GetOutput("FragColor").SetDescription(GetWindow().GetSwapchain());

		/* Set the description and layout of the depth buffer. */
		renderpass.AddDepthStencil().SetDescription(*depthBuffer);

		/* Set offset for uv attribute (position is default). */
		renderpass.GetAttribute("Normal").SetOffset(vkoffsetof(SkinnedAnimated, Normal));
		renderpass.GetAttribute("TexCoord").SetOffset(vkoffsetof(SkinnedAnimated, TexCoord));
		renderpass.GetAttribute("Joints").SetOffset(vkoffsetof(SkinnedAnimated, Joints));
		renderpass.GetAttribute("Weights").SetOffset(vkoffsetof(SkinnedAnimated, Weights));
	};

	/* Make sure the framebuffers are re-created of the window resizes. */
	GetWindow().GetNative().OnSizeChanged += [this](const NativeWindow&, ValueChangedEventArgs<Vector2>)
	{
		GetWindow().CreateFrameBuffers(pipeline->GetRenderpass());
	};

	uiRenderer = new GuiItemRenderer(GetWindow(), GetContent(), 2);
}

void TestGame::LoadContent(void)
{
	GLTFFile file;
	_CrtLoadGLTF(L"../assets/models/Monster/Monster.gltf", file);

	/* Initialize the final vertex buffer and setup the staging buffer with our quad. */
	vrtxBuffer = new Buffer(GetDevice(), file.Buffers[0].Size, BufferUsageFlag::VertexBuffer | BufferUsageFlag::IndexBuffer | BufferUsageFlag::TransferDst, false);

	vector<std::reference_wrapper<Buffer>> buffers;
	buffers.emplace_back(*vrtxBuffer);
	vector<StagingBuffer*> stagingBuffers;
	vector<Mesh*> meshes;

	_CrtLoadAndParseGLTF(file, buffers, stagingBuffers, meshes);
	vrtxStagingBuffer = stagingBuffers[0];
	mesh = meshes[0];

	/* Load the texture. */
	image = &GetContent().FetchTexture2D(file.Images[0].Uri, SamplerCreateInfo(Filter::Linear, SamplerMipmapMode::Linear, SamplerAddressMode::Repeat));

	/* Load the content for the fonts. */
	font = &GetContent().FetchFont(L"{Fonts}LucidaConsole.ttf", 24.0f, CodeChart::ASCII());
}

void TestGame::UnLoadContent(void)
{
	GetContent().Release(*image);
	GetContent().Release(*font);

	delete mesh;
	delete vrtxStagingBuffer;
	delete vrtxBuffer;
}

void TestGame::Finalize(void)
{
	GetContent().Release(*pipeline);

	delete uiRenderer;
	delete material;
	delete transform;
	delete pipeline;
	delete depthBuffer;
}

void TestGame::Update(float)
{
	if (transform)
	{
		transform->SetProjection(cam->GetProjection());
		transform->SetView(cam->GetView());
	}
}

void TestGame::Render(float dt, CommandBuffer & cmdBuffer)
{
	if (!transform) return;
	/* Update the descriptor if needed. */
	transform->Update(cmdBuffer);

	static bool firstRender = true;
	if (firstRender)
	{
		/* Wait for the graphics pipeline to be usable. */
		if (!image->IsUsable()) return;
		firstRender = false;

		/* Initialize the depth buffer for writing. */
		depthBuffer->MakeWritable(cmdBuffer);

		/* Copy quad to final vertex buffer. */
		cmdBuffer.CopyEntireBuffer(*vrtxStagingBuffer, *vrtxBuffer);
		cmdBuffer.MemoryBarrier(*vrtxBuffer, PipelineStageFlag::Transfer, PipelineStageFlag::VertexInput, AccessFlag::VertexAttributeRead);

		/* Make sure the image layout is suitable for shader reads. */
		cmdBuffer.MemoryBarrier(*image, PipelineStageFlag::Transfer, PipelineStageFlag::FragmentShader, ImageLayout::ShaderReadOnlyOptimal, AccessFlag::ShaderRead, image->GetFullRange());

		/* Update the descriptor. */
		material = new DescriptorSet(std::move(pipeline->GetDescriptorPool().Allocate(1)));
		material->Write(pipeline->GetRenderpass().GetUniform("Albedo"), *image);
	}

	static bool firstTextRender = true;
	if (firstTextRender)
	{
		if (font->IsLoaded() && uiRenderer->CanBegin())
		{
			firstTextRender = false;

			cmdBuffer.MemoryBarrier(font->GetAtlas(), PipelineStageFlag::Transfer, PipelineStageFlag::FragmentShader, ImageLayout::ShaderReadOnlyOptimal, AccessFlag::ShaderRead, font->GetAtlas().GetFullRange());
			DescriptorSet *fontInfo = uiRenderer->GetTextRenderer().CreatFont(font->GetAtlas());
			TextUniformBlock *textInfo = uiRenderer->GetTextRenderer().CreateText();
			GuiBackgroundUniformBlock *uiInfo = uiRenderer->GetBackgroundRenderer().CreateGUI();
			AddComponent(item = new Button(*this, uiInfo, textInfo, fontInfo, *font));
			item->SetAutoSize(true);

			item->HoverEnter += [](GuiItem &sender) { sender.SetBackColor(Color::Blue()); };
			item->HoverLeave += [](GuiItem &sender) { sender.SetBackColor(Color::Black() * 0.5f); };
		}
	}

	/* Render scene. */
	cmdBuffer.AddLabel(u8"Monster", Color::Lime());
	cmdBuffer.BindGraphicsPipeline(*pipeline);
	cmdBuffer.BeginRenderPass(pipeline->GetRenderpass(), GetWindow().GetCurrentFramebuffer(pipeline->GetRenderpass()), SubpassContents::Inline);

	cmdBuffer.BindVertexBuffer(0, *mesh);
	cmdBuffer.BindIndexBuffer(mesh->GetIndex());
	cmdBuffer.BindGraphicsDescriptor(*transform);
	cmdBuffer.BindGraphicsDescriptor(*material);
	cmdBuffer.Draw(static_cast<uint32>(mesh->GetIndex().GetElementCount()), 1, 0, 0, 0);

	cmdBuffer.EndRenderPass();
	cmdBuffer.EndLabel();

	/* Render debug text. */
	if (!firstTextRender)
	{
		ustring text = U"Fps: ";
		text += ustring::from(iround(1.0f / dt));
		text += U"\nCPU Usage: ";
		text += ustring::from(ipart(CPU::GetCurrentProcessUsage() * 100.0f));
		text += U'%';

		item->SetText(text);
		item->Render(*uiRenderer);
		uiRenderer->Render(cmdBuffer);
	}
}
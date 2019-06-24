#pragma once
#include "Subpass.h"
#include "Content/Asset.h"
#include "Core/Events/EventBus.h"
#include "Graphics/Vulkan/DescriptorPool.h"

namespace Pu
{
	/* Defines a Vulkan renderpass that's made up of multiple subpasses. */
	class Renderpass
		: public Asset
	{
	public:
		/* Occurs before the renderpass is created. */
		EventBus<Renderpass> PreCreate;
		/* Occurs after the renderpass has been created. */
		EventBus<Renderpass> PostCreate;

		/* Initializes an empty instance of a renderpass. */
		Renderpass(_In_ LogicalDevice &device);
		/* Initializes a new instance of a renderpass with specified shader modules for specified subpasses. */
		Renderpass(_In_ LogicalDevice &device, _In_ std::initializer_list<std::initializer_list<wstring>> shaderModules);
		/* Initializes a new instance of a renderpass with specified subpasses. */
		Renderpass(_In_ LogicalDevice &device, _In_ vector<Subpass> &&subpasses);
		Renderpass(_In_ const Renderpass&) = delete;
		/* Move constructor. */
		Renderpass(_In_ Renderpass &&value);
		/* Releases the resources allocated by the renderpass. */
		~Renderpass(void)
		{
			Destroy();
		}

		_Check_return_ Renderpass& operator =(_In_ const Renderpass&) = delete;
		/* Move assignment. */
		_Check_return_ Renderpass& operator =(_In_ Renderpass &&other);

		/* Preserves the specified output field for the specified subpass. */
		void Preserve(_In_ const Output &field, _In_ uint32 subpass);
		/* Sets the specified output as an input attachment (or depth/stencil) for the specified subpass. */
		void SetAsInput(_In_ const Output &field, _In_ ImageLayout layout, _In_ uint32 subpass);

		/* Gets the subpass at the specified position in the renderpass. */
		_Check_return_ inline Subpass& GetSubpass(_In_ size_t index)
		{
			return subpasses.at(index);
		}

		/* Gets the subpass at the specified position in the renderpass. */
		_Check_return_ inline const Subpass& GetSubpass(_In_ size_t index) const
		{
			return subpasses.at(index);
		}

	protected:
		/* References the renderpass and its underlying shaders and returns itself. */
		virtual Asset& Duplicate(_In_ AssetCache&) override;

	private:
		friend class DescriptorPool;
		friend class GraphicsPipeline;
		friend class AssetFetcher;
		friend class AssetLoader;
		friend class CommandBuffer;
		friend class DescriptorSet;
		friend class GameWindow;
		friend class Framebuffer;

		class LoadTask
			: public Task
		{
		public:
			LoadTask(Renderpass &result, const vector<std::tuple<size_t, size_t, wstring>> &toLoad);

			virtual Result Execute(void) override;
			virtual Result Continue(void) override;

		protected:
			virtual bool ShouldContinue(void) const;

		private:
			Renderpass &renderpass;
			vector<Shader::LoadTask*> children;
		};

		LogicalDevice *device;
		RenderPassHndl hndl;
		PipelineLayoutHndl layoutHndl;
		bool ownsShaders;

		vector<DescriptorSetLayoutHndl> descriptorSetLayouts;
		vector<Subpass> subpasses;
		vector<ClearValue> clearValues;

		std::map<uint32, vector<AttachmentReference>> inputAttachments;
		std::map<uint32, AttachmentReference> depthStencilAttachments;
		std::map<uint32, vector<AttachmentReference>> preserveAttachments;

		void Create(bool viaLoader);
		void CreateRenderpass(void);
		void CreateDescriptorSetLayouts(void);
		void Destroy(void);
	};
}
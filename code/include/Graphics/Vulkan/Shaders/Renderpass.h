#pragma once
#include "Content/Asset.h"
#include "Core/Events/EventBus.h"
#include "Subpass.h"

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
		/* Creates a new instance of a renderpass with specified shader modules for specified subpasses (needs to be initialized). */
		Renderpass(_In_ LogicalDevice &device, _In_ std::initializer_list<std::initializer_list<wstring>> shaderModules);
		/* Creates a new instance of a renderpass with one specific subpass (needs to be initialized). */
		Renderpass(_In_ LogicalDevice &device, _In_ Subpass &&subpass);
		/* Creates a new instance of a renderpass with specified subpasses (needs to be initialized). */
		Renderpass(_In_ LogicalDevice &device, _In_ vector<Subpass> &&subpasses);
		Renderpass(_In_ const Renderpass&) = delete;
		/* Move constructor. */
		Renderpass(_In_ Renderpass &&value);
		/* Releases the resources allocated by the renderpass. */
		virtual ~Renderpass(void)
		{
			Destroy();
		}

		_Check_return_ Renderpass& operator =(_In_ const Renderpass&) = delete;
		/* Move assignment. */
		_Check_return_ Renderpass& operator =(_In_ Renderpass &&other);

		/* Initializes the renderpass, this method is only available for renderpass that aren't initialized yet via either the loader or directly. */
		void Initialize(void);
		/* Adds an external dependency to the end of the renderpass. */
		void AddDependency(_In_ PipelineStageFlag srcStage, _In_ PipelineStageFlag dstStage, _In_ AccessFlag srcAccess, _In_ AccessFlag dstAccess, _In_opt_ DependencyFlag flag = DependencyFlag::None);
		/* Recreates the renderpass after a attachment change. */
		void Recreate(void);

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
		SubpassDependency outputDependency;
		bool ownsShaders, usesDependency;

		vector<Subpass> subpasses;
		vector<ClearValue> clearValues;

		void Create(bool viaLoader);
		void CreateRenderpass(void);
		void CopyReference(vector<vector<AttachmentReference>> &references, size_t i, uint32 refIdx);
		void Destroy(void);
	};
}
#pragma once
#include "Shader.h"
#include "Output.h"
#include "Attribute.h"
#include "Uniform.h"
#include "Core/Events/EventBus.h"
#include "Core/Events/EventArgs.h"

namespace Pu
{
	/* Defines a single Vulkan render pass. */
	class Renderpass
		: public Asset
	{
	public:
		/* Occurs during linking and gives the user the chance to change attachment descriptions. */
		EventBus<Renderpass> OnLinkCompleted;

		/* Initializes an empty instance of a render pass. */
		Renderpass(_In_ LogicalDevice &device);
		/* Initializes a new render pass from the specified subpasses. */
		Renderpass(_In_ LogicalDevice &device, _In_ vector < std::reference_wrapper<Shader>> &&subpasses);
		Renderpass(_In_ const Renderpass&) = delete;
		/* Move contructor. */
		Renderpass(_In_ Renderpass &&value);
		/* Destroys the render pass. */
		virtual ~Renderpass(void)
		{
			Destroy();
		}

		_Check_return_ Renderpass& operator =(_In_ const Renderpass&) = delete;
		/* Move assignment. */
		_Check_return_ Renderpass& operator =(_In_ Renderpass &&other);

		/* Adds a dependency to this rener pass. */
		inline void AddDependency(_In_ const SubpassDependency &dependency)
		{
			dependencies.push_back(dependency);
		}

		/* Adds a depth/stencil buffer to the renderpass. */
		_Check_return_ Output& AddDepthStencil(void);
		/* Gets the specified shader output. */
		_Check_return_ Output& GetOutput(_In_ const string &name);
		/* Gets the specified shader output. */
		_Check_return_ const Output& GetOutput(_In_ const string &name) const;
		/* Gets the specified shader input attribute. */
		_Check_return_ Attribute& GetAttribute(_In_ const string &name);
		/* Gets the specified shader input attribute. */
		_Check_return_ const Attribute& GetAttribute(_In_ const string &name) const;
		/* Gets the specified shader input uniform. */
		_Check_return_ Uniform& GetUniform(_In_ const string &name);
		/* Gets the specified shader input uniform. */
		_Check_return_ const Uniform& GetUniform(_In_ const string &name) const;

	protected:
		/* References the assets and its sub-assets and return itself. */
		virtual Asset& Duplicate(_In_ AssetCache&) override;

	private:
		friend class GraphicsPipeline;
		friend class CommandBuffer;
		friend class Framebuffer;
		friend class GameWindow;
		friend class DescriptorPool;
		friend class AssetLoader;
		friend class AssetFetcher;
		friend struct SavedAsset;

		class LoadTask
			: public Task
		{
		public:
			LoadTask(Renderpass &result, const vector<std::tuple<size_t, wstring>> &toLoad);
			LoadTask(const LoadTask&) = delete;

			LoadTask& operator =(const LoadTask&) = delete;

			virtual Result Execute(void) override;
			virtual Result Continue(void) override;

		private:
			Renderpass &result;
			vector<Shader::LoadTask*> children;
		};

		LogicalDevice &device;
		RenderPassHndl hndl;
		bool usable;

		vector<std::reference_wrapper<Shader>> shaders;
		vector<Attribute> attributes;
		vector<Uniform> uniforms;
		vector<Output> outputs;
		vector<ClearValue> clearValues;
		vector<SubpassDependency> dependencies;

		void Link(bool linkedViaLoader);
		void LoadFields(void);
		void Finalize(bool linkedViaLoader);
		bool CheckIO(const Shader &a, const Shader &b) const;
		void LinkSucceeded(bool linkedViaLoader);
		void LinkFailed(bool linkedViaLoader);
		void Destroy(void);

		/* Needs to be defined for the saved asset. */
		inline Renderpass* Copy(void) { return nullptr; }
	};
}
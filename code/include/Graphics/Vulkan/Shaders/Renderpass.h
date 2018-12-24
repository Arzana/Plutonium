#pragma once
#include "Subpass.h"
#include "Output.h"
#include "Core/Events/EventBus.h"
#include "Core/Events/EventArgs.h"

namespace Pu
{
	/* Defines a single Vulkan render pass. */
	class Renderpass
	{
	public:
		/* Defines a way to load a render pass. */
		class LoadTask
			: public Task
		{
		public:
			/* Initializes a new instance of the render pass load task. */
			LoadTask(_Out_ Renderpass &result, _In_ std::initializer_list<const char*> subpasses);
			LoadTask(_In_ const LoadTask&) = delete;

			_Check_return_ LoadTask& operator =(_In_ const LoadTask&) = delete;

			/* Loads all subpasses. */
			_Check_return_ virtual Result Execute(void) override;
			/* Creates the renderpass and links the subpasses. */
			_Check_return_ virtual Result Continue(void) override;

		private:
			Renderpass &result;
			std::initializer_list<const char*> paths;
			vector<Subpass::LoadTask*> children;
		};

		/* Occurs during linking and gives the user the chance to change attachment descriptions. */
		EventBus<Renderpass, EventArgs> OnAttachmentLink;

		Renderpass(_In_ LogicalDevice &device);
		/* Initializes a new render pass from the specified subpasses. */
		Renderpass(_In_ LogicalDevice &device, _In_ vector<Subpass> &&subpasses);
		Renderpass(_In_ const Renderpass&) = delete;
		/* Move contructor. */
		Renderpass(_In_ Renderpass &&value);
		/* Destroys the render pass. */
		~Renderpass(void)
		{
			Destroy();
		}

		_Check_return_ Renderpass& operator =(_In_ const Renderpass&) = delete;
		/* Move assignment. */
		_Check_return_ Renderpass& operator =(_In_ Renderpass &&other);

		/* Gets whether the renderpass has been loaded. */
		_Check_return_ inline bool IsLoaded(void) const
		{
			return loaded.load();
		}

		/* Gets the specified shader output. */
		_Check_return_ Output& GetOutput(_In_ const string &name);
		/* Gets the specified shader output. */
		_Check_return_ const Output& GetOutput(_In_ const string &name) const;

	private:
		friend class GraphicsPipeline;
		friend class CommandBuffer;
		friend class Framebuffer;

		LogicalDevice &device;
		RenderPassHndl hndl;
		std::atomic_bool loaded;
		bool usable;

		vector<Subpass> subpasses;
		vector<Output> outputs;

		void Link(void);
		bool CheckIO(const Subpass &a, const Subpass &b) const;
		void LinkSucceeded(void);
		void LinkFailed(void);
		void Destroy(void);
	};
}
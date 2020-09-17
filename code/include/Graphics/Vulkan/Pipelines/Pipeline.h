#pragma once
#include "Graphics/Vulkan/Shaders/Subpass.h"
#include "Graphics/Vulkan/Shaders/DescriptorSetLayout.h"

namespace Pu
{
	/* Defines the base class for a Vulkan graphics or compute pipeline. */
	class Pipeline
	{
	public:
		Pipeline(_In_ const Pipeline&) = delete;
		/* Move constructor. */
		Pipeline(_In_ Pipeline &&value);
		/* Releases the resources allocated by the pipeline. */
		virtual ~Pipeline(void)
		{
			FullDestroy();
		}

		_Check_return_ Pipeline& operator =(_In_ const Pipeline&) = delete;
		/* Move assignment. */
		_Check_return_ Pipeline& operator =(_In_ Pipeline &&other);

		/* Finalizes the pipeline, creating the underlying Vulkan resource. */
		virtual void Finalize(void) = 0;
		/* Sets the data for the specialization constants in the specified shader. */
		void SetSpecializationData(_In_ uint32 shader, _In_ const void *data, _In_ size_t size);
		/* Attempts to enable statistics and other debugging information for this pipeline (can slow down pipeline creation). */
		_Check_return_ bool EnableDebugging(void);
		/* Gets the properties of all the executables in this pipeline (requires debugging to be enabled). */
		_Check_return_ const vector<PipelineExecutableProperties>& GetExecutableProperties(void) const;
		/* Gets the statistic for a single executable in this pipeline (requires debugging to be enabled). */
		_Check_return_ const vector<PipelineExecutableStatistic>& GetExecutableStatistics(_In_ uint32 executableIndex) const;
		/* Gets the internal representation of a single executable in this pipeline (requires debugging to be enabled). */
		_Check_return_ const vector<PipelineExecutableInternalRepresentation>& GetExecutableInternals(_In_ uint32 executableIndex) const;

		/* Sets the specialization constants in the specified shader. */
		template <typename data_t>
		inline void SetSpecializationData(_In_ uint32 shader, _In_ const data_t &data)
		{
			SetSpecializationData(shader, &data, sizeof(data_t));
		}

		/* Sets the optional creation flags for this pipeline. */
		inline void SetCreateFlags(_In_ PipelineCreateFlags flags)
		{
			CreateFlags = flags;
		}

		/* Gets whether this pipeline can be bound. */
		_Check_return_ inline bool IsUsable(void) const
		{
			return Hndl;
		}

		/* Sets a debuggable name for the pipeline (only does something on debug mode). */
		inline void SetDebugName(_In_ const string &name) const
		{
#ifdef _DEBUG
			Device->SetDebugName(ObjectType::Pipeline, Hndl, name);
#else
			(void)name;
#endif
		}

	protected:
		/* The raw Vulkan handle to the pipeline. */
		PipelineHndl Hndl;
		/* The raw Vulkan handle to the pipeline layout. */
		PipelineLayoutHndl LayoutHndl;
		/* The logical device on which this pipeline was created. */
		LogicalDevice *Device;
		/* Specifies the create flags that should be used during pipeline creation. */
		PipelineCreateFlags CreateFlags;

		/* Initializes a new instance of a Vulkan pipeline. */
		Pipeline(_In_ LogicalDevice &device, _In_ const ShaderProgram &program);

		/* Gets the shader stages used by the pipeline. */
		_Check_return_ inline const vector<PipelineShaderStageCreateInfo>& GetShaderStages(void) const
		{
			return shaderStages;
		}

		/* Sets the specialization map entries for the pipeline shader stages and validates that all constants have been set. */
		void InitializeSpecializationConstants(_In_ const ShaderProgram &program);
		/* Releases the pipeline. */
		void Destroy(void);

	private:
		friend class CommandBuffer;
		
		vector<SpecializationInfo> specInfos;
		vector<PipelineShaderStageCreateInfo> shaderStages;

#ifdef _DEBUG
		mutable vector<PipelineExecutableProperties> exeProperties;
		mutable std::map<uint32, vector<PipelineExecutableStatistic>> stats;
		mutable std::map<uint32, vector<PipelineExecutableInternalRepresentation>> internals;

		void InitExeProps(void) const;
#endif

		void CreatePipelineLayout(const ShaderProgram &program);
		void DestroyBuffers(void);
		void FullDestroy(void);
	};
}
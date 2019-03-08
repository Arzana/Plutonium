#pragma once
#include "Core/Platform/Windows/Windows.h"
#include "Core/Diagnostics/Logging.h"
#include "Core/Diagnostics/DbgUtils.h"
#include "VulkanFunctions.h"
#include <map>

#define VK_LOAD_EXPORT_PROC(name)					name = Pu::VulkanLoader::LoadExportProc<PFN_##name>(#name)
#define VK_LOAD_GLOBAL_PROC(name)					name = Pu::VulkanLoader::LoadGlobalProc<PFN_##name>(#name)
#define VK_LOAD_INSTANCE_PROC(hndl, name)			name = Pu::VulkanLoader::LoadInstanceProc<PFN_##name>(hndl, #name)
#define VK_LOAD_DEVICE_PROC(instance, hndl, name)	name = Pu::VulkanLoader::LoadDeviceProc<PFN_##name>(instance, hndl, #name)

namespace Pu
{
	class VulkanLoader
	{
	public:
		VulkanLoader(_In_ const VulkanLoader &) = delete;
		VulkanLoader(_In_ VulkanLoader &&) = delete;
		/* Releases the Vulkan library dll. */
		~VulkanLoader(void) noexcept
		{
			if (libHndl)
			{
#ifdef _WIN32
				FreeLibrary(libHndl);
#endif
			}
		}

		_Check_return_ VulkanLoader& operator =(_In_ const VulkanLoader &) = delete;
		_Check_return_ VulkanLoader& operator =(_In_ VulkanLoader &&) = delete;

		/* Retrieves the address of an exported Vulkan function. */
		template <typename _ProcTy>
		_Check_return_ static _ProcTy LoadExportProc(_In_ const char *name)
		{
			const _ProcTy proc = reinterpret_cast<_ProcTy>(GetProcAddress(GetInstance().libHndl, name));
			if (!proc) Log::Error("Unable to load Vulkan export procedure '%s'!", name);
			return proc;
		}

		/* Retrieves the address of an global instance level Vulkan function. */
		template <typename _ProcTy>
		_Check_return_ static _ProcTy LoadGlobalProc(_In_ const char *name)
		{
			const _ProcTy proc = reinterpret_cast<_ProcTy>(GetInstance().vkGetInstanceProcAddr(nullptr, name));
			if (!proc) Log::Error("Unable to load Vulkan instance global procedure '%s'!", name);
			return proc;
		}

		/* Retrieves the address of an instance level Vulkan function. */
		template <typename _ProcTy>
		_Check_return_ static _ProcTy LoadInstanceProc(_In_ InstanceHndl hndl, _In_ const char *name)
		{
			const _ProcTy proc = reinterpret_cast<_ProcTy>(GetInstance().vkGetInstanceProcAddr(hndl, name));
			if (!proc) Log::Error("Unable to load Vulkan instance procedure '%s'!", name);
			return proc;
		}

		/* Retrieves the address of a device level Vulkan function. */
		template <typename _ProcTy>
		_Check_return_ static _ProcTy LoadDeviceProc(_In_ InstanceHndl instance, _In_ DeviceHndl hndl, _In_ const char *name)
		{
			const VulkanLoader &loader = GetInstance();
			std::map<InstanceHndl, PFN_vkGetDeviceProcAddr>::const_iterator it = loader.vkGetDeviceProcAddr.find(instance);
			if (it != loader.vkGetDeviceProcAddr.end())
			{
				const _ProcTy proc = reinterpret_cast<_ProcTy>(it->second(hndl, name));
				if (!proc) Log::Error("Unable to load Vulkan device procedure '%s'!", name);
				return proc;
			}
			else
			{
				Log::Error("Device procedure '%s' is requested from unknown Vulkan instance!", name);
				return nullptr;
			}
		}

	private:
		friend class VulkanInstance;

		using PFN_vkGetInstanceProcAddr = VoidFunction(VKAPI_PTR)(InstanceHndl instance, const char *name);
		using PFN_vkGetDeviceProcAddr = VoidFunction(VKAPI_PTR)(DeviceHndl device, const char *name);

		PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
		std::map<InstanceHndl, PFN_vkGetDeviceProcAddr> vkGetDeviceProcAddr;

#ifdef _WIN32
		HMODULE libHndl;

		VulkanLoader(void)
			: libHndl(LoadLibrary(L"vulkan-1.dll"))
		{
			/* Check if the vulkan dll was loaded correctly. */
			if (libHndl) LoadProcAddr(); 
			else Log::Fatal("Unable to load dynamic Vulkan link library, reason: '%ls'!", _CrtGetErrorString().c_str());
		}
#else
#error "Plutonium Vulkan loader currently doesn't work on this platform!"
#endif

		static inline VulkanLoader& GetInstance(void)
		{
			static VulkanLoader loader;
			return loader;
		}

		void LoadProcAddr()
		{
			/* Load the Vulkan procedure loading function, this is done inline to avoid a deadlock in the ctor. */
			vkGetInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(GetProcAddress(libHndl, "vkGetInstanceProcAddr"));
			if (!vkGetInstanceProcAddr) Log::Fatal("Unable to load vital function vkGetInstanceProcAddr!");
		}

		void AddDeviceProcAddr(InstanceHndl instance)
		{
			if (vkGetDeviceProcAddr.find(instance) == vkGetDeviceProcAddr.end())
			{
				PFN_vkGetDeviceProcAddr proc = LoadInstanceProc<PFN_vkGetDeviceProcAddr>(instance, "vkGetDeviceProcAddr");
				if (!proc) Log::Fatal("Unable to load vital function vkGetDeviceProcAddr!");
				vkGetDeviceProcAddr.emplace(instance, proc);
			}
		}
	};
}
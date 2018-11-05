#include "Graphics/Vulkan/Instance.h"

using namespace Pu;

VulkanInstance::PFN_vkEnumerateInstanceVersion Pu::VulkanInstance::vkEnumerateInstanceVersion = nullptr;
VulkanInstance::PFN_vkEnumerateInstanceExtensionProperties Pu::VulkanInstance::vkEnumerateInstanceExtensionProperties = nullptr;
VulkanInstance::PFN_vkEnumerateInstanceLayerProperties Pu::VulkanInstance::vkEnumerateInstanceLayerProperties = nullptr;
VulkanInstance::PFN_vkCreateInstance Pu::VulkanInstance::vkCreateInstance = nullptr;

Pu::VulkanInstance::VulkanInstance(const char * applicationName, int32 major, int32 minor, int32 patch)
	: hndl(nullptr), vkDestroyInstance(nullptr)
{
	/* Make sure the create procedure is loaded. */
	LoadStaticProcs();

	/* Create application info and instance info. */
	const ApplicationInfo appInfo(applicationName, major, minor, patch, u8"Plutonium", 0, 1, 0);
	const InstanceCreateInfo createInfo(&appInfo);

	/* Create a new instance handle. */
	const Result result = vkCreateInstance(&createInfo, nullptr, &hndl);
	if (result != Result::Success) Log::Fatal("Unable to create Vulkan instance!");

	/* Load the procedures needed for the instance. */
	LoadInstanceProcs();
}

Pu::VulkanInstance::VulkanInstance(VulkanInstance && value)
{
	if (value.hndl != hndl)
	{
		Destroy();

		hndl = hndl;
		vkDestroyInstance = value.vkDestroyInstance;

		value.hndl = nullptr;
		value.vkDestroyInstance = nullptr;
	}
}

Pu::VulkanInstance & Pu::VulkanInstance::operator=(VulkanInstance && other)
{
	if (&other != this)
	{
		Destroy();

		hndl = other.hndl;
		vkDestroyInstance = other.vkDestroyInstance;

		other.hndl = nullptr;
		other.vkDestroyInstance = nullptr;
	}

	return *this;
}

void Pu::VulkanInstance::GetSupportedVersion(uint32 & major, uint32 & minor, uint32 & patch)
{
	/* Make sure the version enumerate procedure is loaded. */
	LoadStaticProcs();

	/* Query supported version, function cannot crash if a valid pointer is passed so just ignore the result. */
	uint32 version;
	vkEnumerateInstanceVersion(&version);

	/* Set versions to appropriate values. */
	major = getMajor(version);
	minor = getMinor(version);
	patch = getPatch(version);
}

vector<ExtensionProperties> Pu::VulkanInstance::GetSupportedExtensions(const char * layer)
{
	/* Make sure the extension enumerate procedure is loaded. */
	LoadStaticProcs();

	/* Query the amount of properties defined. */
	uint32 count;
	vkEnumerateInstanceExtensionProperties(layer, &count, nullptr);

	/* Query all extension properties. */
	vector<ExtensionProperties> result(count);
	vkEnumerateInstanceExtensionProperties(layer, &count, result.data());

	return result;
}

vector<LayerProperties> Pu::VulkanInstance::GetSupportedLayers(void)
{
	/* Make sure the layer enumerate procedure is loaded. */
	LoadStaticProcs();

	/* Query the amount of layers defined. */
	uint32 count;
	vkEnumerateInstanceLayerProperties(&count, nullptr);

	/* Query all layer properties. */
	vector<LayerProperties> result(count);
	vkEnumerateInstanceLayerProperties(&count, result.data());

	return result;
}

void Pu::VulkanInstance::LoadStaticProcs(void)
{
	static bool loaded = false;
	if (!loaded)
	{
		loaded = true;

		VK_LOAD_EXPORT_PROC(vkCreateInstance);

		VK_LOAD_GLOBAL_PROC(vkEnumerateInstanceVersion);
		VK_LOAD_GLOBAL_PROC(vkEnumerateInstanceExtensionProperties);
		VK_LOAD_GLOBAL_PROC(vkEnumerateInstanceLayerProperties);
	}
}

void Pu::VulkanInstance::Destroy(void)
{
	if (hndl) vkDestroyInstance(hndl, nullptr);
}

void Pu::VulkanInstance::LoadInstanceProcs(void)
{
	vkDestroyInstance = VulkanLoader::LoadInstanceProc<PFN_vkDestroyInstance>(hndl, "vkDestroyInstance");
}
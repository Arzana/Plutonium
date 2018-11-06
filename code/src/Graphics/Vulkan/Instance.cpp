#include "Graphics/Vulkan/Instance.h"
#include "Graphics/Vulkan/Loader.h"

using namespace Pu;

PFN_vkEnumerateInstanceVersion Pu::VulkanInstance::vkEnumerateInstanceVersion = nullptr;
PFN_vkEnumerateInstanceExtensionProperties Pu::VulkanInstance::vkEnumerateInstanceExtensionProperties = nullptr;
PFN_vkEnumerateInstanceLayerProperties Pu::VulkanInstance::vkEnumerateInstanceLayerProperties = nullptr;
PFN_vkCreateInstance Pu::VulkanInstance::vkCreateInstance = nullptr;

Pu::VulkanInstance::VulkanInstance(const char * applicationName, int32 major, int32 minor, int32 patch)
	: hndl(nullptr), vkDestroyInstance(nullptr), vkEnumeratePhysicalDevices(nullptr),
	OnDestroy("VulkanInstance::OnDestory")
{
	/* Make sure the create procedure is loaded. */
	LoadStaticProcs();

	/* Create application info and instance info. */
	const ApplicationInfo appInfo(applicationName, major, minor, patch, u8"Plutonium", 0, 1, 0);
	const InstanceCreateInfo createInfo(&appInfo);

	/* Create a new instance handle. */
	const VkApiResult result = vkCreateInstance(&createInfo, nullptr, &hndl);
	if (result != VkApiResult::Success) Log::Fatal("Unable to create Vulkan instance!");

	/* Load the procedures needed for the instance. */
	LoadInstanceProcs();
}

Pu::VulkanInstance::VulkanInstance(VulkanInstance && value)
	: hndl(value.hndl), vkDestroyInstance(value.vkDestroyInstance), vkEnumeratePhysicalDevices(vkEnumeratePhysicalDevices),
	OnDestroy(std::move(value.OnDestroy))
{
	value.hndl = nullptr;
	value.vkDestroyInstance = nullptr;
	value.vkEnumeratePhysicalDevices = nullptr;
}

VulkanInstance & Pu::VulkanInstance::operator=(VulkanInstance && other)
{
	if (this != &other)
	{
		Destroy();
		hndl = other.hndl;
		vkDestroyInstance = other.vkDestroyInstance;
		vkEnumeratePhysicalDevices = other.vkEnumeratePhysicalDevices;
		OnDestroy = std::move(other.OnDestroy);

		other.hndl = nullptr;
		other.vkDestroyInstance = nullptr;
		other.vkEnumeratePhysicalDevices = nullptr;
	}

	return *this;
}

std::tuple<uint32, uint32, uint32> Pu::VulkanInstance::GetSupportedVersion()
{
	/* Make sure the version enumerate procedure is loaded. */
	LoadStaticProcs();

	/* Query supported version, function cannot crash if a valid pointer is passed so just ignore the result. */
	uint32 version;
	vkEnumerateInstanceVersion(&version);

	/* Set versions to appropriate values. */
	return std::make_tuple(getMajor(version), getMinor(version), getPatch(version));
}

vector<ExtensionProperties> Pu::VulkanInstance::GetSupportedExtensions(const char * layer)
{
	/* Make sure the extension enumerate procedure is loaded. */
	LoadStaticProcs();

	/* Query the amount of properties defined. */
	uint32 count;
	vkEnumerateInstanceExtensionProperties(layer, &count, nullptr);

	/* Early out if the count is less than one. */
	if (count < 1) return vector<ExtensionProperties>();

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

	/* Early out if the count is less than one. */
	if (count < 1) return vector<LayerProperties>();

	/* Query all layer properties. */
	vector<LayerProperties> result(count);
	vkEnumerateInstanceLayerProperties(&count, result.data());

	return result;
}

vector<PhysicalDevice> Pu::VulkanInstance::GetPhysicalDevices(void) const
{
	/* Query the amount of physical devices available. */
	uint32 count;
	vkEnumeratePhysicalDevices(hndl, &count, nullptr);

	/* Early out if the count is less than one. */
	if (count < 1) return vector<PhysicalDevice>();

	/* Query all physical device handles. */
	vector<PhysicalDeviceHndl> raw(count);
	vkEnumeratePhysicalDevices(hndl, &count, raw.data());

	/* Convert the handles to objects and return. */
	vector<PhysicalDevice> result;
	VulkanInstance &self = *const_cast<VulkanInstance*>(this);
	for (PhysicalDeviceHndl cur : raw) result.push_back(PhysicalDevice(self, cur));
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
	OnDestroy.Post(*this, EventArgs());
	if (hndl) vkDestroyInstance(hndl, nullptr);
}

void Pu::VulkanInstance::LoadInstanceProcs(void)
{
	VK_LOAD_INSTANCE_PROC(hndl, vkEnumeratePhysicalDevices);
	VK_LOAD_INSTANCE_PROC(hndl, vkDestroyInstance);
	VK_LOAD_INSTANCE_PROC(hndl, vkGetPhysicalDeviceProperties);
	VK_LOAD_INSTANCE_PROC(hndl, vkGetPhysicalDeviceFeatures);
	VK_LOAD_INSTANCE_PROC(hndl, vkGetPhysicalDeviceQueueFamilyProperties);
}
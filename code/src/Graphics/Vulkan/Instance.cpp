#include "Graphics/Vulkan/Instance.h"
#include "Graphics/Vulkan/Loader.h"

using namespace Pu;

PFN_vkEnumerateInstanceVersion Pu::VulkanInstance::vkEnumerateInstanceVersion = nullptr;
PFN_vkEnumerateInstanceExtensionProperties Pu::VulkanInstance::vkEnumerateInstanceExtensionProperties = nullptr;
PFN_vkEnumerateInstanceLayerProperties Pu::VulkanInstance::vkEnumerateInstanceLayerProperties = nullptr;
PFN_vkCreateInstance Pu::VulkanInstance::vkCreateInstance = nullptr;

Pu::VulkanInstance::VulkanInstance(const char * applicationName, std::initializer_list<const char*> extensions, int32 major, int32 minor, int32 patch)
	: hndl(nullptr), vkDestroyInstance(nullptr), vkEnumeratePhysicalDevices(nullptr),
	OnDestroy("VulkanInstance::OnDestory")
{
	/* Make sure the create procedure is loaded. */
	LoadStaticProcs();

#ifdef _DEBUG
	if constexpr (LogAvailableVulkanExtensionsAndLayers) LogAvailableExtensionsAndLayers();
#endif

	/* Create application info and instance info. */
	const ApplicationInfo appInfo(applicationName, major, minor, patch, u8"Plutonium", 0, 1, 0);
	InstanceCreateInfo createInfo(&appInfo);
	createInfo.EnabledExtensionCount = static_cast<uint32>(extensions.size());
	createInfo.EnabledExtensionNames = extensions.begin();

	/* TODO: REMOVE! */
	static const char *layer = "VK_LAYER_LUNARG_standard_validation";
	createInfo.EnabledLayerCount = 1;
	createInfo.EnabledLayerNames = &layer;

	/* Create a new instance handle. */
	VK_VALIDATE(vkCreateInstance(&createInfo, nullptr, &hndl), PFN_vkCreateInstance);

	/* Add the instance to the procedure loader. */
	VulkanLoader::GetInstance().AddDeviceProcAddr(hndl);

	/* Load the procedures needed for the instance and get all physical devices. */
	LoadInstanceProcs();
	GetPhysicalDevices();

	/* If needed setup the debug layer. */
#ifdef _DEBUG
	SetUpDebugLayer();
#endif
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

bool Pu::VulkanInstance::IsExtensionSupported(const char * extension)
{
	const vector<ExtensionProperties> properties = GetSupportedExtensions(nullptr);
	return properties.contains(extension, [](const ExtensionProperties &prop, const char *userParam)
	{
		return !strcmp(prop.ExtensionName, userParam);
	});
}

bool Pu::VulkanInstance::AreExtensionsSupported(std::initializer_list<const char*> extensions)
{
	const vector<ExtensionProperties> properties = GetSupportedExtensions(nullptr);

	size_t found = 0;
	for (const char *extension : extensions)
	{
		for (const ExtensionProperties &prop : properties)
		{
			if (!strcmp(prop.ExtensionName, extension))
			{
				++found;
				break;
			}
		}
	}

	return found >= extensions.size();
}

void Pu::VulkanInstance::GetPhysicalDevices(void)
{
	/* Query the amount of physical devices available. */
	uint32 count;
	vkEnumeratePhysicalDevices(hndl, &count, nullptr);

	/* Early out if the count is less than one. */
	if (count < 1) return;

	/* Query all physical device handles. */
	vector<PhysicalDeviceHndl> raw(count);
	vkEnumeratePhysicalDevices(hndl, &count, raw.data());

	/* Convert the handles to objects and return. */
	for (PhysicalDeviceHndl cur : raw) physicalDevices.emplace_back(PhysicalDevice(*this, cur));
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
	physicalDevices.clear();
	OnDestroy.Post(*this, EventArgs());

#ifdef _DEBUG
	if (msgHndl) vkDestroyDebugUtilsMessengerEXT(hndl, msgHndl, nullptr);
#endif

	if (hndl)
	{
		/* Remove the instance from the loader list and destory the instance. */
		VulkanLoader::GetInstance().vkGetDeviceProcAddr.erase(hndl);
		vkDestroyInstance(hndl, nullptr);
	}
}

void Pu::VulkanInstance::LoadInstanceProcs(void)
{
	/* Physical device related functions. */
	VK_LOAD_INSTANCE_PROC(hndl, vkEnumeratePhysicalDevices);
	VK_LOAD_INSTANCE_PROC(hndl, vkDestroyInstance);
	VK_LOAD_INSTANCE_PROC(hndl, vkGetPhysicalDeviceProperties);
	VK_LOAD_INSTANCE_PROC(hndl, vkGetPhysicalDeviceFeatures);
	VK_LOAD_INSTANCE_PROC(hndl, vkGetPhysicalDeviceQueueFamilyProperties);

	/* Device related functions. */
	VK_LOAD_INSTANCE_PROC(hndl, vkCreateDevice);
	VK_LOAD_INSTANCE_PROC(hndl, vkEnumerateDeviceExtensionProperties);

	/* Surface extension functions. */
	if (IsExtensionSupported(u8"VK_KHR_surface"))
	{
		VK_LOAD_INSTANCE_PROC(hndl, vkDestroySurfaceKHR);
		VK_LOAD_INSTANCE_PROC(hndl, vkGetPhysicalDeviceSurfaceSupportKHR);
		VK_LOAD_INSTANCE_PROC(hndl, vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
		VK_LOAD_INSTANCE_PROC(hndl, vkGetPhysicalDeviceSurfaceFormatsKHR);
		VK_LOAD_INSTANCE_PROC(hndl, vkGetPhysicalDeviceSurfacePresentModesKHR);
	}
	else Log::Warning("Surface extension is not supported on this platform!");

#ifdef _WIN32
	if (IsExtensionSupported(u8"VK_KHR_win32_surface"))
	{
		VK_LOAD_INSTANCE_PROC(hndl, vkCreateWin32SurfaceKHR);
	}
	else Log::Warning("Win32 Create surface extension is not supported by the graphics driver!");
#endif

	/* Debug utilities functions. */
	if (IsExtensionSupported(u8"VK_EXT_debug_utils"))
	{
		VK_LOAD_INSTANCE_PROC(hndl, vkCreateDebugUtilsMessengerEXT);
		VK_LOAD_INSTANCE_PROC(hndl, vkDestroyDebugUtilsMessengerEXT);
	}
	else Log::Warning("Debug utilities is not supported on this platform!");
}

#ifdef _DEBUG
VKAPI_ATTR Bool32 VKAPI_CALL Pu::VulkanInstance::DebugCallback(DebugUtilsMessageSeverityFlag severity, DebugUtilsMessageTypeFlag, const DebugUtilsMessengerCallbackData * data, void *)
{
	switch (severity)
	{
	case Pu::DebugUtilsMessageSeverityFlag::Info:
		//Log::Message(data->Message);
		break;
	case Pu::DebugUtilsMessageSeverityFlag::Warning:
		Log::Warning(data->Message);
		break;
	case Pu::DebugUtilsMessageSeverityFlag::Error:
		Log::Error(data->Message);
		break;
	}

	return false;
}

void Pu::VulkanInstance::SetUpDebugLayer(void)
{
	DebugUtilsMessengerCreateInfo createInfo(VulkanInstance::DebugCallback);
	VK_VALIDATE(vkCreateDebugUtilsMessengerEXT(hndl, &createInfo, nullptr, &msgHndl), PFN_vkCreateDebugUtilsMessenger);
}

void Pu::VulkanInstance::LogAvailableExtensionsAndLayers(void) const
{
	static bool logged = false;

	/* Only log extensions once. */
	if (!logged)
	{
		logged = true;
		const string barStr = string(64, '-') + '\n';

		/* Log everything in one message so other threads can't interfere. */
		string msg = '\n' + barStr;

		const vector<ExtensionProperties> extensions = GetSupportedExtensions(nullptr);
		for (const ExtensionProperties &extension : extensions)
		{
			msg += "Extension ";
			msg += extension.ExtensionName;
			msg += " (";
			msg += std::to_string(getMajor(extension.SpecVersion)) += '.';
			msg += std::to_string(getMinor(extension.SpecVersion)) += '.';
			msg += std::to_string(getPatch(extension.SpecVersion)) += ") available.\n";
		}

		msg += barStr;

		/* Log the layers too bewteen lines. */
		const vector<LayerProperties> layers = GetSupportedLayers();
		for (const LayerProperties &layer : layers)
		{
			msg += "Layer ";
			msg += layer.LayerName;
			msg += " (";
			msg += std::to_string(getMajor(layer.ImplementationVersion)) += '.';
			msg += std::to_string(getMinor(layer.ImplementationVersion)) += '.';
			msg += std::to_string(getPatch(layer.ImplementationVersion)) += ") available.\n";
		}

		msg += barStr;
		Log::Verbose(msg.c_str());
	}
}
#endif
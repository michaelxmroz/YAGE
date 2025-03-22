#ifndef VOLK_IMPLEMENTATION 
#define VOLK_IMPLEMENTATION 
#endif

#include "volk.h"
#include "PlatformDefines.h"

// Platform-specific Vulkan headers
#if YAGE_PLATFORM_WINDOWS
#include <vulkan/vulkan_win32.h>
#elif YAGE_PLATFORM_UNIX
#include <vulkan/vulkan_xlib.h>
#endif

#include "RendererVulkan.h"
#include "Logging.h"
#include <vector>
#include <set>
#include <algorithm>
#include <array>
#include "FileParser.h"
#include "ShaderCompiler.h"

#if _DEBUG

#define VK_CHECK(x)                                                 \
	do                                                              \
	{                                                               \
		VkResult err = x;                                           \
		if (err)                                                    \
		{                                                           \
			LOG_ERROR(string_format("Detected Vulkan error: %s\n", std::to_string(static_cast<int32_t>(err)).c_str()).c_str()); \
			abort();                                                \
		}                                                           \
	} while (0)

#else
#define VK_CHECK(x) { x; }
#endif


void ResizeWindowProcHandler(void* userData, bool isMinimizing)
{
	RendererVulkan* renderer = static_cast<RendererVulkan*>(userData);
	if (isMinimizing)
	{
		renderer->PauseRendering();
	}
	else
	{
		renderer->ResumeRendering();
	}
}

namespace
{
	struct SwapChainSupportDetails 
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

	struct QueueFamilyIndices
	{
		enum class QueueTypes
		{
			Graphics = 0,
			Present = 1,
			COUNT = 2
		};

		QueueFamilyIndices() :
			m_isPresent{ false }
		{
		}

		bool IsComplete()
		{
			bool complete = true;
			for (uint32_t i = 0; i < static_cast<uint32_t>(QueueTypes::COUNT); ++i)
			{
				complete &= m_isPresent[i];
			}

			return complete;
		}

		bool IsFamilyPresent(QueueTypes type)
		{
			return m_isPresent[static_cast<uint32_t>(type)];
		}

		uint32_t GetFamilyIndex(QueueTypes type)
		{
			return m_index[static_cast<uint32_t>(type)];
		}

		bool m_isPresent[static_cast<uint32_t>(QueueTypes::COUNT)];
		uint32_t m_index[static_cast<uint32_t>(QueueTypes::COUNT)];
	};

#if VALIDATION_LAYERS
	VKAPI_ATTR VkBool32 VKAPI_CALL ValidationLogCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData) {

		if(messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		{
			LOG_ERROR(pCallbackData->pMessage);
		}
		else if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		{
			LOG_WARNING(pCallbackData->pMessage);
		}

		return VK_FALSE;
	}

	void InitValidationCallback(VkInstance instance, VkDebugUtilsMessengerEXT& messenger)
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		createInfo.pfnUserCallback = ValidationLogCallback;
		createInfo.pUserData = nullptr; // Optional

		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		VK_CHECK(func(instance, &createInfo, nullptr, &messenger));
	}
#endif

	bool ValidateExtensions(const std::vector<const char*>& required,
		const std::vector<VkExtensionProperties>& available)
	{
		for (auto extension : required)
		{
			bool found = false;
			for (auto& available_extension : available)
			{
				if (strcmp(available_extension.extensionName, extension) == 0)
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				return false;
			}
		}

		return true;
	}

	bool ValidateLayers(const std::vector<const char*>& required,
		const std::vector<VkLayerProperties>& available)
	{
		for (auto extension : required)
		{
			bool found = false;
			for (auto& available_extension : available)
			{
				if (strcmp(available_extension.layerName, extension) == 0)
				{
					found = true;
					break;
				}
			}

			if (!found)
			{
				return false;
			}
		}

		return true;
	}

	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	void FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface, QueueFamilyIndices& indices)
	{
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

		int i = 0;
		for (const auto& queueFamily : queueFamilies) 
		{
			if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) 
			{
				indices.m_isPresent[static_cast<uint32_t>(QueueFamilyIndices::QueueTypes::Graphics)] = true;
				indices.m_index[static_cast<uint32_t>(QueueFamilyIndices::QueueTypes::Graphics)] = i;
			}
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
			if (presentSupport)
			{
				indices.m_isPresent[static_cast<uint32_t>(QueueFamilyIndices::QueueTypes::Present)] = true;
				indices.m_index[static_cast<uint32_t>(QueueFamilyIndices::QueueTypes::Present)] = i;
			}

			if (indices.IsComplete())
			{
				break;
			}
			i++;
		}
	}

	bool CheckDeviceExtensionSupport(VkPhysicalDevice device) 
	{
		const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		for (const auto& extension : availableExtensions) {
			requiredExtensions.erase(extension.extensionName);
		}

		return requiredExtensions.empty();
	}

	bool IsDeviceSupported(VkPhysicalDevice device, VkSurfaceKHR surface)
	{
		QueueFamilyIndices indices;
		FindQueueFamilies(device, surface, indices);

		bool extensionsSupported = CheckDeviceExtensionSupport(device);

		bool swapChainAdequate = false;
		if (extensionsSupported) {
			SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device, surface);
			swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
		}

		return indices.IsComplete() && swapChainAdequate;
	}

	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height) 
	{
		if (capabilities.currentExtent.width != UINT32_MAX) {
			return capabilities.currentExtent;
		}
		else {
			VkExtent2D actualExtent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			return actualExtent;
		}
	}

	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) 
	{
		return VK_PRESENT_MODE_IMMEDIATE_KHR;
	}

	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) 
	{
		for (const auto& availableFormat : availableFormats) 
		{
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_DISPLAY_P3_LINEAR_EXT)
			{
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	VkShaderModule CreateShaderModule(VkDevice logicalDevice, const std::vector<uint32_t>& code) 
	{
		VkShaderModuleCreateInfo createInfo { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
		createInfo.codeSize = code.size() * 4;
		createInfo.pCode = code.data();

		VkShaderModule shaderModule;
		VK_CHECK(vkCreateShaderModule(logicalDevice, &createInfo, nullptr, &shaderModule));
		return shaderModule;
	}

	VkVertexInputBindingDescription GetVertexBindingDescription() 
	{
		VkVertexInputBindingDescription bindingDescription{};
		bindingDescription.binding = 0;
		bindingDescription.stride = sizeof(Vertex);
		bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescription;
	}

	std::array<VkVertexInputAttributeDescription, 2> GetVertexAttributeDescriptions() 
	{
		std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, m_pos);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, m_uv);

		return attributeDescriptions;
	}

	uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) 
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) 
		{
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) 
			{
				return i;
			}
		}

		return 0;
	}

	void CreateBuffer(VkDevice logicalDevice, VkPhysicalDevice physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) 
	{
		VkBufferCreateInfo bufferInfo { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
		bufferInfo.size = size;
		bufferInfo.usage = usage;
		bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		VK_CHECK(vkCreateBuffer(logicalDevice, &bufferInfo, nullptr, &buffer));

		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(logicalDevice, buffer, &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = FindMemoryType(physicalDevice, memRequirements.memoryTypeBits, properties);

		VK_CHECK(vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &bufferMemory));

		vkBindBufferMemory(logicalDevice, buffer, bufferMemory, 0);
	}

	void TransitionImageLayout(VkCommandBuffer buffer, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		VkImageMemoryBarrier barrier{ VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if (oldLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else 
		{
			throw std::invalid_argument("unsupported layout transition!");
		}


		vkCmdPipelineBarrier(
			buffer,
			sourceStage, 
			destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);
	}

	void CopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
	{
		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;

		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = {
			width,
			height,
			1
		};

		vkCmdCopyBufferToImage(
			commandBuffer,
			buffer,
			image,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region
		);
	}
}

RendererVulkan::RendererVulkan(StateMachine& stateMachine, uint32_t sourceWidth, uint32_t sourceHeight, uint32_t scale)
	: m_instance(VK_NULL_HANDLE)
	, m_physicalDevice(VK_NULL_HANDLE)
	, m_logicalDevice(VK_NULL_HANDLE)
	, m_graphicsQueue(VK_NULL_HANDLE)
	, m_surface(VK_NULL_HANDLE)
#if VALIDATION_LAYERS
	, m_debugMessenger(VK_NULL_HANDLE)
#endif
	, m_sourceWidth(sourceWidth)
	, m_sourceHeight(sourceHeight)
	, m_scaledWidth(sourceWidth * scale)
	, m_scaledHeight(sourceHeight * scale)
	, m_shouldResize(false)
	, m_shouldRender(true)
	, m_stateMachine(stateMachine)
	, m_mainShaderName("main.glsl")
	, m_fullscreenQuadVertices {{{-1,-1,0},{0,0}}, {{1,1,0},{1,1}}, {{-1,1,0},{0,1}}, {{1,-1,0},{1,0}} }
	, m_fullscreenQuadIndices {0, 1, 2, 0, 3, 1}
{
	Init();
}

RendererVulkan::~RendererVulkan()
{
	vkDestroySemaphore(m_logicalDevice, m_renderFinishedSemaphore, nullptr);
	vkDestroySemaphore(m_logicalDevice, m_imageAvailableSemaphore, nullptr);

	vkDestroyBuffer(m_logicalDevice, m_vertexBuffer, nullptr);
	vkFreeMemory(m_logicalDevice, m_vertexBufferMemory, nullptr);

	vkDestroyBuffer(m_logicalDevice, m_indexBuffer, nullptr);
	vkFreeMemory(m_logicalDevice, m_indexBufferMemory, nullptr);

	vkDestroyBuffer(m_logicalDevice, m_imageUploadBuffer, nullptr);
	vkFreeMemory(m_logicalDevice, m_imageUploadBufferMemory, nullptr);

	vkDestroySampler(m_logicalDevice, m_textureSampler, nullptr);
	vkDestroyImageView(m_logicalDevice, m_textureImageView, nullptr);
	vkDestroyImage(m_logicalDevice, m_textureImage, nullptr);
	vkFreeMemory(m_logicalDevice, m_textureImageMemory, nullptr);

	vkDestroyCommandPool(m_logicalDevice, m_commandPool, nullptr);



	vkDestroyPipeline(m_logicalDevice, m_graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(m_logicalDevice, m_pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(m_logicalDevice, m_descriptorSetLayout, nullptr);
	vkDestroyDescriptorPool(m_logicalDevice, m_descriptorPool, nullptr);

	vkDestroyRenderPass(m_logicalDevice, m_renderPass, nullptr);

	CleanupSwapChain();

	vkDestroyDevice(m_logicalDevice, nullptr);

#if VALIDATION_LAYERS
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT");
	func(m_instance, m_debugMessenger, nullptr);
#endif

	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	vkDestroyInstance(m_instance, nullptr);
	m_backend.CleanupWindow();
}

void RendererVulkan::CreateInstance()
{
	uint32_t instance_extension_count;
	VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, nullptr));

	std::vector<VkExtensionProperties> instance_extensions(instance_extension_count);
	VK_CHECK(vkEnumerateInstanceExtensionProperties(nullptr, &instance_extension_count, instance_extensions.data()));

	std::vector<const char*> active_instance_extensions;

	// Common surface extension needed for all platforms
	active_instance_extensions.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
	
	// Platform-specific surface extension
#if YAGE_PLATFORM_WINDOWS
	active_instance_extensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif YAGE_PLATFORM_UNIX
	active_instance_extensions.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
#endif

#if VALIDATION_LAYERS
	active_instance_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

	if (!ValidateExtensions(active_instance_extensions, instance_extensions))
	{
		throw std::runtime_error("Required instance extensions are missing.");
	}

	uint32_t instance_layer_count;
	VK_CHECK(vkEnumerateInstanceLayerProperties(&instance_layer_count, nullptr));

	std::vector<VkLayerProperties> supported_validation_layers(instance_layer_count);
	VK_CHECK(vkEnumerateInstanceLayerProperties(&instance_layer_count, supported_validation_layers.data()));

	VkApplicationInfo appInfo
	{
		VkStructureType::VK_STRUCTURE_TYPE_APPLICATION_INFO,
		NULL,
		"YAGE",
		VK_MAKE_VERSION(1,0,0),
		"YAGEEngine",
		VK_MAKE_VERSION(1,0,0),
		VK_API_VERSION_1_2
	};

	VkInstanceCreateInfo instanceInfo
	{
		VkStructureType::VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO
	};

	std::vector<const char*> validationLayers;

#if VALIDATION_LAYERS
	validationLayers.push_back("VK_LAYER_KHRONOS_validation");

	if (ValidateLayers(validationLayers, supported_validation_layers))
	{
		LOG_INFO("Enabled Validation Layers:")
			for (const auto& layer : validationLayers)
			{
				LOG_INFO(layer);
			}
	}
	else
	{
		throw std::runtime_error("Required validation layers are missing.");
	}
#endif

	instanceInfo.pApplicationInfo = &appInfo;
	instanceInfo.enabledExtensionCount = static_cast<uint32_t>(active_instance_extensions.size());
	instanceInfo.ppEnabledExtensionNames = active_instance_extensions.data();
	instanceInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
	instanceInfo.ppEnabledLayerNames = validationLayers.data();

	VK_CHECK(vkCreateInstance(&instanceInfo, nullptr, &m_instance));

	volkLoadInstanceOnly(m_instance);
}

void RendererVulkan::SelectPhysicalDevice()
{
	uint32_t deviceCount = 0;
	VK_CHECK(vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr));

	if (deviceCount == 0) {
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	VK_CHECK(vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data()));

	for (const auto& device : devices) {
		if (IsDeviceSupported(device, m_surface)) {
			m_physicalDevice = device;
			break;
		}
	}

	if (m_physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

void RendererVulkan::CreateLogicalDevice()
{
	QueueFamilyIndices indices;
	FindQueueFamilies(m_physicalDevice, m_surface, indices);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.GetFamilyIndex(QueueFamilyIndices::QueueTypes::Graphics), indices.GetFamilyIndex(QueueFamilyIndices::QueueTypes::Present) };

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{ VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};

	VkDeviceCreateInfo createInfo{ VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };

	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	createInfo.pEnabledFeatures = &deviceFeatures;

	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	VK_CHECK(vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_logicalDevice));

	volkLoadDevice(m_logicalDevice);

	vkGetDeviceQueue(m_logicalDevice, indices.GetFamilyIndex(QueueFamilyIndices::QueueTypes::Graphics), 0, &m_graphicsQueue);
	vkGetDeviceQueue(m_logicalDevice, indices.GetFamilyIndex(QueueFamilyIndices::QueueTypes::Present), 0, &m_presentQueue);
}

void RendererVulkan::CreateSwapChain()
{
	SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(m_physicalDevice, m_surface);

	VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = ChooseSwapExtent(swapChainSupport.capabilities, m_scaledWidth, m_scaledHeight);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{ VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
	createInfo.surface = m_surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices;
	FindQueueFamilies(m_physicalDevice, m_surface, indices);
	uint32_t queueFamilyIndices[] = { indices.GetFamilyIndex(QueueFamilyIndices::QueueTypes::Graphics), indices.GetFamilyIndex(QueueFamilyIndices::QueueTypes::Present) };

	if (queueFamilyIndices[0] != queueFamilyIndices[1]) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	VK_CHECK(vkCreateSwapchainKHR(m_logicalDevice, &createInfo, nullptr, &m_swapChain));

	vkGetSwapchainImagesKHR(m_logicalDevice, m_swapChain, &imageCount, nullptr);
	m_swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_logicalDevice, m_swapChain, &imageCount, m_swapChainImages.data());
	m_swapChainImageFormat = surfaceFormat.format;
}

void RendererVulkan::CreateImageViews()
{
	m_swapChainImageViews.resize(m_swapChainImages.size());

	for (size_t i = 0; i < m_swapChainImages.size(); i++) 
	{
		VkImageViewCreateInfo createInfo { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		createInfo.image = m_swapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = m_swapChainImageFormat;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		VK_CHECK(vkCreateImageView(m_logicalDevice, &createInfo, nullptr, &m_swapChainImageViews[i]));
	}
}

void RendererVulkan::CreateRenderPass()
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = m_swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	VK_CHECK(vkCreateRenderPass(m_logicalDevice, &renderPassInfo, nullptr, &m_renderPass));
}

void RendererVulkan::CreateGraphicsPipeline()
{
	ShaderCompiler::CombinedShaderBinary shaders = ShaderCompiler::GetCompiledShaders(m_mainShaderName);

	VkShaderModule vertShaderModule = CreateShaderModule(m_logicalDevice, shaders.m_vs);
	VkShaderModule fragShaderModule = CreateShaderModule(m_logicalDevice, shaders.m_fs);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	VkPipelineVertexInputStateCreateInfo vertexInputInfo { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };

	VkVertexInputBindingDescription bindingDescription = GetVertexBindingDescription();
	auto attributeDescriptions = GetVertexAttributeDescriptions();

	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)m_scaledWidth;
	viewport.height = (float)m_scaledHeight;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = VkExtent2D{ m_scaledWidth, m_scaledHeight };

	VkPipelineViewportStateCreateInfo viewportState { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;

	VK_CHECK(vkCreatePipelineLayout(m_logicalDevice, &pipelineLayoutInfo, nullptr, &m_pipelineLayout));

	VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

	VkPipelineDynamicStateCreateInfo dynamicStateInfo = {};
	dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateInfo.dynamicStateCount = 2;
	dynamicStateInfo.pDynamicStates = dynamicStates;

	VkGraphicsPipelineCreateInfo pipelineInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicStateInfo;
	pipelineInfo.layout = m_pipelineLayout;
	pipelineInfo.renderPass = m_renderPass;
	pipelineInfo.subpass = 0;

	VK_CHECK(vkCreateGraphicsPipelines(m_logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_graphicsPipeline));

	vkDestroyShaderModule(m_logicalDevice, fragShaderModule, nullptr);
	vkDestroyShaderModule(m_logicalDevice, vertShaderModule, nullptr);
}


void RendererVulkan::CreateFramebuffers()
{
	m_swapChainFramebuffers.resize(m_swapChainImageViews.size());

	for (size_t i = 0; i < m_swapChainImageViews.size(); i++) 
	{
		VkImageView attachments[] = 
		{
			m_swapChainImageViews[i]
		};

		VkFramebufferCreateInfo framebufferInfo { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
		framebufferInfo.renderPass = m_renderPass;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = m_scaledWidth;
		framebufferInfo.height = m_scaledHeight;
		framebufferInfo.layers = 1;

		VK_CHECK(vkCreateFramebuffer(m_logicalDevice, &framebufferInfo, nullptr, &m_swapChainFramebuffers[i]));
	}
}

void RendererVulkan::CreateCommandPool()
{
	QueueFamilyIndices queueFamilyIndices;
	FindQueueFamilies(m_physicalDevice, m_surface, queueFamilyIndices);

	VkCommandPoolCreateInfo poolInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
	poolInfo.queueFamilyIndex = queueFamilyIndices.GetFamilyIndex(QueueFamilyIndices::QueueTypes::Graphics);
	m_graphicsQueueFamilyIndex = poolInfo.queueFamilyIndex;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VK_CHECK(vkCreateCommandPool(m_logicalDevice, &poolInfo, nullptr, &m_commandPool));
}

void RendererVulkan::CreateCommandBuffers()
{
	m_commandBuffers.resize(m_swapChainFramebuffers.size());

	VkCommandBufferAllocateInfo allocInfo { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	allocInfo.commandPool = m_commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)m_commandBuffers.size();

	VK_CHECK(vkAllocateCommandBuffers(m_logicalDevice, &allocInfo, m_commandBuffers.data()));
}

void RendererVulkan::CreateSemaphores()
{
	VkSemaphoreCreateInfo semaphoreInfo { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };

	VK_CHECK(vkCreateSemaphore(m_logicalDevice, &semaphoreInfo, nullptr, &m_imageAvailableSemaphore));
	VK_CHECK(vkCreateSemaphore(m_logicalDevice, &semaphoreInfo, nullptr, &m_renderFinishedSemaphore));
}

void RendererVulkan::CreateDescriptorPool()
{
	VkDescriptorPoolSize poolSizes[2]{ {} ,{} };
	//VkDescriptorPoolSize poolSize{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(m_swapChainImages.size());
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = 1;

	VkDescriptorPoolCreateInfo poolInfo { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
	poolInfo.poolSizeCount = 2;
	poolInfo.pPoolSizes = poolSizes;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolInfo.maxSets = static_cast<uint32_t>(m_swapChainImages.size()) + 1;

	VK_CHECK(vkCreateDescriptorPool(m_logicalDevice, &poolInfo, nullptr, &m_descriptorPool));
}

void RendererVulkan::CreateDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(m_swapChainImages.size(), m_descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	allocInfo.descriptorPool = m_descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(m_swapChainImages.size());
	allocInfo.pSetLayouts = layouts.data();

	m_descriptorSets.resize(m_swapChainImages.size());
	VK_CHECK(vkAllocateDescriptorSets(m_logicalDevice, &allocInfo, m_descriptorSets.data()));

	for (size_t i = 0; i < m_swapChainImages.size(); i++) 
	{

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = m_textureImageView;
		imageInfo.sampler = m_textureSampler;

		VkWriteDescriptorSet descriptorWrite { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		descriptorWrite.dstSet = m_descriptorSets[i];
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(m_logicalDevice, 1, &descriptorWrite, 0, nullptr);
	}
}

void RendererVulkan::CreateVertexBuffer()
{
	uint32_t size = sizeof(Vertex) * 4;
	CreateBuffer(m_logicalDevice, m_physicalDevice, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_vertexBuffer, m_vertexBufferMemory);

	void* data;
	vkMapMemory(m_logicalDevice, m_vertexBufferMemory, 0, size, 0, &data);
	memcpy(data, m_fullscreenQuadVertices, (size_t)size);
	vkUnmapMemory(m_logicalDevice, m_vertexBufferMemory);
}

void RendererVulkan::CreateTextureImage()
{
	VkDeviceSize imageSize = m_sourceWidth * m_sourceHeight * 4;
	CreateBuffer(m_logicalDevice, m_physicalDevice, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_imageUploadBuffer, m_imageUploadBufferMemory);

	void* data;
	vkMapMemory(m_logicalDevice, m_imageUploadBufferMemory, 0, imageSize, 0, &data);
	memset(data, 1, static_cast<size_t>(imageSize));
	vkUnmapMemory(m_logicalDevice, m_imageUploadBufferMemory);

	VkImageCreateInfo imageInfo { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = static_cast<uint32_t>(m_sourceWidth);
	imageInfo.extent.height = static_cast<uint32_t>(m_sourceHeight);
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

	VK_CHECK(vkCreateImage(m_logicalDevice, &imageInfo, nullptr, &m_textureImage));

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(m_logicalDevice, m_textureImage, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(m_physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	vkAllocateMemory(m_logicalDevice, &allocInfo, nullptr, &m_textureImageMemory);
	vkBindImageMemory(m_logicalDevice, m_textureImage, m_textureImageMemory, 0);

	VkCommandBuffer cb1 = BeginRecording();
	TransitionImageLayout(cb1, m_textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	EndRecording(cb1);
	VkCommandBuffer cb2 = BeginRecording();
	CopyBufferToImage(cb2, m_imageUploadBuffer, m_textureImage, m_sourceWidth, m_sourceHeight);
	EndRecording(cb2);
	VkCommandBuffer cb3 = BeginRecording();
	TransitionImageLayout(cb3, m_textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	EndRecording(cb3);
}

void RendererVulkan::CreateTextureSampler()
{
	VkSamplerCreateInfo samplerInfo { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
	samplerInfo.magFilter = VK_FILTER_NEAREST;
	samplerInfo.minFilter = VK_FILTER_NEAREST;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	VK_CHECK(vkCreateSampler(m_logicalDevice, &samplerInfo, nullptr, &m_textureSampler));
}

void RendererVulkan::CleanupSwapChain()
{
	for (auto framebuffer : m_swapChainFramebuffers)
	{
		vkDestroyFramebuffer(m_logicalDevice, framebuffer, nullptr);
	}

	for (auto imageView : m_swapChainImageViews)
	{
		vkDestroyImageView(m_logicalDevice, imageView, nullptr);
	}

	vkDestroySwapchainKHR(m_logicalDevice, m_swapChain, nullptr);
}

void RendererVulkan::RecreateSwapChain()
{
	uint32_t width = 0, height = 0;
	m_backend.GetWindowSize(width, height);
	while (width == 0 || height == 0)
	{
		m_backend.GetWindowSize(width, height);
		KeyBindRequest dummy;
		m_backend.ProcessEvents(dummy);
	}

	vkDeviceWaitIdle(m_logicalDevice);

	m_shouldResize = false;

	CleanupSwapChain();

	CreateSwapChain();
	CreateImageViews();
	CreateFramebuffers();
}

void RendererVulkan::CreateDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding samplerLayoutBinding{};
	samplerLayoutBinding.binding = 0;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkDescriptorSetLayoutCreateInfo layoutInfo { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	layoutInfo.bindingCount = 1;
	layoutInfo.pBindings = &samplerLayoutBinding;

	VK_CHECK(vkCreateDescriptorSetLayout(m_logicalDevice, &layoutInfo, nullptr, &m_descriptorSetLayout));
}

void RendererVulkan::CreateTextureImageView()
{
	VkImageViewCreateInfo viewInfo { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	viewInfo.image = m_textureImage;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VK_CHECK(vkCreateImageView(m_logicalDevice, &viewInfo, nullptr, &m_textureImageView));
}

VkCommandBuffer RendererVulkan::BeginRecording()
{
	VkCommandBufferAllocateInfo allocInfo { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = m_commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(m_logicalDevice, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void RendererVulkan::EndRecording(VkCommandBuffer buffer)
{
	vkEndCommandBuffer(buffer);

	VkSubmitInfo submitInfo { VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &buffer;

	vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_graphicsQueue);

	vkFreeCommandBuffers(m_logicalDevice, m_commandPool, 1, &buffer);
}

void RendererVulkan::CreateIndexBuffer()
{
	uint32_t size = sizeof(uint16_t) * 6;
	CreateBuffer(m_logicalDevice, m_physicalDevice, size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_indexBuffer, m_indexBufferMemory);

	void* data;
	vkMapMemory(m_logicalDevice, m_indexBufferMemory, 0, size, 0, &data);
	memcpy(data, m_fullscreenQuadIndices, (size_t) size);
	vkUnmapMemory(m_logicalDevice, m_indexBufferMemory);
}

void RendererVulkan::Init()
{
	VK_CHECK(volkInitialize());

	m_backend.InitWindow(m_scaledWidth, m_scaledHeight, this);
    CreateInstance();
#if VALIDATION_LAYERS
	InitValidationCallback(m_instance, m_debugMessenger);
#endif

	m_backend.CreateSurface(m_instance, m_surface);
	SelectPhysicalDevice();
	CreateLogicalDevice();
	CreateSwapChain();
	CreateImageViews();
	CreateRenderPass();
	CreateDescriptorSetLayout();
	CreateGraphicsPipeline();
	CreateFramebuffers();
	CreateCommandPool();
	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateTextureImage();
	CreateTextureImageView();
	CreateTextureSampler();
	CreateDescriptorPool();
	CreateDescriptorSets();
	CreateCommandBuffers();
	CreateSemaphores();
}


void RendererVulkan::RegisterOptionsCallbacks(UserSettings& userSettings)
{
	userSettings.m_graphicsScalingFactor.RegisterCallback(std::bind(&RendererVulkan::SetScale, this, std::placeholders::_2));
}

void RendererVulkan::SetScale(uint32_t scale)
{
    m_scaledWidth = m_sourceWidth * scale;
	m_scaledHeight = m_sourceHeight * scale;
	m_backend.ResizeWindow(m_scaledWidth, m_scaledHeight);
	m_shouldResize = true;
}

void RendererVulkan::SetWindowTitle(const char* title)
{
	m_backend.SetWindowTitle(title);
}

bool RendererVulkan::PauseRendering()
{
	if (m_shouldRender)
	{
		m_shouldRender = false;
		m_stateMachine.SetState(StateMachine::EngineState::PAUSED);
		return true;
	}
	return false;
}

bool RendererVulkan::ResumeRendering()
{
	if (!m_shouldRender)
	{
		m_shouldRender = true;
		m_stateMachine.SetState(StateMachine::EngineState::RUNNING);
		return true;
	}
	return false;
}

bool RendererVulkan::ProcessEvents(KeyBindRequest& keyBindRequest)
{
	return m_backend.ProcessEvents(keyBindRequest);
}

const std::unordered_map<uint32_t, bool>& RendererVulkan::GetInputEventMap()
{
	return m_backend.GetInputEventMap();
}

bool RendererVulkan::BeginDraw(const void* renderedImage)
{
	if(!m_shouldRender)
	{
		return false;
	}

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(m_logicalDevice, m_swapChain, UINT64_MAX, m_imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
	
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_shouldResize)
	{
		RecreateSwapChain();
		return false;
	}
	else if (result != VK_SUCCESS) 
	{
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	m_commandBufferIndex = imageIndex;
	
	if (renderedImage != nullptr)
	{
		uint32_t imageSize = m_sourceWidth * m_sourceHeight * 4;
		void* data;
		vkMapMemory(m_logicalDevice, m_imageUploadBufferMemory, 0, imageSize, 0, &data);
		memcpy(data, renderedImage, imageSize);
		vkUnmapMemory(m_logicalDevice, m_imageUploadBufferMemory);
	}
	
	//VK_CHECK(vkResetCommandPool(m_logicalDevice, m_commandPool, 0));

	VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	VkCommandBuffer& activeBuffer = m_commandBuffers[m_commandBufferIndex];
	VK_CHECK(vkBeginCommandBuffer(activeBuffer, &beginInfo));


	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)m_scaledWidth;
	viewport.height = (float)m_scaledHeight;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = VkExtent2D{ m_scaledWidth, m_scaledHeight };

	vkCmdSetViewport(activeBuffer, 0, 1, &viewport);
	vkCmdSetScissor(activeBuffer, 0, 1, &scissor);

	
	TransitionImageLayout(activeBuffer, m_textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	CopyBufferToImage(activeBuffer, m_imageUploadBuffer, m_textureImage, m_sourceWidth, m_sourceHeight);
	TransitionImageLayout(activeBuffer, m_textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	VkRenderPassBeginInfo renderPassInfo{ VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
	renderPassInfo.renderPass = m_renderPass;
	renderPassInfo.framebuffer = m_swapChainFramebuffers[m_commandBufferIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = VkExtent2D{ m_scaledWidth, m_scaledHeight };
	VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(activeBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	
	vkCmdBindPipeline(activeBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);
	VkBuffer vertexBuffers[] = { m_vertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(activeBuffer, 0, 1, vertexBuffers, offsets);

	vkCmdBindIndexBuffer(activeBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT16);

	vkCmdBindDescriptorSets(activeBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSets[m_commandBufferIndex], 0, nullptr);

	vkCmdDrawIndexed(activeBuffer, static_cast<uint32_t>(6), 1, 0, 0, 0);
	
	return true;
}

void RendererVulkan::EndDraw()
{
	vkCmdEndRenderPass(m_commandBuffers[m_commandBufferIndex]);

	VK_CHECK(vkEndCommandBuffer(m_commandBuffers[m_commandBufferIndex]));

	VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };

	VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphore };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_commandBuffers[m_commandBufferIndex];

	VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphore };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	VK_CHECK(vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE));

	VkPresentInfoKHR presentInfo{ VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { m_swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &m_commandBufferIndex;

	vkQueuePresentKHR(m_presentQueue, &presentInfo);

	vkQueueWaitIdle(m_presentQueue);
	
}

void RendererVulkan::WaitForIdle()
{
	vkDeviceWaitIdle(m_logicalDevice);
}

void* RendererVulkan::GetWindowHandle()
{
	return m_backend.GetWindowHandle();
}

void* RendererVulkan::GetDisplay()
{
#if YAGE_PLATFORM_UNIX
	return m_backend.GetDisplay();
#else
	// Not needed on Windows
	return nullptr;
#endif
}


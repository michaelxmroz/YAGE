#pragma once
#include "vulkan/vulkan.h"
#include <vector>

#define WIN32_BACKEND 1

#if _DEBUG
#define VALIDATION_LAYERS 1
#else
#define VALIDATION_LAYERS 0
#endif


#if WIN32_BACKEND
#include "BackendWin32.h"

typedef BackendWin32 Backend;
#endif
#include "MiniMath.h"

class RendererVulkan
{
public:
	RendererVulkan(uint32_t sourceWidth, uint32_t sourceHeight, uint32_t scale);
	~RendererVulkan();

	RendererVulkan(const RendererVulkan& other) = delete;
	RendererVulkan operator= (const RendererVulkan& other) = delete;

	void Draw(const void* renderedImage);
	void WaitForIdle();
	bool RequestExit();

	void ShowFPS_Cheap(double deltaTime);

private:
	void Init();
	void CreateInstance();
	void SelectPhysicalDevice();
	void CreateLogicalDevice();
	void CreateSwapChain();
	void CreateImageViews();
	void CreateRenderPass();
	void CreateDescriptorSetLayout();
	void CreateDescriptorPool();
	void CreateDescriptorSets();
	void CreateGraphicsPipeline();
	void CreateFramebuffers();
	void CreateCommandPool();
	void CreateVertexBuffer();
	void CreateIndexBuffer();
	void CreateTextureImage();
	void CreateTextureImageView();
	void CreateTextureSampler();

	void CreateCommandBuffers();
	void CreateSemaphores();

	VkCommandBuffer BeginRecording();
	void EndRecording(VkCommandBuffer buffer);

	Backend m_backend;
	VkInstance m_instance;
	VkPhysicalDevice m_physicalDevice;
	VkDevice m_logicalDevice;
	VkQueue m_graphicsQueue;
	VkQueue m_presentQueue;
	VkSurfaceKHR m_surface;
	VkSwapchainKHR m_swapChain;
	std::vector<VkImage> m_swapChainImages;
	std::vector<VkImageView> m_swapChainImageViews;
	VkFormat m_swapChainImageFormat;
	const char* m_mainShaderName;
	VkRenderPass m_renderPass;
	VkPipelineLayout m_pipelineLayout;
	VkPipeline m_graphicsPipeline;
	std::vector<VkFramebuffer> m_swapChainFramebuffers;
	VkCommandPool m_commandPool;

	VkBuffer m_vertexBuffer;
	VkDeviceMemory m_vertexBufferMemory;
	VkBuffer m_indexBuffer;
	VkDeviceMemory m_indexBufferMemory;

	VkBuffer m_imageUploadBuffer;
	VkDeviceMemory m_imageUploadBufferMemory;

	VkImage m_textureImage;
	VkDeviceMemory m_textureImageMemory;
	VkImageView m_textureImageView;
	VkSampler m_textureSampler;

	VkDescriptorSetLayout m_descriptorSetLayout;
	VkDescriptorPool m_descriptorPool;
	std::vector<VkDescriptorSet> m_descriptorSets;

	std::vector<VkCommandBuffer> m_commandBuffers;
	VkSemaphore m_imageAvailableSemaphore;
	VkSemaphore m_renderFinishedSemaphore;

	const Vertex m_fullscreenQuadVertices[4];
	const uint16_t m_fullscreenQuadIndices[6];

#if VALIDATION_LAYERS
	VkDebugUtilsMessengerEXT m_debugMessenger;
#endif

	uint32_t m_sourceWidth;
	uint32_t m_sourceHeight;
	uint32_t m_scaledWidth;
	uint32_t m_scaledHeight;
};

typedef RendererVulkan Renderer;

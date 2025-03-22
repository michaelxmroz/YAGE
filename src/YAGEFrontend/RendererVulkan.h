#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <vector>

#define VALIDATION_LAYERS 0

#include "Backend.h"
#include "MiniMath.h"
#include "EngineState.h"

class UI;

class RendererVulkan
{
public:
	RendererVulkan(StateMachine& stateMachine, uint32_t sourceWidth, uint32_t sourceHeight, uint32_t scale);
	~RendererVulkan();

	RendererVulkan(const RendererVulkan& other) = delete;
	RendererVulkan operator= (const RendererVulkan& other) = delete;

	void RegisterOptionsCallbacks(UserSettings& userSettings);

	void SetScale(uint32_t scale);
	void SetWindowTitle(const char* title);
	bool PauseRendering();
	bool ResumeRendering();

	bool ProcessEvents(KeyBindRequest& keyBindingRequest);
	const std::unordered_map<uint32_t, bool>& GetInputEventMap();

	bool BeginDraw(const void* renderedImage);
	void EndDraw();
	void WaitForIdle();

	void* GetWindowHandle();
	void* GetDisplay();

private:

	friend class UI;

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

	void CleanupSwapChain();
	void RecreateSwapChain();

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
	uint32_t m_commandBufferIndex;
	VkSemaphore m_imageAvailableSemaphore;
	VkSemaphore m_renderFinishedSemaphore;

	uint32_t m_graphicsQueueFamilyIndex;

	const Vertex m_fullscreenQuadVertices[4];
	const uint16_t m_fullscreenQuadIndices[6];

#if VALIDATION_LAYERS
	VkDebugUtilsMessengerEXT m_debugMessenger;
#endif

	uint32_t m_sourceWidth;
	uint32_t m_sourceHeight;
	uint32_t m_scaledWidth;
	uint32_t m_scaledHeight;
	bool m_shouldResize;
	bool m_shouldRender;
	StateMachine& m_stateMachine;
};

typedef RendererVulkan Renderer;

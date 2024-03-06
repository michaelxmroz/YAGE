#include "UI.h"
#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_vulkan.h"
#include "Logger.h"
#include "Logging.h"

static void check_vk_result(VkResult err)
{
    if (err == 0)
        return;
    LOG_ERROR(string_format("[vulkan] Error: VkResult = %d\n", err).c_str());
    if (err < 0)
        abort();
}

#ifdef _DEBUG
#define APP_USE_VULKAN_DEBUG_REPORT
#endif

#ifdef APP_USE_VULKAN_DEBUG_REPORT
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
{
    (void)flags; (void)object; (void)location; (void)messageCode; (void)pUserData; (void)pLayerPrefix; // Unused arguments
    fprintf(stderr, "[vulkan] Debug report from ObjectType: %i\nMessage: %s\n\n", objectType, pMessage);
    return VK_FALSE;
}
#endif // APP_USE_VULKAN_DEBUG_REPORT

namespace UI_Internal
{
    void DrawMainMenuBar()
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Reset")) {}
                if (ImGui::MenuItem("Load")) 
                {
                    Backend::OpenFileDialog();
                }
                if (ImGui::MenuItem("Recent")) {}
                if (ImGui::MenuItem("Quick Save", "CTRL+1")) {}
                if (ImGui::MenuItem("Quick Load", "CTRL+2")) {}
                if (ImGui::MenuItem("Save state as")) {}
                if (ImGui::MenuItem("Load state")) {}

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit"))
            {
                if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
                if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
                ImGui::Separator();
                if (ImGui::MenuItem("Cut", "CTRL+X")) {}
                if (ImGui::MenuItem("Copy", "CTRL+C")) {}
                if (ImGui::MenuItem("Paste", "CTRL+V")) {}
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }
}

UI::UI(RendererVulkan& renderer)
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    //io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

    HWND* windowHandle = static_cast<HWND*>(renderer.GetWindowHandle());
    ImGui_ImplWin32_Init(*windowHandle);

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = renderer.m_instance;
    init_info.PhysicalDevice = renderer.m_physicalDevice;
    init_info.Device = renderer.m_logicalDevice;
    init_info.QueueFamily = renderer.m_graphicsQueueFamilyIndex;
    init_info.Queue = renderer.m_graphicsQueue;
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = renderer.m_descriptorPool;
    init_info.RenderPass = renderer.m_renderPass;
    init_info.Subpass = 0;
    init_info.MinImageCount = 2;
    init_info.ImageCount = 2;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = nullptr;
    init_info.CheckVkResultFn = check_vk_result;

    if (!ImGui_ImplVulkan_Init(&init_info))
    {
        LOG_ERROR("Initializing imgui Vulkan back end failed");
    }
}

void UI::Prepare()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    bool show = true;
    ImGui::ShowDemoWindow(&show);

    UI_Internal::DrawMainMenuBar();
}

void UI::Draw(RendererVulkan& renderer)
{
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), renderer.m_commandBuffers[renderer.m_commandBufferIndex]);
}

UI::~UI()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

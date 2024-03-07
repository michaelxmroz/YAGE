#include "UI.h"
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_vulkan.h"
#include "Logger.h"
#include "Logging.h"
#include "EngineState.h"
#include <algorithm>

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
    void DrawMainMenuBar(UIState& state, EngineData& data)
    {
        ImVec2 viewportPos = ImGui::GetMainViewport()->Pos;
        ImVec2 viewportSize = ImGui::GetMainViewport()->Size;
        if (!data.m_gameLoaded || state.m_submenuShown || ImGui::IsMouseHoveringRect(viewportPos, ImVec2(viewportPos.x + viewportSize.x, viewportPos.y + ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2), false))
        {
            state.m_showMenuBar = true;
        }
        else
        {
            state.m_showMenuBar = false;
        }

        if (state.m_showMenuBar && state.m_menuBarAlpha < 2.0f)
        {
            state.m_menuBarAlpha += 0.2f;
        }
        else if (!state.m_showMenuBar && state.m_menuBarAlpha > 0.0f)
        {
            state.m_menuBarAlpha -= 0.01f;
        }

        state.m_submenuShown = false;

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, std::min(1.0f,state.m_menuBarAlpha));

        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                state.m_submenuShown = true;
                if (ImGui::MenuItem("Reset")) 
                {
                	data.m_state = EngineData::State::RESET;
                }
                if (ImGui::MenuItem("Load")) 
                {
                    std::string path = Backend::OpenFileDialog();
                    if (!path.empty())
                    {
                        data.m_gamePath = path;
                        data.m_state = EngineData::State::RESET;
                    }
                }
                if (ImGui::MenuItem("Recent")) {}
                if (ImGui::MenuItem("Quick Save", "CTRL+1")) 
                {
                    data.m_saveLoadState = EngineData::SaveLoadState::SAVE;
                    data.m_saveLoadPath = "";
                }
                if (ImGui::MenuItem("Quick Load", "CTRL+2")) 
                {
                    data.m_saveLoadState = EngineData::SaveLoadState::LOAD;
                    data.m_saveLoadPath = "";
                }
                if (ImGui::MenuItem("Save state as")) {}
                if (ImGui::MenuItem("Load state")) {}

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit"))
            {
                state.m_submenuShown = true;
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

        ImGui::PopStyleVar();
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

void UI::Prepare(EngineData& data)
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    bool show = true;
    ImGui::ShowDemoWindow(&show);

    UI_Internal::DrawMainMenuBar(m_state, data);
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

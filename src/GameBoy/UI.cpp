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

static int ImGui_CreateVkSurface(ImGuiViewport* viewport, ImU64 vk_instance, const void* vk_allocator, ImU64* out_vk_surface)
{
    VkSurfaceKHR surface;
    HWND windowHandle = static_cast<HWND>(viewport->PlatformHandleRaw);
    Backend::CreateSurface((VkInstance)vk_instance, surface, windowHandle);
    *out_vk_surface = (ImU64)surface;
    return 0;
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
    void ShowAudioOptions(UIState& state, EngineData& data)
	{
        if (state.m_activeWindow == UIState::ActiveWindow::AUDIO)
        {
			ImGui::OpenPopup("Audio Options");
			state.m_activeWindow = UIState::ActiveWindow::NONE; 
            
            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        }
        // Always center this window when appearing

        if (ImGui::BeginPopupModal("Audio Options", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            state.m_submenuShown = true;

            int volume = static_cast<int>(data.m_userSettings.m_audioVolume.GetValue() * 100.0f);
            ImGui::Text("Master Volume");
            ImGui::SliderInt("##", &volume, 0, 100, "%d", ImGuiSliderFlags_None);

            data.m_userSettings.m_audioVolume.SetValue(static_cast<float>(volume) / 100.0f);

            if (ImGui::Button("Save", ImVec2(120, 0))) 
            { 
                data.m_userSettings.Save();
                ImGui::CloseCurrentPopup(); 
            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            { 
                data.m_userSettings.DiscardChanges();
                ImGui::CloseCurrentPopup(); 
            }
            ImGui::EndPopup();
        }
	}

    void ShowGraphicsOptions(UIState& state, EngineData& data)
    {
        if (state.m_activeWindow == UIState::ActiveWindow::GRAPHICS)
        {
            ImGui::OpenPopup("Graphics Options");
            state.m_activeWindow = UIState::ActiveWindow::NONE;

            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        }

        if (ImGui::BeginPopupModal("Graphics Options", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            state.m_submenuShown = true;

            int scale = static_cast<int>(data.m_userSettings.m_graphicsScalingFactor.GetValue());
            ImGui::Text("Resolution Scale");
            ImGui::SliderInt("##", &scale, 1, 25, "%d", ImGuiSliderFlags_None);
            data.m_userSettings.m_graphicsScalingFactor.SetValue(static_cast<uint32_t>(scale));

            ImGui::SameLine();

            ImGui::LabelText("##", "%sx%s",FileParser::ToString(data.m_baseWidth * scale).c_str(), FileParser::ToString(data.m_baseHeight * scale).c_str());

            if (ImGui::Button("Save", ImVec2(120, 0)))
            {
                data.m_userSettings.Save();
                ImGui::CloseCurrentPopup();
            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                data.m_userSettings.DiscardChanges();
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

    void ShowSystemOptions(UIState& state, EngineData& data)
    {
        if (state.m_activeWindow == UIState::ActiveWindow::SYSTEM)
        {
            ImGui::OpenPopup("System Options");
            state.m_activeWindow = UIState::ActiveWindow::NONE;

            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        }
        // Always center this window when appearing

        if (ImGui::BeginPopupModal("System Options", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            state.m_submenuShown = true;

            bool check = data.m_userSettings.m_systemUseBootrom.GetValue();
            ImGui::Checkbox("Use Bootrom", &check);
            data.m_userSettings.m_systemUseBootrom.SetValue(check);

            ImGui::Text("Bootrom Path");

            std::string path = data.m_userSettings.m_systemBootromPath.GetValue();

            ImGui::PushID("##TextBox");
            //We're not going to write into the buffer as the box will always be read-only
            ImGui::InputText("", const_cast<char*>(path.c_str()), ImGuiInputTextFlags_ReadOnly);
            ImGui::PopID();

            ImGui::SameLine();

            if (ImGui::Button("Select"))
            {
                path = Backend::OpenFileLoadDialog(L"Rom Files (*.gb; *.rom; *.bin)", L"*.gb;*.rom;*.bin");
                if (!path.empty())
                {
                    data.m_userSettings.m_systemBootromPath.SetValue(path);
                }
            }

            if (ImGui::Button("Save", ImVec2(120, 0)))
            {
                data.m_userSettings.Save();
                ImGui::CloseCurrentPopup();
            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                data.m_userSettings.DiscardChanges();
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
    }

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
                if (ImGui::MenuItem("Reset", 0, false, data.m_gameLoaded))
                {
                	data.m_engineState.SetState(StateMachine::EngineState::RESET);
                }
                if (ImGui::MenuItem("Load")) 
                {
                    std::string path = Backend::OpenFileLoadDialog(L"Rom Files (*.gb; *.rom)", L"*.gb;*.rom");
                    if (!path.empty())
                    {
                        data.m_gamePath = path;
                        data.m_userSettings.AddRecentFile(path);
                        data.m_engineState.SetState(StateMachine::EngineState::RESET);
                        data.m_userSettings.Save();
                    }
                }
                if (ImGui::BeginMenu("Load Recent"))
                {
                    bool hasAtLeastOne = false;
                    for(uint32_t i = 0; i < data.m_userSettings.m_recentFiles.size(); i++)
					{
                        const std::string path = data.m_userSettings.m_recentFiles[i].GetValue();

                        if (data.m_userSettings.m_recentFiles[i].GetValue().empty())
                        {
                            continue;
                        }

						if (ImGui::MenuItem(path.c_str()))
						{
							data.m_gamePath = path;
							data.m_engineState.SetState(StateMachine::EngineState::RESET);
						}

                        hasAtLeastOne = true;
					}

                    if(!hasAtLeastOne)
					{
						ImGui::MenuItem("No recent files");
					}

                    ImGui::EndMenu();
                }
                if (ImGui::MenuItem("Quick Save", "CTRL+1", false, data.m_gameLoaded))
                {
                    data.m_saveLoadState = EngineData::SaveLoadState::SAVE;
                    data.m_saveLoadPath = "";
                }
                if (ImGui::MenuItem("Quick Load", "CTRL+2", false, data.m_gameLoaded))
                {
                    data.m_saveLoadState = EngineData::SaveLoadState::LOAD;
                    data.m_saveLoadPath = "";
                }
                if (ImGui::MenuItem("Save state as", 0, false, data.m_gameLoaded))
                {
                    std::string path = Backend::OpenFileSaveDialog(L"Save state files (*.ssf)", L"*.ssf", L"ssf");
                    if (!path.empty())
                    {
                        data.m_saveLoadState = EngineData::SaveLoadState::SAVE;
                        data.m_saveLoadPath = path;
                    }
                }
                if (ImGui::MenuItem("Load state", 0, false, data.m_gameLoaded))
                {
                    std::string path = Backend::OpenFileLoadDialog(L"Save state files (*.ssf)", L"*.ssf");
                    if (!path.empty())
                    {
                        data.m_saveLoadState = EngineData::SaveLoadState::LOAD;
                        data.m_saveLoadPath = path;
                    }
                }

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Options"))
            {
                state.m_submenuShown = true;
                if (ImGui::MenuItem("System")) 
                {
                    state.m_activeWindow = UIState::ActiveWindow::SYSTEM;
                }
                if (ImGui::MenuItem("Graphics")) 
                {
                    state.m_activeWindow = UIState::ActiveWindow::GRAPHICS;
                }
                if (ImGui::MenuItem("Audio")) 
                {
                    state.m_activeWindow = UIState::ActiveWindow::AUDIO;
                }
                if (ImGui::MenuItem("Controls")) {}
                if (ImGui::MenuItem("Debug")) {}

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
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

    
    ImGui::GetPlatformIO().Platform_CreateVkSurface = ImGui_CreateVkSurface;
    
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
    //ImGui::ShowDemoWindow(&show);

    UI_Internal::DrawMainMenuBar(m_state, data);

    UI_Internal::ShowSystemOptions(m_state, data);
    UI_Internal::ShowGraphicsOptions(m_state, data);
    UI_Internal::ShowAudioOptions(m_state, data);

    if (m_state.m_submenuShown && data.m_engineState.GetState() == StateMachine::EngineState::RUNNING)
    {
        data.m_engineState.SetState(StateMachine::EngineState::PAUSED);
    }
    else if (!m_state.m_submenuShown && data.m_engineState.GetState() == StateMachine::EngineState::PAUSED)
    {
        data.m_engineState.SetState(StateMachine::EngineState::RUNNING);
    }
}

void UI::Draw(RendererVulkan& renderer)
{
    ImGui::Render();

    ImGui::UpdatePlatformWindows();
    ImGui::RenderPlatformWindowsDefault();

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), renderer.m_commandBuffers[renderer.m_commandBufferIndex]);
}

UI::~UI()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

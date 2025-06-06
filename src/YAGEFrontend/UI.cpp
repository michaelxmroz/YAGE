#include "UI.h"
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_vulkan.h"
#include "Logger.h"
#include "Logging.h"
#include "EngineState.h"
#include "Input.h"
#include "UIStrings.h"
#include "volk.h"

const char* FONT_PATH = "../../../externalLibs/imgui/misc/fonts/ProggyClean.ttf";

namespace
{
    template<typename ... Args>
    void ShowUIMessage(UIState& state, const char* message, Args ... args)
    {
        state.formatedMessage = string_format(message, args ...);

        state.m_messageAlphaTween.Reset();
    }
}

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

const ImVec4 logMessageColors[3] = 
{
	ImVec4(1.0f, 1.0f, 1.0f, 1.0f), //INFO
	ImVec4(1.0f, 1.0f, 0.0f, 1.0f), //WARNING
	ImVec4(1.0f, 0.0f, 0.0f, 1.0f)  //ERROR
};

namespace
{
    ImVec4 GetColorForLogLevel(Logger::LogLevel level)
    {
        switch (level)
        {
		case Logger::LogLevel::Info:
			return logMessageColors[0];
		case Logger::LogLevel::Warning:
			return logMessageColors[1];
		case Logger::LogLevel::Error:
			return logMessageColors[2];
		default:
			return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		}
	}

    void DrawUIMessageBox(UIState& state, double deltaMs)
    {
        if (state.formatedMessage.empty() || state.m_messageAlphaTween.HasFinished())
        {
            return;
        }

        const ImVec2 viewportSize = ImGui::GetMainViewport()->Size;
        const ImVec2 viewportPos = ImGui::GetMainViewport()->Pos;

        ImGui::PushFont(state.m_fontLarge);

        const char* message = state.formatedMessage.c_str();
        ImVec2 textSize = ImGui::CalcTextSize(message);

        const ImVec2 windowPadding = ImGui::GetStyle().WindowPadding;
        if (textSize.x + windowPadding.x * 2.0f > viewportSize.x)

        {
            ImGui::PopFont();
            ImGui::PushFont(state.m_fontSmall);
            textSize = ImGui::CalcTextSize(message);
        }

        const float boxHeight = ImGui::GetFontSize() + windowPadding.y * 2;
        const float boxWidth = viewportSize.x;
        const ImVec2 boxPos = ImVec2(viewportPos.x, viewportSize.y + viewportPos.y - boxHeight);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, state.m_messageAlphaTween.Update(static_cast<float>(deltaMs))));
        ImGui::SetNextWindowPos(boxPos);
        ImGui::SetNextWindowSize(ImVec2(boxWidth, boxHeight));
        ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);

        if (ImGui::Begin("BottomLeftBox", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings)) 
        {
            ImGui::TextUnformatted(message);
        }
        ImGui::End();

        ImGui::PopStyleColor();

        ImGui::PopStyleVar(2);

        ImGui::PopFont();
    }

    void DrawLogWindow(UIState& state)
    {
        if (state.m_showLogWindow)
        {
			ImGui::SetNextWindowSize(ImVec2(800, 400), ImGuiCond_FirstUseEver);
            if (!ImGui::Begin("Log", &state.m_showLogWindow))
            {
				ImGui::End();
				return;
			}

			ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

            uint32_t messageCount = Logger::GlobalLogBuffer::GetMessageCount();
            std::string message;
            Logger::LogLevel level;
            for (uint32_t i = 0; i < messageCount; i++)
            {
                uint32_t reverseIndex = messageCount - i - 1;
                Logger::GlobalLogBuffer::GetLogMessage(message, level, reverseIndex);
                ImVec4 color = GetColorForLogLevel(level);

                ImGui::PushStyleColor(ImGuiCol_Text, color);
                ImGui::TextUnformatted(message.c_str());
                ImGui::PopStyleColor();
            }

            uint32_t newMessageIndex = Logger::GlobalLogBuffer::GetCurrentMessageIndex();

            if (newMessageIndex != state.m_lastMessageIndex)
            {
                state.m_lastMessageIndex = newMessageIndex;
				ImGui::SetScrollHereY(1.0f);
			}

			ImGui::PopStyleVar();
			ImGui::EndChild();
			ImGui::End();
		}
	}

    void DrawStatsWindow(UIState& state, EngineData& data)
    {
        if (state.m_showStatsWindow)
        {
            ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_FirstUseEver);
            if (!ImGui::Begin("Stats", &state.m_showStatsWindow))
            {
                ImGui::End();
                return;
            }

            ImGui::Text("Allocated Memory: ");
            ImGui::SameLine();
            ImGui::Text(std::to_string(data.m_stats.m_allocatedMemory).c_str());

            ImGui::End();
        }
    }

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
            state.m_submenuState.Update(true);

            int volume = static_cast<int>(data.m_userSettings.m_audioVolume.GetValue() * 100.0f);
            ImGui::Text("Master Volume");
            ImGui::SliderInt("##", &volume, 0, 100, "%d", ImGuiSliderFlags_None);

            data.m_userSettings.m_audioVolume.SetValue(static_cast<float>(volume) / 100.0f);

            if (ImGui::Button("Save", ImVec2(120, 0))) 
            { 
                data.m_userSettings.Save();
                ImGui::CloseCurrentPopup();
                ShowUIMessage(state, UIStrings::OPTION_SAVED);
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
            state.m_submenuState.Update(true);

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
                ShowUIMessage(state, UIStrings::OPTION_SAVED);
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
            state.m_submenuState.Update(true);

            ImGui::Checkbox("Use Turbo", &data.m_turbo);

            ImGui::BeginDisabled(!data.m_turbo);
            float turbo  = static_cast<float>(data.m_userSettings.m_systemTurboSpeed.GetValue());
            float turboValues[10] = { 0.25f, 0.5f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f };

            int index = 0;
            if (turbo < 0.5f)
            {
				index = 0;
			}
            else if (turbo < 1.0f)
            {
				index = 1;
			}
            else
            {
				index = static_cast<int>(turbo) + 1;
			}

            ImGui::SliderInt("##", &index, 0, 9, std::to_string(turboValues[index]).c_str());
            data.m_userSettings.m_systemTurboSpeed.SetValue(turboValues[index]);
            ImGui::EndDisabled();

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
                ShowUIMessage(state, UIStrings::OPTION_SAVED);
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

    void ShowInputOptions(UIState& state, EngineData& data)
    {
        if (state.m_activeWindow == UIState::ActiveWindow::INPUTS)
        {
            ImGui::OpenPopup("Input Options");
            state.m_activeWindow = UIState::ActiveWindow::NONE;

            ImVec2 center = ImGui::GetMainViewport()->GetCenter();
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        }

        bool openKeyBindingPopup = false;

        if (ImGui::BeginPopupModal("Input Options", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            state.m_submenuState.Update(true);

            ImGui::Text("Keybindings");
            std::string assignedKey(" ");
            
            if (ImGui::BeginTable("KeybindingsTable", 2))
            {
                for (uint32_t i = 0; i < data.m_userSettings.m_keyBindings.size(); i++)
                {
                    ImGui::TableNextRow();
                    const char* uiName = InputActionNames[i];
                    auto& keyBinding = data.m_userSettings.m_keyBindings[i];

                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text(uiName);

                    uint32_t boundValue = keyBinding.GetValue();
                    if (boundValue != 0)
                    {
                        assignedKey = Backend::ConvertVirtualKeyToString(boundValue);
                    }
                    else
                    {
                        assignedKey[0] = ' ';
                    }

                    ImGui::TableSetColumnIndex(1);
                    ImGui::PushID("##KeybindButton");
                    if (ImGui::Button(assignedKey.c_str(), ImVec2(40, 0)))
                    {
                        state.m_keybindingTitle = "Binding " + std::string(uiName);

						openKeyBindingPopup = true;
						state.m_keybindingIndex = i;
                    }
                    ImGui::PopID();
                }

                ImGui::EndTable();
            }    
            
            if (openKeyBindingPopup)
            {
                ImGui::OpenPopup(state.m_keybindingTitle.c_str());
                data.m_keyBindRequest.m_status = KeyBindRequest::Status::REQUESTED;
            }

            //bool unused_open = true;
            if (ImGui::BeginPopupModal(state.m_keybindingTitle.c_str()))
            {
                std::string bindingText = "Press any button to bind as:";
                ImGui::Text(bindingText.c_str());
                
                float windowWidth = ImGui::GetWindowSize().x;
                float textWidth = ImGui::CalcTextSize(InputActionNames[state.m_keybindingIndex]).x;
                float centerX = (windowWidth - textWidth) * 0.5f;
                ImGui::SetCursorPosX(centerX);
                ImGui::Text(InputActionNames[state.m_keybindingIndex]);

                ImGui::SetCursorPosX((windowWidth - 120) * 0.5f);
                if (ImGui::Button("Cancel", ImVec2(120, 0)))
                {
                    data.m_keyBindRequest.m_status = KeyBindRequest::Status::NONE;
                    ImGui::CloseCurrentPopup();
                }

                if(data.m_keyBindRequest.m_status == KeyBindRequest::Status::CONFIRMED)
				{
                    data.m_userSettings.m_keyBindings[state.m_keybindingIndex].SetValue(data.m_keyBindRequest.m_keyCode);
					data.m_keyBindRequest.m_status = KeyBindRequest::Status::NONE;
                    ImGui::CloseCurrentPopup();
				}

                ImGui::EndPopup();
            }

            if (ImGui::Button("Save", ImVec2(120, 0)))
            {
                data.m_userSettings.Save();
                ImGui::CloseCurrentPopup();
                ShowUIMessage(state, UIStrings::OPTION_SAVED);
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

    void DrawMainMenuBar(UIState& state, EngineData& data, DebuggerUI& debugger, double deltaMs)
    {
        ImVec2 viewportPos = ImGui::GetMainViewport()->Pos;
        ImVec2 viewportSize = ImGui::GetMainViewport()->Size;

        float menuAlpha = 1.0f;
        if (!data.m_gameLoaded || state.m_submenuState.IsOpen() || ImGui::IsMouseHoveringRect(viewportPos, ImVec2(viewportPos.x + viewportSize.x, viewportPos.y + ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2), false))
        {
            state.m_menuBarAlphaTween.Reset();
        }
        else
        {
            menuAlpha = state.m_menuBarAlphaTween.Update(static_cast<float>(deltaMs));
        }

        state.m_submenuState.Update(false);

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, std::min(1.0f, menuAlpha));

        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                state.m_submenuState.Update(true);
                if (ImGui::MenuItem("Reset", 0, false, data.m_gameLoaded))
                {
                	data.m_engineState.SetState(StateMachine::EngineState::RESET);
                    ShowUIMessage(state, UIStrings::EMULATOR_RESET);
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
                        ShowUIMessage(state, UIStrings::SUCCESSFUL_LOAD);
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
                            ShowUIMessage(state, UIStrings::SUCCESSFUL_LOAD);
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
                    ShowUIMessage(state, UIStrings::STATE_SAVED);
                }
                if (ImGui::MenuItem("Quick Load", "CTRL+2", false, data.m_gameLoaded))
                {
                    data.m_saveLoadState = EngineData::SaveLoadState::LOAD;
                    data.m_saveLoadPath = "";
                    ShowUIMessage(state, UIStrings::STATE_LOADED);
                }
                if (ImGui::MenuItem("Save state as", 0, false, data.m_gameLoaded))
                {
                    std::string path = Backend::OpenFileSaveDialog(L"Save state files (*.ssf)", L"*.ssf", L"ssf");
                    if (!path.empty())
                    {
                        data.m_saveLoadState = EngineData::SaveLoadState::SAVE;
                        data.m_saveLoadPath = path;
                        ShowUIMessage(state, UIStrings::STATE_SAVED);
                    }
                }
                if (ImGui::MenuItem("Load state", 0, false, data.m_gameLoaded))
                {
                    std::string path = Backend::OpenFileLoadDialog(L"Save state files (*.ssf)", L"*.ssf");
                    if (!path.empty())
                    {
                        data.m_saveLoadState = EngineData::SaveLoadState::LOAD;
                        data.m_saveLoadPath = path;
                        ShowUIMessage(state, UIStrings::STATE_LOADED);
                    }
                }

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Options"))
            {
                state.m_submenuState.Update(true);
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
                if (ImGui::MenuItem("Inputs")) 
                {
					state.m_activeWindow = UIState::ActiveWindow::INPUTS;
                }
                if (ImGui::BeginMenu("Debug"))
                {
                    if (ImGui::MenuItem("Debugger"))
                    {
                        debugger.Toggle(data.m_debuggerState);
                    }
                    if (ImGui::MenuItem("Log Window"))
                    {
                        state.m_showLogWindow = !state.m_showLogWindow;
                    }
                    if (ImGui::MenuItem("Stats"))
                    {
                        state.m_showStatsWindow = !state.m_showStatsWindow;
                    }

                    ImGui::EndMenu();
                }

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

    m_state.m_fontSmall = io.Fonts->AddFontFromFileTTF(FONT_PATH, 13.0f);
    m_state.m_fontLarge = io.Fonts->AddFontFromFileTTF(FONT_PATH, 26.0f);

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

    ImGui_ImplVulkan_LoadFunctions(VK_API_VERSION_1_3, [](const char* function_name, void* instance) { return vkGetInstanceProcAddr(static_cast<VkInstance>(instance), function_name); }, renderer.m_instance);

    if (!ImGui_ImplVulkan_Init(&init_info))
    {
        LOG_ERROR("Initializing imgui Vulkan back end failed");
    }

    m_state.m_menuBarAlphaTween = Tween(1.0f, 0.0f, 2000.0f, 1000.0f);
    m_state.m_messageAlphaTween = Tween(0.75f, 0.0f, 2000.0f, 1500.0f);

#if _DEBUG
    m_state.m_showLogWindow = true;
#endif
}

void UI::Prepare(EngineData& data, double deltaMs)
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    bool show = true;
    //ImGui::ShowDemoWindow(&show);

    DrawMainMenuBar(m_state, data, m_debugger, deltaMs);

    DrawLogWindow(m_state);
    DrawStatsWindow(m_state, data);
    m_debugger.Draw(data.m_debuggerState);

    ShowSystemOptions(m_state, data);
    ShowGraphicsOptions(m_state, data);
    ShowAudioOptions(m_state, data);
    ShowInputOptions(m_state, data);

    DrawUIMessageBox(m_state, deltaMs);

    if (m_state.m_submenuState.HasOpened() && data.m_engineState.GetState() == StateMachine::EngineState::RUNNING)
    {
        data.m_engineState.SetState(StateMachine::EngineState::PAUSED);
    }
    else if (m_state.m_submenuState.HasClosed() && data.m_engineState.GetState() == StateMachine::EngineState::PAUSED)
    {
        data.m_engineState.SetState(StateMachine::EngineState::RUNNING);
    }
    m_state.m_submenuState.EndFrame();
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

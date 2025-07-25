#include "EngineController.h"
#include "DebuggerUtils.h"

std::string EngineController::s_persistentMemoryPath;

void* AllocFunc(uint32_t size)
{
    LOG_INFO(string_format("Emulator is allocating %i bytes of memory", size).c_str());
    return new uint8_t[size];
}

void FreeFunc(void* ptr)
{
    LOG_INFO("Emulator is freeing memory");
    delete[] static_cast<uint8_t*>(ptr);
}

#if _DEBUG
void DumpMemory(void* userData)
{
    EngineController* engine = static_cast<EngineController*>(userData);
    engine->Save(true);
    DebuggerUtils::TriggerBreakpoint(userData);
}
#endif

void GatherStats(Emulator& emulator, EngineData& state)
{
    state.m_stats.m_allocatedMemory = emulator.GetMemoryUse();
#if defined (_DEBUG)
    state.m_debuggerState.m_cpuStatePrevious = state.m_debuggerState.m_cpuState;
    state.m_debuggerState.m_cpuState = emulator.GetCPUState();
    state.m_debuggerState.m_tCyclesStepped = state.m_debuggerState.m_cpuState.m_tCyclesStepped;
    state.m_debuggerState.m_rawMemoryView = emulator.GetRawMemoryView();
    state.m_debuggerState.m_ppuState = emulator.GetPPUState();
#endif
}

EngineController::EngineController(EngineData& state) :
    m_data(state)
{
    m_data.m_baseWidth = EmulatorConstants::SCREEN_WIDTH;
    m_data.m_baseHeight = EmulatorConstants::SCREEN_HEIGHT;

    m_renderer = new RendererVulkan(m_data.m_engineState, EmulatorConstants::SCREEN_WIDTH, EmulatorConstants::SCREEN_HEIGHT, m_data.m_userSettings.m_graphicsScalingFactor.GetValue());
    m_renderer->RegisterOptionsCallbacks(m_data.m_userSettings);
    
    m_audio = new Audio();
    m_audio->Init();
    m_audio->RegisterOptionsCallbacks(m_data.m_userSettings);
    m_audio->RegisterEngineStateChangeCallbacks(m_data.m_engineState);

    m_UI = new UI(*m_renderer);

    m_inputHandler = new InputHandler();
    m_inputHandler->RegisterOptionsCallbacks(m_data.m_userSettings);

    m_emulator = nullptr;
}

void EngineController::Run()
{
    while (m_data.m_engineState.GetState() != StateMachine::EngineState::EXIT)
    {
        Logger::FileOutput::Clear();

        if (m_data.m_engineState.GetState() == StateMachine::EngineState::RESET)
        {
            m_data.m_engineState.SetState(StateMachine::EngineState::RUNNING);
        }

        std::vector<char> romBlob;
        if (!m_data.m_gamePath.empty())
        {
            if (!FileParser::Read(m_data.m_gamePath, romBlob))
            {
                LOG_ERROR("Could not read file at provided path");
            }
        }

        std::vector<char> bootromBlob;
        if (m_data.m_userSettings.m_systemUseBootrom.GetValue())
        {
            m_data.m_bootromPath = m_data.m_userSettings.m_systemBootromPath.GetValue();
            if (!m_data.m_bootromPath.empty())
            {
                if (!FileParser::Read(m_data.m_bootromPath, bootromBlob))
                {
                    LOG_ERROR("Could not read bootrom file");
                }
            }
        }

        std::string fileWithoutEnding = FileParser::StripFileEnding(m_data.m_gamePath.c_str());
        s_persistentMemoryPath = string_format("%s.%s", fileWithoutEnding.c_str(), PERSISTENT_MEMORY_FILE_ENDING);

        std::vector<char> ramBlob;
        FileParser::Read(s_persistentMemoryPath, ramBlob);

        if (!romBlob.empty())
        {
            std::string windowTitle = string_format("YAGE - %s", fileWithoutEnding.c_str());
#if _DEBUG
            windowTitle += " - DEBUG";
#endif
            m_renderer->SetWindowTitle(windowTitle.c_str());

            CreateEmulator(bootromBlob, romBlob, ramBlob);
        }

        RunEmulatorLoop();

        CleanupEmulator();
    }

    m_renderer->WaitForIdle();
    m_audio->Terminate();
}

EngineController::~EngineController()
{
    delete m_inputHandler;
    delete m_UI;
    delete m_audio;
    delete m_renderer;
    CleanupEmulator();
}

void EngineController::SavePersistentMemory(const void* data, uint32_t size)
{
    if (!FileParser::Write(s_persistentMemoryPath, data, static_cast<size_t>(size)))
    {
        LOG_ERROR("Could not write persistent save file");
    }
}

void EngineController::CreateEmulator(const std::vector<char>& bootromBlob, const std::vector<char>& romBlob, const std::vector<char>& ramBlob)
{
    m_emulator = Emulator::Create(&AllocFunc, &FreeFunc);

    m_data.m_gameLoaded = true;

    m_emulator->SetLoggerCallback(&LogMessage);

    std::string filename = FileParser::StripPath(m_data.m_gamePath.c_str());

    if (!bootromBlob.empty())
    {
        m_emulator->Load(filename.c_str(), romBlob.data(), static_cast<uint32_t>(romBlob.size()), bootromBlob.data(), static_cast<uint32_t>(bootromBlob.size()));
    }
    else
    {
        m_emulator->Load(filename.c_str(), romBlob.data(), static_cast<uint32_t>(romBlob.size()));
    }

    if (!ramBlob.empty())
    {
        m_emulator->LoadPersistentMemory(ramBlob.data(), static_cast<uint32_t>(ramBlob.size()));
    }
    m_emulator->SetPersistentMemoryCallback(SavePersistentMemory);

    m_emulator->SetAudioBuffer(m_audio->GetAudioBuffer(), m_audio->GetAudioBufferSize(), m_audio->GetSampleRate(), m_audio->GetWritePosition());
}

void EngineController::CleanupEmulator()
{
    Emulator::Delete(m_emulator);
    m_emulator = nullptr;
    m_data.m_debuggerState.ResetEmulatorData();
    
}

void EngineController::RunEmulatorLoop()
{
    uint32_t frameCount = 0;
    const void* frameBuffer = nullptr;

    Clock clock;
    clock.Start();
    int64_t previousFrameUs = clock.Query();

    while (m_data.m_engineState.GetState() == StateMachine::EngineState::RUNNING 
        || m_data.m_engineState.GetState() == StateMachine::EngineState::PAUSED)
    {
        int64_t currentFrameUs = clock.Query();
        int64_t deltaUs = currentFrameUs - previousFrameUs;
        previousFrameUs = currentFrameUs;
        double deltaMs = deltaUs / 1000.0;

        if (frameCount == 0)
        {
            //On the first frame we just fake the delta time
            deltaMs = m_preferredFrameTime;
        }

        //There is no way we'll catch up to that, just skip some frames
        if (deltaMs > m_preferredFrameTime * 2)
        {
            deltaMs = m_preferredFrameTime;
        }

        if (!m_renderer->ProcessEvents(m_data.m_keyBindRequest))
        {
			m_data.m_engineState.SetState(StateMachine::EngineState::EXIT);
			break;
        }

        EmulatorInputs::InputState inputState;
        m_inputHandler->Update(m_data, m_renderer->GetInputEventMap(), inputState);

        if (m_data.m_gameLoaded)
        {
            bool shouldStep = m_data.m_engineState.GetState() != StateMachine::EngineState::PAUSED;
            double emulatorDeltaMs = deltaMs;
            bool microstep = m_data.m_debuggerState.m_microstepping;
            if(m_data.m_debuggerState.m_debuggerActive && m_data.m_debuggerState.m_debuggerSteps >= 0)
            {
                emulatorDeltaMs = microstep ? EMULATOR_CLOCK_MS : EMULATOR_CLOCK_MS * 4;

                if (!microstep && m_data.m_debuggerState.m_tCyclesStepped != 0)
                {
                    emulatorDeltaMs = EMULATOR_CLOCK_MS * (4 - m_data.m_debuggerState.m_tCyclesStepped);
                    microstep = true;
                }

                if(m_data.m_debuggerState.m_debuggerSteps == 0)
                {
                    shouldStep = false;
                }
                else if(m_data.m_debuggerState.m_debuggerSteps > 0)
                {
                    m_data.m_debuggerState.m_debuggerSteps--;
                }
            }

			if(shouldStep)
			{
                m_emulator->SetTurboSpeed(m_data.m_turbo ? m_data.m_userSettings.m_systemTurboSpeed.GetValue() : 1.0f);

                if (m_data.m_debuggerState.m_triggerDebugBreak)
                {
                    m_data.m_debuggerState.m_triggerDebugBreak = false;
                    DebuggerUtils::TriggerBreakpoint(nullptr);
                }

                m_emulator->Step(inputState, emulatorDeltaMs, microstep);
                frameBuffer = m_emulator->GetFrameBuffer();
                m_audio->Play();
			}

            if(shouldStep || m_data.m_debuggerState.m_forceGatherStats)
            {
                GatherStats(*m_emulator, m_data);
                m_data.m_debuggerState.m_forceGatherStats = false;
            }

            HandleSaveLoad();

            frameCount++;
        }

        if (m_renderer->BeginDraw(frameBuffer))
        {
            m_UI->Prepare(m_data, deltaMs, m_emulator);
            m_UI->Draw(*m_renderer);
            m_renderer->EndDraw();
        }

        clock.Limit(static_cast<int64_t>(m_preferredFrameTime * 1000));
    }
    m_audio->Pause();
}

void EngineController::HandleSaveLoad()
{
    if (m_data.m_saveLoadState == EngineData::SaveLoadState::SAVE && m_data.m_gameLoaded)
    {
        Save(false);
    }
    else if (m_data.m_saveLoadState == EngineData::SaveLoadState::LOAD)
    {
        Load();
    }
    m_data.m_saveLoadState = EngineData::SaveLoadState::NONE;
}

void EngineController::Load()
{
    std::string saveStatePath = m_data.m_saveLoadPath;
    if (saveStatePath.empty())
    {
        std::string fileWithoutEnding = FileParser::StripFileEnding(m_data.m_gamePath.c_str());
        saveStatePath = string_format("%s.%s", fileWithoutEnding.c_str(), SAVE_STATE_FILE_ENDING);
    }
    std::vector<char> saveState;
    if (FileParser::Read(saveStatePath, saveState))
    {
        SerializationView loadData{ reinterpret_cast<uint8_t*>(saveState.data()), static_cast<uint32_t>(saveState.size()) };
        m_emulator->Deserialize(loadData);
    }
}

void EngineController::Save(bool rawData) const
{
    SerializationView savedState = m_emulator->Serialize(rawData);
    std::string saveStatePath = m_data.m_saveLoadPath;
    if (saveStatePath.empty())
    {
        std::string fileWithoutEnding = FileParser::StripFileEnding(m_data.m_gamePath.c_str());
        saveStatePath = string_format("%s.%s", fileWithoutEnding.c_str(), SAVE_STATE_FILE_ENDING);
    }

    FileParser::Write(saveStatePath, savedState.data, savedState.size);
}

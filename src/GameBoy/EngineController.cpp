#include "EngineController.h"
#include "Debugging.h"

std::string EngineController::s_persistentMemoryPath;

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

        if (romBlob.size() > 0)
        {
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

inline void EngineController::CreateEmulator(const std::vector<char>& bootromBlob, const std::vector<char>& romBlob, const std::vector<char>& ramBlob)
{
    m_emulator = Emulator::Create();

    m_data.m_gameLoaded = true;

    m_emulator->SetLoggerCallback(&LogMessage);

    std::string filename = FileParser::StripPath(m_data.m_gamePath.c_str());

    if (bootromBlob.size() > 0)
    {
        m_emulator->Load(filename.c_str(), romBlob.data(), static_cast<uint32_t>(romBlob.size()), bootromBlob.data(), static_cast<uint32_t>(bootromBlob.size()));
    }
    else
    {
        m_emulator->Load(filename.c_str(), romBlob.data(), static_cast<uint32_t>(romBlob.size()));
    }

    if (ramBlob.size() > 0)
    {
        m_emulator->LoadPersistentMemory(ramBlob.data(), static_cast<uint32_t>(ramBlob.size()));
    }
    m_emulator->SetPersistentMemoryCallback(SavePersistentMemory);

    m_emulator->SetAudioBuffer(m_audio->GetAudioBuffer(), m_audio->GetAudioBufferSize(), m_audio->GetSampleRate(), m_audio->GetWritePosition());
}

inline void EngineController::CleanupEmulator()
{
    Emulator::Delete(m_emulator);
    m_emulator = nullptr;
}

inline void EngineController::RunEmulatorLoop()
{
    if (m_emulator != nullptr)
    {
        //Debug stops
        //m_emulator->SetPCCallback(0x0604, &Debugging::TriggerBreakpoint);
        //m_emulator->SetInstructionCountCallback(100000, &Debugging::TriggerBreakpoint);
        //m_emulator->SetInstructionCallback(0x40, &Debugging::TriggerBreakpoint);
    }

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
            if (m_data.m_engineState.GetState() != StateMachine::EngineState::PAUSED)
            {
                m_emulator->SetTurboSpeed(m_data.m_turbo ? m_data.m_userSettings.m_systemTurboSpeed.GetValue() : 1.0f);
                m_emulator->Step(inputState, deltaMs);
                frameBuffer = m_emulator->GetFrameBuffer();
                m_audio->Play();
            }

            HandleSaveLoad();

            frameCount++;
        }

        if (m_renderer->BeginDraw(frameBuffer))
        {
            m_UI->Prepare(m_data);
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
        std::vector<uint8_t> saveState = m_emulator->Serialize();
        std::string saveStatePath = m_data.m_saveLoadPath;
        if(saveStatePath.empty())
		{
			std::string fileWithoutEnding = FileParser::StripFileEnding(m_data.m_gamePath.c_str());
			saveStatePath = string_format("%s.%s", fileWithoutEnding.c_str(), SAVE_STATE_FILE_ENDING);
		}

        FileParser::Write(saveStatePath, saveState.data(), saveState.size());
    }
    else if (m_data.m_saveLoadState == EngineData::SaveLoadState::LOAD)
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
            m_emulator->Deserialize(reinterpret_cast<uint8_t*>(saveState.data()), static_cast<uint32_t>(saveState.size()));
        }
    }
    m_data.m_saveLoadState = EngineData::SaveLoadState::NONE;
}

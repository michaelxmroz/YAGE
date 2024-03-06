#include "EngineController.h"

std::string EngineController::s_persistentMemoryPath;

EngineController::EngineController(EngineData state) :
    m_data(state)
{
    m_renderer = new RendererVulkan(EmulatorConstants::SCREEN_WIDTH, EmulatorConstants::SCREEN_HEIGHT, 3);
    m_audio = new Audio();
    m_audio->Init();
    m_UI = new UI(*m_renderer);
    m_inputHandler = new InputHandler();
    m_emulator = nullptr;
}

void EngineController::Run()
{
    while (m_data.m_state != EngineData::State::EXIT)
    {
        if (m_data.m_state == EngineData::State::RESET)
        {
            m_data.m_state = EngineData::State::RUNNING;
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
        if (!m_data.m_bootromPath.empty())
        {
            if (!FileParser::Read(m_data.m_bootromPath, bootromBlob))
            {
                LOG_ERROR("Could not read bootrom file");
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
    delete m_renderer;
    delete m_audio;
    delete m_UI;
    delete m_inputHandler;
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
}

inline void EngineController::RunEmulatorLoop()
{
    //Debug stops
    //emu->SetPCCallback(0xC1B9, &Debugging::TriggerBreakpoint);
    //emu->SetInstructionCountCallback(166644, &Debugging::TriggerBreakpoint);

    uint32_t frameCount = 0;
    const void* frameBuffer = nullptr;

    Clock clock;
    clock.Start();
    int64_t previousFrameUs = clock.Query();

    while (m_data.m_state == EngineData::State::RUNNING)
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

        m_UI->Prepare();

        EmulatorInputs::InputState inputState;
        m_inputHandler->Update(inputState);

        if (m_data.m_gameLoaded)
        {
            if (!m_inputHandler->IsPaused())
            {
                m_emulator->Step(inputState, deltaMs);
                frameBuffer = m_emulator->GetFrameBuffer();
            }

            //TODO add to UI
            if (m_inputHandler->m_debugSaveState)
            {
                std::vector<uint8_t> saveState = m_emulator->Serialize();
                std::string fileWithoutEnding = FileParser::StripFileEnding(m_data.m_gamePath.c_str());
                std::string saveStatePath = string_format("%s%u.%s", fileWithoutEnding.c_str(), 1, SAVE_STATE_FILE_ENDING);
                FileParser::Write(saveStatePath, saveState.data(), saveState.size());
            }
            else if (m_inputHandler->m_debugLoadState)
            {
                std::string fileWithoutEnding = FileParser::StripFileEnding(m_data.m_gamePath.c_str());
                std::string saveStatePath = string_format("%s%u.%s", fileWithoutEnding.c_str(), 1, SAVE_STATE_FILE_ENDING);
                std::vector<char> saveState;
                if (FileParser::Read(saveStatePath, saveState))
                {
                    m_emulator->Deserialize(reinterpret_cast<uint8_t*>(saveState.data()), static_cast<uint32_t>(saveState.size()));
                }
            }

            if (frameCount == 0)
            {
                //Start playing audio after buffer has been filled for the first frame
                m_audio->Play();
            }

            frameCount++;
        }
        m_renderer->BeginDraw(frameBuffer);
        m_UI->Draw(*m_renderer);
        m_renderer->EndDraw();

        if (m_renderer->RequestExit())
        {
            m_data.m_state = EngineData::State::EXIT;
        }

        std::cout << "True Frame time: " << deltaMs << std::endl;
        clock.Limit(static_cast<int64_t>(m_preferredFrameTime * 1000));
    }

}

#include <iostream>
#include "CommandLineArguments.h"
#include "Emulator.h"
#include "FileParser.h"
#include "ScreenshotUtility.h"
#include "Logging.h"
#include "Input.h"
#include "RendererVulkan.h"
#include "Audio.h"
#include "Clock.h"
#include "Debugging.h"

#define PERSISTENT_MEMORY_FILE_ENDING "sav"
#define SAVE_STATE_FILE_ENDING "ssf"

#define DEFAULT_FRAME_DURATION 16667

static std::string s_persistentMemoryPath;

void SavePersistentMemory(const void* data, uint32_t size)
{
    if (!FileParser::Write(s_persistentMemoryPath, data, static_cast<size_t>(size)))
    {
        LOG_ERROR("Could not write persistent save file");
    }
}

int main(int argc, char* argv[])
{
    CommandLineParser commandLine(argc, argv);

    std::string filePath = commandLine.GetArgument("-file");
    if (filePath.empty())
    {
        LOG_ERROR("Please provide a valid file path with the -file argument");
        return -1;
    }

    std::vector<char> romBlob;
    if (!FileParser::Read(filePath, romBlob))
    {
        LOG_ERROR("Could not read file at provided path");
        return -1;
    }

    std::string bootromPath = commandLine.GetArgument("-bootrom");
    std::vector<char> bootromBlob;
    if (!bootromPath.empty())
    {
        if (!FileParser::Read(bootromPath, bootromBlob))
        {
            LOG_ERROR("Could not read bootrom at provided path");
        }
    }

    std::string fileWithoutEnding = FileParser::StripFileEnding(filePath.c_str());
    std::string filename = FileParser::StripPath(filePath.c_str());

    s_persistentMemoryPath = string_format("%s.%s", fileWithoutEnding.c_str(), PERSISTENT_MEMORY_FILE_ENDING);
    
    std::vector<char> ramBlob;
    FileParser::Read(s_persistentMemoryPath, ramBlob);

    {
        Renderer renderer(EmulatorConstants::SCREEN_WIDTH, EmulatorConstants::SCREEN_HEIGHT, 3);

        const double preferredFrameTime = 1000.0 / EmulatorConstants::PREFERRED_REFRESH_RATE;

        Audio audio;
        audio.Init();

        Emulator* emu = Emulator::Create();

        emu->SetLoggerCallback(&LogMessage);
        if (bootromBlob.size() > 0)
        {
            emu->Load(filename.c_str(), romBlob.data(), static_cast<uint32_t>(romBlob.size()), bootromBlob.data(), static_cast<uint32_t>(bootromBlob.size()));
        }
        else
        {
            emu->Load(filename.c_str(), romBlob.data(), static_cast<uint32_t>(romBlob.size()));
        }

        if (ramBlob.size() > 0)
        {
            emu->LoadPersistentMemory(ramBlob.data(), static_cast<uint32_t>(ramBlob.size()));
        }
        emu->SetPersistentMemoryCallback(SavePersistentMemory);

        emu->SetAudioBuffer(audio.GetAudioBuffer(), audio.GetAudioBufferSize(), audio.GetSampleRate(), audio.GetWritePosition());

        //Debug stops
        //emu->SetPCCallback(0xC1B9, &Debugging::TriggerBreakpoint);
        //emu->SetInstructionCountCallback(165290, &Debugging::TriggerBreakpoint);

        InputHandler inputHandler;
        uint32_t frameCount = 0;
        const void* frameBuffer = nullptr;

        Clock clock;

        clock.Start();

        int64_t previousFrameUs = clock.Query();

        while (!renderer.RequestExit())
        {
            int64_t currentFrameUs = clock.Query();
            int64_t deltaUs = currentFrameUs - previousFrameUs;
            previousFrameUs = currentFrameUs;
            double deltaMs = deltaUs / 1000.0;

            if (frameCount == 0)
            {
                //On the first frame we just fake the delta time
                deltaMs = preferredFrameTime;
            }

            EmulatorInputs::InputState inputState;
            inputHandler.Update(inputState);

            if (!inputHandler.IsPaused())
            {
                emu->Step(inputState, deltaMs);
                frameBuffer = emu->GetFrameBuffer();
            }

            renderer.Draw(frameBuffer);

            if (inputHandler.m_debugSaveState)
            {
                std::vector<uint8_t> saveState = emu->Serialize();
                std::string saveStatePath = string_format("%s%u.%s", fileWithoutEnding.c_str(), 1, SAVE_STATE_FILE_ENDING);
                FileParser::Write(saveStatePath, saveState.data(), saveState.size());
            }
            else if (inputHandler.m_debugLoadState)
            {
                std::string saveStatePath = string_format("%s%u.%s", fileWithoutEnding.c_str(), 1, SAVE_STATE_FILE_ENDING);
                std::vector<char> saveState;
                if (FileParser::Read(saveStatePath, saveState))
                {
                    emu->Deserialize(reinterpret_cast<uint8_t*>(saveState.data()), static_cast<uint32_t>(saveState.size()));
                }
            }

            if (frameCount == 0)
            {
                //Start playing audio after buffer has been filled for the first frame
                audio.Play();
            }

            frameCount++;

            //std::cout << "True Frame time: " << deltaMs << std::endl;
            clock.Limit(preferredFrameTime * 1000);
        }

        renderer.WaitForIdle();
        audio.Terminate();

        //ScreenshotUtility::CreateScreenshot("../screen.png", frameBuffer, EmulatorConstants::SCREEN_WIDTH, EmulatorConstants::SCREEN_HEIGHT);

        Emulator::Delete(emu);
    }
    return 0;
}

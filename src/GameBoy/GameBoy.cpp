// GameBoy.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "CommandLineArguments.h"
#include "Emulator.h"
#include "FileParser.h"
#include "ScreenshotUtility.h"
#include "Logging.h"
#include "Input.h"
#include "RendererVulkan.h"

#define PERSISTENT_MEMORY_FILE_ENDING "sav"

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
    s_persistentMemoryPath = string_format("%s.%s", fileWithoutEnding.c_str(), PERSISTENT_MEMORY_FILE_ENDING);
    
    std::vector<char> ramBlob;
    FileParser::Read(s_persistentMemoryPath, ramBlob);

    {
        Renderer renderer(EmulatorConstants::SCREEN_WIDTH, EmulatorConstants::SCREEN_HEIGHT, 3);

        Emulator* emu = Emulator::Create();

        emu->SetLoggerCallback(&LogMessage);
        if (bootromBlob.size() > 0)
        {
            emu->Load(romBlob.data(), static_cast<uint32_t>(romBlob.size()), bootromBlob.data(), static_cast<uint32_t>(bootromBlob.size()));
        }
        else
        {
            emu->Load(romBlob.data(), static_cast<uint32_t>(romBlob.size()));
        }

        if (ramBlob.size() > 0)
        {
            emu->LoadPersistentMemory(ramBlob.data(), static_cast<uint32_t>(ramBlob.size()));
        }
        emu->SetPersistentMemoryCallback(SavePersistentMemory);

        InputHandler inputHandler;
        uint32_t frameCount = 0;
        const void* frameBuffer = nullptr;
        while (!renderer.RequestExit())
        {
            EmulatorInputs::InputState inputState;
            inputHandler.Update(inputState);

            if (!inputHandler.IsPaused())
            {
                emu->Step(inputState);
                frameBuffer = emu->GetFrameBuffer();
            }

            renderer.Draw(frameBuffer);

            frameCount++;
        }

        renderer.WaitForIdle();

        //ScreenshotUtility::CreateScreenshot("../screen.png", frameBuffer, EmulatorConstants::SCREEN_WIDTH, EmulatorConstants::SCREEN_HEIGHT);

        Emulator::Delete(emu);
    }
    return 0;
}

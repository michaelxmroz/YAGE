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

int main(int argc, char* argv[])
{
    CommandLineParser commandLine(argc, argv);

    std::string filePath = commandLine.GetArgument("-file");
    if (filePath.empty())
    {
        LOG_ERROR("Please provide a valid file path with the -file argument");
        return -1;
    }

    std::shared_ptr<std::vector<char>> romBlob = std::make_shared<std::vector<char>>();
    if (!FileParser::Read(filePath, *romBlob))
    {
        LOG_ERROR("Could not read file at provided path");
        return -1;
    }

    {
        Renderer renderer(EmulatorConstants::SCREEN_WIDTH, EmulatorConstants::SCREEN_HEIGHT, 3);

        Emulator* emu = Emulator::Create();

        emu->SetLoggerCallback(&LogMessage);
        emu->Load(&((*romBlob)[0]), static_cast<uint32_t>(romBlob.get()->size()));

        InputHandler inputHandler;
        uint32_t frameCount = 0;
        const void* frameBuffer = nullptr;
        while (!renderer.RequestExit())
        {
            EmulatorInputs::InputState inputState;
            inputHandler.GetInputState(inputState);
            emu->Step(inputState);
            frameBuffer = emu->GetFrameBuffer();

            renderer.Draw(frameBuffer);

            frameCount++;
        }

        renderer.WaitForIdle();

        //ScreenshotUtility::CreateScreenshot("../screen.png", frameBuffer, EmulatorConstants::SCREEN_WIDTH, EmulatorConstants::SCREEN_HEIGHT);

        Emulator::Delete(emu);
    }
    return 0;
}

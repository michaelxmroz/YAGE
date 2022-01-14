// GameBoy.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "CommandLineArguments.h"
#include "Logging.h"
#include "FileParser.h"
#include "VirtualMachine.h"
#include "ScreenshotUtility.h"

void RenderCallbackScreenshot(const void* image)
{
    ScreenshotUtility::CreateScreenshot("../screen.png", image, SCREEN_WIDTH, SCREEN_HEIGHT);
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

    std::shared_ptr<std::vector<char>> romBlob = std::make_shared<std::vector<char>>();
    if (!FileParser::Parse(filePath, *romBlob))
    {
        LOG_ERROR("Could not read file at provided path");
        return -1;
    }

    VirtualMachine vm;
    if (!vm.Load(romBlob))
    {
        LOG_ERROR("Could not load ROM");
        return -1;
    }

    vm.SetRenderCallback(RenderCallbackScreenshot);

    if (!vm.Start())
    {
        LOG_ERROR("Error while running the virtual machine");
        return -1;
    }

    return 0;
}

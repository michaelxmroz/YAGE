#include "CommandLineArguments.h"
#include "Logging.h"
#include "EngineState.h"
#include "EngineController.h"

int main(int argc, char* argv[])
{
    Logger::FileOutput::Init("log.txt");

    CommandLineParser commandLine(argc, argv);

    std::string filePath = commandLine.GetArgument("-file");
    std::string bootromPath = commandLine.GetArgument("-bootrom");

    EngineData data;
    data.m_gamePath = filePath;
    data.m_bootromPath = bootromPath;

    EngineController controller(data);

    controller.Run();
    
    return 0;
}

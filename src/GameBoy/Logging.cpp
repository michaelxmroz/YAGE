#include "Logging.h"
#include "Windows.h"
#include "debugapi.h"
#include <iostream>

//TODO separate into platform layer

namespace Logger_Helpers
{
	void OutputToVisualStudioConsole(const char* message)
	{
		OutputDebugStringA(message);
		std::cout << message;
	}
}

void LogMessage(const char* message, uint8_t severity)
{
	if (severity == 0)
	{
		LOG_INFO(message);
	}
	else if (severity == 1)
	{
		LOG_WARNING(message);
	}
	else
	{
		LOG_ERROR(message);
	}
}

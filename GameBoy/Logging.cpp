#include "Logging.h"
#include "Windows.h"
#include "debugapi.h"
#include <iostream>

namespace Logger_Helpers
{
	void OutputToVisualStudioConsole(const char* message)
	{
		OutputDebugStringA(message);
		std::cout << message;
	}
}

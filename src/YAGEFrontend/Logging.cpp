#include "Logger.h"
#include <iostream>


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
	else if (severity == 2)
	{
		LOG_ERROR(message);
	}
	else if (severity == 3)
	{
		LOG_MINIMAL(message);
	}
}

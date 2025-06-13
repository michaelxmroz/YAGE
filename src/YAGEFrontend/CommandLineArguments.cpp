#include "CommandLineArguments.h"

CommandLineParser::CommandLineParser(int argc, char** argv)
{
	Parse(argc, argv);
}

std::string CommandLineParser::GetArgument(const std::string& arg)
{
	if (m_argumentDb.count(arg))
	{
		return m_argumentDb[arg];
	}
	return "";
}

bool CommandLineParser::HasArgument(const std::string& arg)
{
	return m_argumentDb.count(arg) > 0;
}

void CommandLineParser::Parse(int argc, char** argv)
{
	for (int i = 1; i < argc; i++)
	{
		std::string arg(argv[i]);
		
		// Skip if not starting with -
		if (arg.empty() || arg[0] != '-')
		{
			continue;
		}

		// Remove the leading -
		arg = arg.substr(1);

		// Check if argument has a value (contains =)
		size_t equalsPos = arg.find('=');
		if (equalsPos != std::string::npos)
		{
			std::string key = arg.substr(0, equalsPos);
			std::string value = arg.substr(equalsPos + 1);
			m_argumentDb[key] = value;
		}
		else
		{
			// Boolean flag, store empty string to indicate presence
			m_argumentDb[arg] = "";
		}
	}
}

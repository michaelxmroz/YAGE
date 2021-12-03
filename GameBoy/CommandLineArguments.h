#pragma once
#include <string>
#include <map>
#include <cassert>

class CommandLineParser
{
public:
	CommandLineParser(int argc, char** argv);

	std::string GetArgument(const std::string& arg);
private:
	void Parse(int argc, char** argv);

	std::map<std::string, std::string> _argumentDb;
};


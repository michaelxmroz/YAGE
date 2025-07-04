#pragma once
#include <string>
#include <map>
#include <cassert>
#include <vector>

class CommandLineParser
{
public:
	CommandLineParser(int argc, char** argv);

	std::string GetArgument(const std::string& arg);
	bool HasArgument(const std::string& arg);

	static CommandLineParser* GlobalCMDParser;
private:
	void Parse(int argc, char** argv);

	std::map<std::string, std::string> m_argumentDb;
};

namespace FileParser
{
	bool Read(std::string path, std::vector<char>& parsedBlob);

	std::string GetFileNameFromPath(std::string path);
	std::vector<std::string> GetFilesInPathRecursive(std::string path);
};


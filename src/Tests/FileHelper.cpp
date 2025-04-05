#include "FileHelper.h"
#include <string>
#include <vector>
#include <fstream>
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

CommandLineParser* CommandLineParser::GlobalCMDParser;

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
	std::cout << "no arg found" << std::endl;
	return "";
}

void CommandLineParser::Parse(int argc, char** argv)
{
	int argCount = argc - 1;

	assert(argCount % 2 == 0);

	int iterationCount = argCount / 2;
	for (size_t i = 0; i < iterationCount; i++)
	{
		size_t index = i * 2 + 1;
		std::string key(argv[index]);
		std::string value(argv[index + 1]);
		m_argumentDb.emplace(key, value);
	}
}


bool FileParser::Read(std::string path, std::vector<char>& parsedBlob)
{
	std::ifstream file;
	file.open(path, std::ios::in | std::ios::binary | std::ios::ate);
	if (file.is_open())
	{
		std::streampos size = file.tellg();
		file.seekg(0, std::ios::beg);

		parsedBlob.clear();
		parsedBlob.resize(size);

		file.read(&(parsedBlob[0]), size);
		file.close();

		return true;
	}
	else
	{
		return false;
	}
}

std::string FileParser::GetFileNameFromPath(std::string path)
{
	fs::path parsedPath(path);
	return parsedPath.stem().string();
}
std::vector<std::string> FileParser::GetFilesInPathRecursive(std::string path)
{
	fs::path dirPath(path);
	if (!fs::exists(dirPath))
	{
		std::cout << "Path does not exist:" << path << std::endl;
		return {};
	}
	std::cout << "Found path:" << path << std::endl;
	std::vector<std::string> files;
	for (const fs::directory_entry& dir_entry :
		std::filesystem::recursive_directory_iterator(dirPath))
	{
		fs::path filePath = dir_entry.path();
		if (filePath.extension().string().compare(".gb") == 0)
		{
			std::string file = dir_entry.path().string();
			files.push_back(file);
		}
	}

	return files;
}

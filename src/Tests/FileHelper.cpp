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
	std::cout << "Argument not found" << std::endl;
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
			std::cout << "Invalid Argument" << std::endl;
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

bool HasExcludedSuffix(const std::string& filename, const std::vector<std::string>& excludedSuffixes) 
{
	for (const auto& suffix : excludedSuffixes) 
	{
		if (filename.size() >= suffix.size() &&
			filename.compare(filename.size() - suffix.size(), suffix.size(), suffix) == 0) 
		{
			return true;
		}
	}
	return false;
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

	std::vector<std::string> excludedSuffixes = { 
		"-S", // Combined SGB tests
		"-dmg0", // DMG0 tests
		"-mgb", // MGB tests
		"-sgb", // SGB1 tests
		"-sgb2", // SGB2 tests
		"-C" // gbc tests
	};

	std::vector<std::string> files;
	for (const fs::directory_entry& dir_entry :
		std::filesystem::recursive_directory_iterator(dirPath))
	{
		fs::path filePath = dir_entry.path();
		if (filePath.extension().string().compare(".gb") == 0)
		{
			std::string filename = dir_entry.path().stem().string();
			// Only add the file if it does not have one of the excluded suffixes.
			if (!HasExcludedSuffix(filename, excludedSuffixes)) 
			{
				std::string file = dir_entry.path().string();
				files.push_back(file);
			}
		}
	}

	return files;
}

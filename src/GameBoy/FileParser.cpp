#include "FileParser.h"
#include <filesystem>

namespace fs = std::filesystem;


std::string FileParser::StripFileEnding(const char* name)
{
	std::string str(name);
	std::size_t found = str.find('.');
	if (found == std::string::npos)
	{
		return "";
	}
	return str.substr(0, found);
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

bool FileParser::Write(std::string path, const void* data, size_t size)
{
	std::ofstream file;
	file.open(path, std::ios::out | std::ios::binary | std::ios::trunc);
	if (file.is_open())
	{
		file.write(reinterpret_cast<const char*>(data), size);
		file.close();
		return true;
	}
	return false;
}

bool FileParser::CreateDirectory(std::string path)
{
	if (!fs::is_directory(path) || !fs::exists(path)) 
	{
		fs::create_directory(path);
		return true;
	}
	return false;
}

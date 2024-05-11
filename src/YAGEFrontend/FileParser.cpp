#include "FileParser.h"
#include <filesystem>
#include <string>
#include <sstream>

namespace fs = std::filesystem;

std::string FileParser::StripPath(const char* path)
{
	fs::path parsedPath(path);
	return parsedPath.filename().string();
}

std::string FileParser::StripFileEnding(const char* name)
{
	fs::path parsedPath(name);
	fs::path strippedPath(parsedPath.parent_path());
	strippedPath += '/';
	strippedPath += parsedPath.stem();
	return strippedPath.string();
}

bool FileParser::Read(std::string path, std::vector<char>& parsedBlob)
{
	std::ifstream file;
	file.open(path, std::ios::in | std::ios::binary | std::ios::ate);
	if (file.is_open())
	{
		std::streampos size = file.tellg();
		
		if (size == 0)
		{
			return false;
		}
		
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

bool FileParser::Read(std::string path, std::string& strOut)
{
	std::ifstream file;
	file.open(path, std::ios::in | std::ios::ate);
	if (file.is_open())
	{
		std::streampos size = file.tellg();
		file.seekg(0, std::ios::beg);

		strOut.resize(size);

		file.read(&strOut[0], size);
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

bool FileParser::Write(std::string path, const std::string& data)
{
	std::ofstream file;
	file.open(path, std::ios::out | std::ios::trunc);
	if (file.is_open())
	{
		file.write(data.c_str(), data.size());
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

bool FileParser::SplitString(const std::string& str, std::vector<std::string>& tokens, const char delimiter)
{
	std::string token;
	std::stringstream tokenStream(str);
	bool hasSplit = false;
	while (std::getline(tokenStream, token, delimiter))
	{
		tokens.push_back(token);
		hasSplit = true;
	}
	return hasSplit;
}

bool FileParser::SplitStringOnce(const std::string& str, std::string& substr1, std::string& substr2, const char delimiter)
{
	std::string token;
	std::stringstream tokenStream(str);
	if (std::getline(tokenStream, token, delimiter))
	{
		substr1 = token;
		if (std::getline(tokenStream, token, '\n'))
		{
			substr2 = token;
			return true;
		}
	}
	return false;
}

template <> uint32_t FileParser::FromString<uint32_t>(const std::string& str)
{
	return std::stoul(str);
}

template <> int32_t FileParser::FromString<int32_t>(const std::string& str)
{
	return std::stoi(str);
}

template <> float FileParser::FromString<float>(const std::string& str)
{
	return std::stof(str);
}

template <> bool FileParser::FromString<bool>(const std::string& str)
{
	return str.size() == 1 && str[0] == '1';
}

template <> std::string FileParser::FromString<std::string>(const std::string& str)
{
	return str;
}

template<> std::string FileParser::ToString(const std::string& value)
{
	return value;
}

template<>
std::string FileParser::ToString(const bool& value)
{
	return value ? "1" : "0";
}

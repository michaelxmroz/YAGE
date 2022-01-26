#pragma once
#include <string>
#include <vector>
#include <fstream>

namespace FileParser
{
	std::string StripFileEnding(const char* name);
	bool Read(std::string path, std::vector<char>& parsedBlob);
	bool Write(std::string path, const void* data, size_t size);
	bool CreateDirectory(std::string path);
};


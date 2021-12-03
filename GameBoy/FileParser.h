#pragma once
#include <string>
#include <vector>
#include <fstream>

namespace FileParser
{
	bool Parse(std::string path, std::vector<char>& parsedBlob);
};


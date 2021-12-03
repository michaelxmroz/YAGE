#include "FileParser.h"

bool FileParser::Parse(std::string path, std::vector<char>& parsedBlob)
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

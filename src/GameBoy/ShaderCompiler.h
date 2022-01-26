#pragma once

#include <vector>

namespace ShaderCompiler
{
	enum class Types
	{
		Vertex = 0,
		Fragment = 1,
		COUNT = 2
	};

	std::vector<uint32_t> Compile(const char* name, const std::vector<char>& shaderBlob, Types type, bool optimize = true);
};


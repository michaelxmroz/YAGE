#pragma once
#include <vector>
#include <memory>

class VirtualMachine
{
public:
	VirtualMachine()
	{
	}

	bool Load(std::shared_ptr<std::vector<char>> romBlob)
	{
		_romBlob = romBlob;

		return true;
	}

	bool Run()
	{
		return true;
	}

private:
	std::shared_ptr<std::vector<char>> _romBlob;
};


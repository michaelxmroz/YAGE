#pragma once
#include "Memory.h"

//Dummy serial connector
class Serial
{
public:
	Serial();
	void Init(Memory& memory);
};


#pragma once
#include <cstdint>

namespace ScreenshotUtility
{
	bool CreateScreenshot(const char* path, const void* data, uint32_t width, uint32_t height);
};


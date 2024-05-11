#include "ScreenshotUtility.h"
#include "miniz.h"
#include <fstream>
#include "FileParser.h"

bool ScreenshotUtility::CreateScreenshot(const char* path, const void* data, uint32_t width, uint32_t height)
{
    size_t png_data_size = 0;
    void* pPNG_data = tdefl_write_image_to_png_file_in_memory_ex(data, width, height, 3, &png_data_size, 6, MZ_FALSE);
    if (!pPNG_data)
        return false;
    else
    {
        FileParser::Write(path, pPNG_data, png_data_size);
    }

    mz_free(pPNG_data);
    return true;
}

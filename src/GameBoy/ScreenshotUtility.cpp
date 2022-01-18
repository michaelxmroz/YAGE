#include "ScreenshotUtility.h"
#include "miniz.h"
#include <fstream>

bool ScreenshotUtility::CreateScreenshot(const char* path, const void* data, uint32_t width, uint32_t height)
{
    size_t png_data_size = 0;
    void* pPNG_data = tdefl_write_image_to_png_file_in_memory_ex(data, width, height, 3, &png_data_size, 6, MZ_FALSE);
    if (!pPNG_data)
        return false;
    else
    {
        std::ofstream file;
        file.open(path, std::ios::out | std::ios::binary | std::ios::trunc);
        if (file.is_open())
        {
            file.write(reinterpret_cast<const char*>(pPNG_data), png_data_size);
            file.close();
        }
    }

    mz_free(pPNG_data);
    return true;
}

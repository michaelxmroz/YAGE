#pragma once
#include "vulkan/vulkan.h"
#include <string>

class BackendWin32
{
public:
	void InitWindow(uint32_t width, uint32_t height);
	void CleanupWindow();

	void ResizeWindow(uint32_t width, uint32_t height);

	static void CreateSurface(VkInstance instance, VkSurfaceKHR& surface, HWND& hwnd);
	void CreateSurface(VkInstance instance, VkSurfaceKHR& surface);
	bool ProcessEvents();

	void SetWindowTitle(const char* title);
	HWND* GetWindowHandle();

	static std::string OpenFileLoadDialog(const wchar_t* fileTypeDescription, const wchar_t* fileTypeEndings);
	static std::string OpenFileSaveDialog(const wchar_t* fileTypeDescription, const wchar_t* fileTypeEndings, const wchar_t* fileExtension);

	static std::string GetPersistentDataPath();
private:
	class Window
	{
	public:
		uint32_t m_width;
		uint32_t m_height;
		HWND m_hwnd;
	};

	Window m_window;
};

typedef BackendWin32 Backend;


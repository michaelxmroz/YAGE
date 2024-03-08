#pragma once
#include "vulkan/vulkan.h"
#include <thread>

class BackendWin32
{
public:
	void InitWindow(uint32_t width, uint32_t height);
	void CleanupWindow();

	static void CreateSurface(VkInstance instance, VkSurfaceKHR& surface, HWND& hwnd);
	void CreateSurface(VkInstance instance, VkSurfaceKHR& surface);
	bool ProcessEvents();

	void SetWindowTitle(const char* title);
	HWND* GetWindowHandle();

	static std::string OpenFileLoadDialog(const wchar_t* fileTypeDescription, const wchar_t* fileTypeEndings);
	static std::string OpenFileSaveDialog(const wchar_t* fileTypeDescription, const wchar_t* fileTypeEndings, const wchar_t* fileExtension);

	static std::string GetPersistentDataPath();
private:

	struct WindowState
	{
		uint32_t m_width;
		uint32_t m_height;
		bool m_run;
		HWND m_hwnd;
	};

	class Window
	{
	public:
		void operator()();
		void CleanupWindow();
		WindowState m_state;
	};

	Window m_window;
	std::thread m_windowThread;
};

typedef BackendWin32 Backend;


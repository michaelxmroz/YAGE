#pragma once
#include "vulkan/vulkan.h"
#include <thread>

class BackendWin32
{
public:
	void InitWindow(uint32_t width, uint32_t height);
	void CleanupWindow();
	void CreateSurface(VkInstance instance, VkSurfaceKHR& surface);
	bool RequestQuit() const;
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


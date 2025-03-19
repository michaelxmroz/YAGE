#pragma once
#include <string>
#include <vector>
#include "Input.h"



#include "vulkan/vulkan.h"

class BackendWin32
{
public:

	struct WindowUserData
	{
		void* m_renderer;
		HICON m_icon;
	};

	void InitWindow(uint32_t width, uint32_t height, void* userData);
	void CleanupWindow();

	void ResizeWindow(uint32_t width, uint32_t height);
	void GetWindowSize(uint32_t& width, uint32_t& height);

	static void CreateSurface(VkInstance instance, VkSurfaceKHR& surface, HWND& hwnd);
	void CreateSurface(VkInstance instance, VkSurfaceKHR& surface);
	bool ProcessEvents(KeyBindRequest& keyBindingRequest);

	const std::unordered_map<uint32_t, bool>& GetInputEventMap();

	void SetWindowTitle(const char* title);
	HWND* GetWindowHandle();

	static std::string OpenFileLoadDialog(const wchar_t* fileTypeDescription, const wchar_t* fileTypeEndings);
	static std::string OpenFileSaveDialog(const wchar_t* fileTypeDescription, const wchar_t* fileTypeEndings, const wchar_t* fileExtension);

	static std::string GetPersistentDataPath();

	static void GetDefaultInputMapping(std::unordered_map<uint32_t, InputActions>& inputMapping);

	static std::string ConvertVirtualKeyToString(uint32_t virtualKey);
	static uint32_t ConvertCharToVirtualKey(char c);
private:

	class Window
	{
	public:
		uint32_t m_width;
		uint32_t m_height;
		HWND m_hwnd;
	};

	Window m_window;
	std::unordered_map<uint32_t, bool> m_rawInputEvents;
	uint32_t m_nextKeyDown;
	WindowUserData m_data;
};


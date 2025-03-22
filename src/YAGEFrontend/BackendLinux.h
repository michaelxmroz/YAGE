#pragma once

#include "PlatformDefines.h"

#if YAGE_PLATFORM_UNIX

#include <string>
#include <vector>
#include "Input.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>

#include "vulkan/vulkan.h"

class BackendLinux
{
public:
	struct WindowUserData
	{
		void* m_renderer;
		// Linux equivalent of icon handling if needed
	};

	void InitWindow(uint32_t width, uint32_t height, void* userData);
	void CleanupWindow();

	void ResizeWindow(uint32_t width, uint32_t height);
	void GetWindowSize(uint32_t& width, uint32_t& height);

	static void CreateSurface(VkInstance instance, VkSurfaceKHR& surface, Window& window, Display* display);
	void CreateSurface(VkInstance instance, VkSurfaceKHR& surface);
	bool ProcessEvents(KeyBindRequest& keyBindingRequest);

	const std::unordered_map<uint32_t, bool>& GetInputEventMap();

	void SetWindowTitle(const char* title);
	Window* GetWindowHandle();
	Display* GetDisplay();

	static std::string OpenFileLoadDialog(const wchar_t* fileTypeDescription, const wchar_t* fileTypeEndings);
	static std::string OpenFileSaveDialog(const wchar_t* fileTypeDescription, const wchar_t* fileTypeEndings, const wchar_t* fileExtension);
	
	// Add file dialog rendering function
	static void RenderFileDialog();
	
	// Add functions to check for and retrieve file dialog results
	static bool HasFileDialogResult();
	static std::string GetFileDialogResult();

	static std::string GetPersistentDataPath();

	static void GetDefaultInputMapping(std::unordered_map<uint32_t, InputActions>& inputMapping);

	static std::string ConvertKeysymToString(KeySym keysym);
	static KeySym ConvertCharToKeysym(char c);
	
	// Matching the Windows backend's function
	static std::string ConvertVirtualKeyToString(uint32_t virtualKey);
	
	// Helper functions for wchar_t conversions
	static std::string WideToUtf8(const wchar_t* wstr);
	static std::wstring Utf8ToWide(const std::string& str);
	
private:
	class LinuxWindow
	{
	public:
		uint32_t m_width;
		uint32_t m_height;
		Window m_window;
		Display* m_display;
		int m_screen;
		XVisualInfo* m_visualInfo;
		Colormap m_colormap;
		Atom m_wmDeleteMessage;
	};

	LinuxWindow m_window;
	std::unordered_map<uint32_t, bool> m_rawInputEvents;
	uint32_t m_nextKeyDown{0};
	WindowUserData m_data;
};

#endif // YAGE_PLATFORM_UNIX 
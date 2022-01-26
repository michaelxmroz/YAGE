#include "BackendWin32.h"
#include <stdexcept>


namespace Win32Internal
{
    LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
        case WM_CLOSE:
            PostQuitMessage(0);
            return 0;
        }
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    HWND CreateWin32Window(uint32_t width, uint32_t height, HMODULE& instanceID)
    {
        const wchar_t CLASS_NAME[] = L"GB Window Class";

        WNDCLASS wc = { };

        wc.lpfnWndProc = WindowProc;
        wc.hInstance = instanceID;
        wc.lpszClassName = CLASS_NAME;

        RegisterClass(&wc);

        RECT rect;
        rect.left = 100;
        rect.top = 100;
        rect.right = 100 + width;
        rect.bottom = 100 + height;

        AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, 0, 0);

        HWND hwnd = CreateWindowEx(
            0,                              // Optional window styles.
            CLASS_NAME,                     // Window class
            L"GameBoy",    // Window text
            WS_OVERLAPPEDWINDOW,            // Window style

            // Size and position
            rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,

            NULL,       // Parent window    
            NULL,       // Menu
            instanceID,  // Instance handle
            NULL        // Additional application data
        );

        if (hwnd == NULL)
        {
            return 0;
        }

        ShowWindow(hwnd, 1);
        return hwnd;
    }
}

void BackendWin32::InitWindow(uint32_t width, uint32_t height)
{
    m_window.m_state.m_width = width;
    m_window.m_state.m_height = height;
    m_windowThread = std::thread(std::ref(m_window));
}

void BackendWin32::CleanupWindow()
{
    m_window.m_state.m_run = false;
    m_windowThread.join();
    m_window.CleanupWindow();
}

void BackendWin32::CreateSurface(VkInstance instance, VkSurfaceKHR& surface)
{
    VkWin32SurfaceCreateInfoKHR surfaceInfo{ VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
    HMODULE instanceID = GetModuleHandle(nullptr);
    surfaceInfo.hwnd = m_window.m_state.m_hwnd;
    surfaceInfo.hinstance = instanceID;

    if (vkCreateWin32SurfaceKHR(instance, &surfaceInfo, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    };
}

bool BackendWin32::RequestQuit() const
{
    return !m_window.m_state.m_run;
}

void BackendWin32::Window::operator()()
{
    m_state.m_run = true;

    HMODULE instanceID = GetModuleHandle(nullptr);
    m_state.m_hwnd = Win32Internal::CreateWin32Window(m_state.m_width, m_state.m_height, instanceID);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0 && m_state.m_run)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    m_state.m_run = false;
}

void BackendWin32::Window::CleanupWindow()
{
    DestroyWindow(m_state.m_hwnd);
}

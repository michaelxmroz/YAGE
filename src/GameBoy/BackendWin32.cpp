#include "BackendWin32.h"
#include <stdexcept>
#include "imgui.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "Windows.h"
#define STRICT_TYPED_ITEMIDS
#include <shlobj.h>
#include <objbase.h>      // For COM headers
#include <shobjidl.h>     // for IFileDialogEvents and IFileDialogControlEvents
#include <shlwapi.h>
#include <knownfolders.h> // for KnownFolder APIs/datatypes/function headers
#include <propvarutil.h>  // for PROPVAR-related functions
#include <propkey.h>      // for the Property key APIs/datatypes
#include <propidl.h>      // for the Property System APIs
#include <strsafe.h>      // for StringCchPrintfW
#include <shtypes.h>      // for COMDLG_FILTERSPEC
#include <new>
#include "Logger.h"
#include "Logging.h"
#include <locale>
#include <codecvt>


extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Win32Internal
{
    std::string ConvertUTF16ToUTF8(const std::wstring& utf16String)
    {
        int requiredSize = WideCharToMultiByte(CP_UTF8, 0, utf16String.c_str(), -1, nullptr, 0, nullptr, nullptr);

        if (requiredSize == 0) {
            return std::string(); // Return an empty string in case of an error
        }

        std::string utf8String(requiredSize, 0);
        int result = WideCharToMultiByte(CP_UTF8, 0, utf16String.c_str(), -1, &utf8String[0], requiredSize, nullptr, nullptr);

        if (result == 0) {
            return std::string(); // Return an empty string in case of an error
        }

        return utf8String;
    }

    static void check_winapi_result(HRESULT res)
    {
        if (res >= 0)
            return;
        LOG_ERROR(string_format("[Winapi]: HResult = %d\n", res).c_str());
        if (res < 0)
            abort();
    }

    LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        if (ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam))
            return true;

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

        ::SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) & ~WS_SIZEBOX);

        ShowWindow(hwnd, 1);
        UpdateWindow(hwnd);
        return hwnd;
    }

    //Adapted from the Microsoft example code
    std::string OpenFileDialog(IFileDialog* pfd, DWORD options, const wchar_t* fileTypeDescription, const wchar_t* fileTypeEndings)
    {
        std::string result("");

        // Set the options on the dialog.
        DWORD dwFlags;
        // Before setting, always get the options first in order not to override existing options.
        Win32Internal::check_winapi_result(pfd->GetOptions(&dwFlags));

        // In this case, get shell items only for file system items.
        Win32Internal::check_winapi_result(pfd->SetOptions(dwFlags | options));

        // Set the file types to display only. Notice that, this is a 1-based array.
        const COMDLG_FILTERSPEC c_rgTypes[] = { fileTypeDescription, fileTypeEndings };
        Win32Internal::check_winapi_result(pfd->SetFileTypes(ARRAYSIZE(c_rgTypes), c_rgTypes));

        // Show the dialog
        HRESULT hr = pfd->Show(NULL);

        if (SUCCEEDED(hr))
        {
            // Obtain the result, once the user clicks the 'Open' button.
            // The result is an IShellItem object.
            IShellItem* psiResult;
            hr = pfd->GetResult(&psiResult);
            if (SUCCEEDED(hr))
            {
                PWSTR pszFilePath = NULL;
                hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                if (SUCCEEDED(hr))
                {
                    result = Win32Internal::ConvertUTF16ToUTF8(pszFilePath);
                }
                CoTaskMemFree(pszFilePath);
                psiResult->Release();
            }
        }

        return result;
    }
}

void BackendWin32::InitWindow(uint32_t width, uint32_t height)
{
    m_window.m_width = width;
    m_window.m_height = height;

    HMODULE instanceID = GetModuleHandle(nullptr);
    m_window.m_hwnd = Win32Internal::CreateWin32Window(width, height, instanceID);
}


void BackendWin32::CleanupWindow()
{
    DestroyWindow(m_window.m_hwnd);
}

void BackendWin32::ResizeWindow(uint32_t width, uint32_t height)
{
    m_window.m_width = width;
    m_window.m_height = height;
    RECT rect;
    ::GetWindowRect(m_window.m_hwnd, &rect);
    rect.left = 100;
    rect.top = 100;
    rect.right = rect.left + width;
    rect.bottom = rect.top + height;

    AdjustWindowRectEx(&rect, WS_OVERLAPPEDWINDOW, 0, 0);
    SetWindowPos(m_window.m_hwnd, HWND_TOP, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_SHOWWINDOW);
}


void BackendWin32::CreateSurface(VkInstance instance, VkSurfaceKHR& surface, HWND& hwnd)
{
VkWin32SurfaceCreateInfoKHR surfaceInfo{ VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR };
	HMODULE instanceID = GetModuleHandle(nullptr);
	surfaceInfo.hwnd = hwnd;
	surfaceInfo.hinstance = instanceID;

	if (vkCreateWin32SurfaceKHR(instance, &surfaceInfo, nullptr, &surface) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface!");
	};
}

void BackendWin32::CreateSurface(VkInstance instance, VkSurfaceKHR& surface)
{
    BackendWin32::CreateSurface(instance, surface, m_window.m_hwnd);
}

void BackendWin32::SetWindowTitle(const char* title)
{
    SetWindowTextA(m_window.m_hwnd, title);
}

HWND* BackendWin32::GetWindowHandle()
{
    return &m_window.m_hwnd;
}


bool BackendWin32::ProcessEvents()
{
    MSG msg = {};
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) 
    {
        if (msg.message == WM_QUIT) {
            return false;
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return true;
}

std::string BackendWin32::OpenFileLoadDialog(const wchar_t* fileTypeDescription, const wchar_t* fileTypeEndings)
{
    IFileDialog* pfd = NULL;

    Win32Internal::check_winapi_result(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd)));

    std::string result = Win32Internal::OpenFileDialog(pfd, FOS_FORCEFILESYSTEM, fileTypeDescription, fileTypeEndings);
    pfd->Release();

    return result;
}

std::string BackendWin32::OpenFileSaveDialog(const wchar_t* fileTypeDescription, const wchar_t* fileTypeEndings, const wchar_t* fileExtension)
{
    IFileSaveDialog* pfd = NULL;

    Win32Internal::check_winapi_result(CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd)));
    pfd->SetDefaultExtension(fileExtension);

    std::string result = Win32Internal::OpenFileDialog(pfd, NULL, fileTypeDescription, fileTypeEndings);
    pfd->Release();

    return result;
}

std::string BackendWin32::GetPersistentDataPath()
{
    PWSTR pszFilePath = NULL;
    SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, &pszFilePath);
    std::string path = Win32Internal::ConvertUTF16ToUTF8(pszFilePath);
    CoTaskMemFree(pszFilePath);
    return path;
}
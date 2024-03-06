#include "BackendWin32.h"
#include <stdexcept>
#include "imgui.h"

//#ifndef WIN32_LEAN_AND_MEAN
//#define WIN32_LEAN_AND_MEAN
//#endif
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

#pragma comment(linker, "\"/manifestdependency:type='Win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")


extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Win32Internal
{
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

        ShowWindow(hwnd, 1);
        return hwnd;
    }


    // Indices of file types
#define INDEX_WORDDOC 1
#define INDEX_WEBPAGE 2
#define INDEX_TEXTDOC 3

// Controls
#define CONTROL_GROUP           2000
#define CONTROL_RADIOBUTTONLIST 2
#define CONTROL_RADIOBUTTON1    1
#define CONTROL_RADIOBUTTON2    2       // It is OK for this to have the same ID as CONTROL_RADIOBUTTONLIST,
                                        // because it is a child control under CONTROL_RADIOBUTTONLIST

// IDs for the Task Dialog Buttons
#define IDC_BASICFILEOPEN                       100
#define IDC_ADDITEMSTOCUSTOMPLACES              101
#define IDC_ADDCUSTOMCONTROLS                   102
#define IDC_SETDEFAULTVALUESFORPROPERTIES       103
#define IDC_WRITEPROPERTIESUSINGHANDLERS        104
#define IDC_WRITEPROPERTIESWITHOUTUSINGHANDLERS 105

    /* File Dialog Event Handler *****************************************************************************************************/

    class CDialogEventHandler : public IFileDialogEvents,
        public IFileDialogControlEvents
    {
    public:
        // IUnknown methods
        IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv)
        {
            static const QITAB qit[] = {
                QITABENT(CDialogEventHandler, IFileDialogEvents),
                QITABENT(CDialogEventHandler, IFileDialogControlEvents),
                { 0 },
    #pragma warning(suppress:4838)
            };
            return QISearch(this, qit, riid, ppv);
        }

        IFACEMETHODIMP_(ULONG) AddRef()
        {
            return InterlockedIncrement(&_cRef);
        }

        IFACEMETHODIMP_(ULONG) Release()
        {
            long cRef = InterlockedDecrement(&_cRef);
            if (!cRef)
                delete this;
            return cRef;
        }

        // IFileDialogEvents methods
        IFACEMETHODIMP OnFileOk(IFileDialog*) { return S_OK; };
        IFACEMETHODIMP OnFolderChange(IFileDialog*) { return S_OK; };
        IFACEMETHODIMP OnFolderChanging(IFileDialog*, IShellItem*) { return S_OK; };
        IFACEMETHODIMP OnHelp(IFileDialog*) { return S_OK; };
        IFACEMETHODIMP OnSelectionChange(IFileDialog*) { return S_OK; };
        IFACEMETHODIMP OnShareViolation(IFileDialog*, IShellItem*, FDE_SHAREVIOLATION_RESPONSE*) { return S_OK; };
        IFACEMETHODIMP OnTypeChange(IFileDialog* pfd);
        IFACEMETHODIMP OnOverwrite(IFileDialog*, IShellItem*, FDE_OVERWRITE_RESPONSE*) { return S_OK; };

        // IFileDialogControlEvents methods
        IFACEMETHODIMP OnItemSelected(IFileDialogCustomize* pfdc, DWORD dwIDCtl, DWORD dwIDItem);
        IFACEMETHODIMP OnButtonClicked(IFileDialogCustomize*, DWORD) { return S_OK; };
        IFACEMETHODIMP OnCheckButtonToggled(IFileDialogCustomize*, DWORD, BOOL) { return S_OK; };
        IFACEMETHODIMP OnControlActivating(IFileDialogCustomize*, DWORD) { return S_OK; };

        CDialogEventHandler() : _cRef(1) { };
    private:
        ~CDialogEventHandler() { };
        long _cRef;
    };

    // IFileDialogEvents methods
    // This method gets called when the file-type is changed (combo-box selection changes).
    // For sample sake, let's react to this event by changing the properties show.
    HRESULT CDialogEventHandler::OnTypeChange(IFileDialog* pfd)
    {
        IFileSaveDialog* pfsd;
        HRESULT hr = pfd->QueryInterface(&pfsd);
        if (SUCCEEDED(hr))
        {
            UINT uIndex;
            hr = pfsd->GetFileTypeIndex(&uIndex);   // index of current file-type
            if (SUCCEEDED(hr))
            {
                IPropertyDescriptionList* pdl = NULL;

                switch (uIndex)
                {
                case INDEX_WORDDOC:
                    // When .doc is selected, let's ask for some arbitrary property, say Title.
                    hr = PSGetPropertyDescriptionListFromString(L"prop:System.Title", IID_PPV_ARGS(&pdl));
                    if (SUCCEEDED(hr))
                    {
                        // FALSE as second param == do not show default properties.
                        hr = pfsd->SetCollectedProperties(pdl, FALSE);
                        pdl->Release();
                    }
                    break;

                case INDEX_WEBPAGE:
                    // When .html is selected, let's ask for some other arbitrary property, say Keywords.
                    hr = PSGetPropertyDescriptionListFromString(L"prop:System.Keywords", IID_PPV_ARGS(&pdl));
                    if (SUCCEEDED(hr))
                    {
                        // FALSE as second param == do not show default properties.
                        hr = pfsd->SetCollectedProperties(pdl, FALSE);
                        pdl->Release();
                    }
                    break;

                case INDEX_TEXTDOC:
                    // When .txt is selected, let's ask for some other arbitrary property, say Author.
                    hr = PSGetPropertyDescriptionListFromString(L"prop:System.Author", IID_PPV_ARGS(&pdl));
                    if (SUCCEEDED(hr))
                    {
                        // TRUE as second param == show default properties as well, but show Author property first in list.
                        hr = pfsd->SetCollectedProperties(pdl, TRUE);
                        pdl->Release();
                    }
                    break;
                }
            }
            pfsd->Release();
        }
        return hr;
    }

    // IFileDialogControlEvents
    // This method gets called when an dialog control item selection happens (radio-button selection. etc).
    // For sample sake, let's react to this event by changing the dialog title.
    HRESULT CDialogEventHandler::OnItemSelected(IFileDialogCustomize* pfdc, DWORD dwIDCtl, DWORD dwIDItem)
    {
        IFileDialog* pfd = NULL;
        HRESULT hr = pfdc->QueryInterface(&pfd);
        if (SUCCEEDED(hr))
        {
            if (dwIDCtl == CONTROL_RADIOBUTTONLIST)
            {
                switch (dwIDItem)
                {
                case CONTROL_RADIOBUTTON1:
                    hr = pfd->SetTitle(L"Longhorn Dialog");
                    break;

                case CONTROL_RADIOBUTTON2:
                    hr = pfd->SetTitle(L"Vista Dialog");
                    break;
                }
            }
            pfd->Release();
        }
        return hr;
    }

    // Instance creation helper
    HRESULT CDialogEventHandler_CreateInstance(REFIID riid, void** ppv)
    {
        *ppv = NULL;
        CDialogEventHandler* pDialogEventHandler = new (std::nothrow) CDialogEventHandler();
        HRESULT hr = pDialogEventHandler ? S_OK : E_OUTOFMEMORY;
        if (SUCCEEDED(hr))
        {
            hr = pDialogEventHandler->QueryInterface(riid, ppv);
            pDialogEventHandler->Release();
        }
        return hr;
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

void BackendWin32::SetWindowTitle(const char* title)
{
    SetWindowTextA(m_window.m_state.m_hwnd, title);
}

HWND* BackendWin32::GetWindowHandle()
{
    return &m_window.m_state.m_hwnd;
}

const COMDLG_FILTERSPEC c_rgSaveTypes[] =
{
    {L"Word Document (*.doc)",       L"*.doc"},
    {L"Web Page (*.htm; *.html)",    L"*.htm;*.html"},
    {L"Text Document (*.txt)",       L"*.txt"},
    {L"All Documents (*.*)",         L"*.*"}
};

std::string BackendWin32::OpenFileDialog()
{
    // CoCreate the File Open Dialog object.
    IFileDialog* pfd = NULL;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
    if (SUCCEEDED(hr))
    {
        // Create an event handling object, and hook it up to the dialog.
        IFileDialogEvents* pfde = NULL;
        hr = Win32Internal::CDialogEventHandler_CreateInstance(IID_PPV_ARGS(&pfde));
        if (SUCCEEDED(hr))
        {
            // Hook up the event handler.
            DWORD dwCookie;
            hr = pfd->Advise(pfde, &dwCookie);
            if (SUCCEEDED(hr))
            {
                // Set the options on the dialog.
                DWORD dwFlags;

                // Before setting, always get the options first in order not to override existing options.
                hr = pfd->GetOptions(&dwFlags);
                if (SUCCEEDED(hr))
                {
                    // In this case, get shell items only for file system items.
                    hr = pfd->SetOptions(dwFlags | FOS_FORCEFILESYSTEM);
                    if (SUCCEEDED(hr))
                    {
                        // Set the file types to display only. Notice that, this is a 1-based array.
                        hr = pfd->SetFileTypes(ARRAYSIZE(c_rgSaveTypes), c_rgSaveTypes);
                        if (SUCCEEDED(hr))
                        {
                            // Set the selected file type index to Word Docs for this example.
                            hr = pfd->SetFileTypeIndex(INDEX_WORDDOC);
                            if (SUCCEEDED(hr))
                            {
                                // Set the default extension to be ".doc" file.
                                hr = pfd->SetDefaultExtension(L"doc");
                                if (SUCCEEDED(hr))
                                {
                                    // Show the dialog
                                    hr = pfd->Show(NULL);
                                    if (SUCCEEDED(hr))
                                    {
                                        // Obtain the result, once the user clicks the 'Open' button.
                                        // The result is an IShellItem object.
                                        IShellItem* psiResult;
                                        hr = pfd->GetResult(&psiResult);
                                        if (SUCCEEDED(hr))
                                        {
                                            // We are just going to print out the name of the file for sample sake.
                                            PWSTR pszFilePath = NULL;
                                            hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                                            if (SUCCEEDED(hr))
                                            {
                                                TaskDialog(NULL,
                                                    NULL,
                                                    L"CommonFileDialogApp",
                                                    pszFilePath,
                                                    NULL,
                                                    TDCBF_OK_BUTTON,
                                                    TD_INFORMATION_ICON,
                                                    NULL);
                                                CoTaskMemFree(pszFilePath);
                                            }
                                            psiResult->Release();
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                // Unhook the event handler.
                pfd->Unadvise(dwCookie);
            }
            pfde->Release();
        }
        pfd->Release();
    }
    return "";
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

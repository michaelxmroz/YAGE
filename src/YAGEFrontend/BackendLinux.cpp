#include "BackendLinux.h"
#include "Input.h"
#include "ImGuiFileBrowser.h"
#include "PlatformDefines.h"

#if YAGE_PLATFORM_UNIX
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include <vulkan/vulkan_xlib.h>
#include <stdexcept>
#include <unistd.h>
#include <pwd.h>
#include "imgui.h"
#include <cstring>
#include <sys/stat.h>
#include <limits.h>
#include "Logger.h"
#include "Logging.h"
#include "volk.h"

// Global file browser instance for dialogs
static ImGuiFileBrowser g_FileBrowser;

extern void ResizeWindowProcHandler(void* userData, bool isMinimizing);

// Function to handle X11 errors
int X11ErrorHandler(Display*, XErrorEvent*) {
    return 0; // Ignore X11 errors
}

void BackendLinux::InitWindow(uint32_t width, uint32_t height, void* userData)
{
    // Set up X11 error handler
    XSetErrorHandler(X11ErrorHandler);

    // Open X display
    m_window.m_display = XOpenDisplay(nullptr);
    if (m_window.m_display == nullptr) {
        throw std::runtime_error("Failed to open X display");
    }

    // Get screen
    m_window.m_screen = DefaultScreen(m_window.m_display);

    // Create window
    Window root = RootWindow(m_window.m_display, m_window.m_screen);
    
    // Set up visual info
    XVisualInfo visualInfo;
    visualInfo.screen = m_window.m_screen;
    int visualCount;
    XVisualInfo* visuals = XGetVisualInfo(m_window.m_display, VisualScreenMask, &visualInfo, &visualCount);
    if (visualCount == 0) {
        throw std::runtime_error("Failed to get X visual info");
    }
    m_window.m_visualInfo = new XVisualInfo;
    memcpy(m_window.m_visualInfo, &visuals[0], sizeof(XVisualInfo));
    XFree(visuals);

    // Create colormap
    m_window.m_colormap = XCreateColormap(m_window.m_display, root, m_window.m_visualInfo->visual, AllocNone);

    // Set window attributes
    XSetWindowAttributes windowAttribs;
    windowAttribs.colormap = m_window.m_colormap;
    windowAttribs.background_pixel = 0;
    windowAttribs.border_pixel = 0;
    windowAttribs.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | 
                              ButtonPressMask | ButtonReleaseMask | PointerMotionMask |
                              StructureNotifyMask;

    // Create the window
    m_window.m_window = XCreateWindow(
        m_window.m_display, root,
        0, 0, width, height, 0,
        m_window.m_visualInfo->depth, InputOutput, 
        m_window.m_visualInfo->visual,
        CWColormap | CWBackPixel | CWBorderPixel | CWEventMask, &windowAttribs
    );

    // Set window title
    XStoreName(m_window.m_display, m_window.m_window, "YAGE Emulator");

    // Set window protocols for proper closing
    m_window.m_wmDeleteMessage = XInternAtom(m_window.m_display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(m_window.m_display, m_window.m_window, &m_window.m_wmDeleteMessage, 1);

    // Show the window
    XMapWindow(m_window.m_display, m_window.m_window);
    XFlush(m_window.m_display);

    // Store window dimensions
    m_window.m_width = width;
    m_window.m_height = height;

    // Store user data
    m_data.m_renderer = userData;
}

void BackendLinux::CleanupWindow()
{
    // Clean up X11 resources
    if (m_window.m_visualInfo) {
        delete m_window.m_visualInfo;
        m_window.m_visualInfo = nullptr;
    }

    if (m_window.m_display) {
        XDestroyWindow(m_window.m_display, m_window.m_window);
        XFreeColormap(m_window.m_display, m_window.m_colormap);
        XCloseDisplay(m_window.m_display);
        m_window.m_display = nullptr;
    }
}

void BackendLinux::ResizeWindow(uint32_t width, uint32_t height)
{
    if (m_window.m_display && m_window.m_window) {
        XResizeWindow(m_window.m_display, m_window.m_window, width, height);
        m_window.m_width = width;
        m_window.m_height = height;
    }
}

void BackendLinux::GetWindowSize(uint32_t& width, uint32_t& height)
{
    width = m_window.m_width;
    height = m_window.m_height;
}

void BackendLinux::CreateSurface(VkInstance instance, VkSurfaceKHR& surface)
{
    if (!m_window.m_display || !m_window.m_window) {
        throw std::runtime_error("Window not initialized!");
    }

    VkXlibSurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    createInfo.dpy = m_window.m_display;
    createInfo.window = m_window.m_window;

    if (vkCreateXlibSurfaceKHR(instance, &createInfo, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface!");
    }
}

void BackendLinux::CreateSurface(VkInstance instance, VkSurfaceKHR& surface, Window& window, Display* display)
{
    VkXlibSurfaceCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    createInfo.dpy = display;
    createInfo.window = window;

    if (vkCreateXlibSurfaceKHR(instance, &createInfo, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface!");
    }
}

bool BackendLinux::ProcessEvents(KeyBindRequest& keyBindingRequest)
{
    bool shouldExit = false;
    
    // Clear previous events
    m_nextKeyDown = 0;
    
    XEvent event;
    while (XPending(m_window.m_display) > 0) {
        XNextEvent(m_window.m_display, &event);
        
        switch (event.type) {
            case KeyPress: {
                KeySym keysym = XkbKeycodeToKeysym(m_window.m_display, event.xkey.keycode, 0, 0);
                uint32_t key = static_cast<uint32_t>(keysym);
                m_rawInputEvents[key] = true;
                m_nextKeyDown = key;
                break;
            }
            case KeyRelease: {
                KeySym keysym = XkbKeycodeToKeysym(m_window.m_display, event.xkey.keycode, 0, 0);
                uint32_t key = static_cast<uint32_t>(keysym);
                m_rawInputEvents[key] = false;
                break;
            }
            case ConfigureNotify: {
                if (m_window.m_width != event.xconfigure.width || 
                    m_window.m_height != event.xconfigure.height) {
                    m_window.m_width = event.xconfigure.width;
                    m_window.m_height = event.xconfigure.height;
                    ResizeWindowProcHandler(m_data.m_renderer, false);
                }
                break;
            }
            case ClientMessage: {
                if (event.xclient.data.l[0] == static_cast<long>(m_window.m_wmDeleteMessage)) {
                    shouldExit = true;
                }
                break;
            }
        }
    }
    
    if (keyBindingRequest.m_status == KeyBindRequest::RequestStatus::REQUESTED && m_nextKeyDown != 0) {
        keyBindingRequest.m_keyCode = m_nextKeyDown;
        keyBindingRequest.m_status = KeyBindRequest::RequestStatus::CONFIRMED;
    }
    
    return shouldExit;
}

const std::unordered_map<uint32_t, bool>& BackendLinux::GetInputEventMap()
{
    return m_rawInputEvents;
}

void BackendLinux::SetWindowTitle(const char* title)
{
    if (m_window.m_display && m_window.m_window) {
        XStoreName(m_window.m_display, m_window.m_window, title);
    }
}

Window* BackendLinux::GetWindowHandle()
{
    return &m_window.m_window;
}

Display* BackendLinux::GetDisplay()
{
    return m_window.m_display;
}

std::string BackendLinux::WideToUtf8(const wchar_t* wstr)
{
    if (!wstr) return std::string();
    
    // Determine required buffer size
    size_t size_needed = wcstombs(nullptr, wstr, 0) + 1;
    if (size_needed == 0) return std::string();
    
    // Allocate buffer
    std::string result(size_needed, 0);
    
    // Convert wide string to UTF-8
    size_t bytes_written = wcstombs(&result[0], wstr, size_needed);
    if (bytes_written == (size_t)-1) return std::string();
    
    // Resize to actual length (excluding null terminator)
    result.resize(bytes_written);
    return result;
}

std::wstring BackendLinux::Utf8ToWide(const std::string& str)
{
    if (str.empty()) return std::wstring();
    
    // Determine required buffer size
    size_t size_needed = mbstowcs(nullptr, str.c_str(), 0) + 1;
    if (size_needed == 0) return std::wstring();
    
    // Allocate buffer
    std::wstring result(size_needed, 0);
    
    // Convert UTF-8 to wide string
    size_t chars_written = mbstowcs(&result[0], str.c_str(), size_needed);
    if (chars_written == (size_t)-1) return std::wstring();
    
    // Resize to actual length (excluding null terminator)
    result.resize(chars_written);
    return result;
}

std::string BackendLinux::OpenFileLoadDialog(const wchar_t* fileTypeDescription, const wchar_t* fileTypeEndings)
{
    // Convert wide strings to UTF-8
    std::string description = WideToUtf8(fileTypeDescription);
    std::string endings = WideToUtf8(fileTypeEndings);
    
    // Open file dialog using ImGui
    g_FileBrowser.OpenDialog("Open File", description.c_str(), endings.c_str());
    
    // Dialog will be rendered during the next frame's ImGui rendering
    // Return empty string now, actual path will be returned after dialog is closed
    return "";
}

std::string BackendLinux::OpenFileSaveDialog(const wchar_t* fileTypeDescription, const wchar_t* fileTypeEndings, const wchar_t* fileExtension)
{
    // Convert wide strings to UTF-8
    std::string description = WideToUtf8(fileTypeDescription);
    std::string endings = WideToUtf8(fileTypeEndings);
    std::string extension = WideToUtf8(fileExtension);
    
    // Open save dialog using ImGui
    g_FileBrowser.SaveDialog("Save File", description.c_str(), endings.c_str(), extension.c_str());
    
    // Dialog will be rendered during the next frame's ImGui rendering
    // Return empty string now, actual path will be returned after dialog is closed
    return "";
}

// Add file dialog rendering function
void BackendLinux::RenderFileDialog()
{
    static std::string s_selectedFilePath;
    static bool s_fileSelected = false;
    
    ImGuiFileBrowser::FileDialogResult result = g_FileBrowser.Render();
    if (result == ImGuiFileBrowser::FileDialogResult::OK) {
        // Dialog was confirmed, store the selected file
        s_selectedFilePath = g_FileBrowser.GetSelectedFile();
        s_fileSelected = true;
    }
}

// Add functions to check for and retrieve file dialog results
bool BackendLinux::HasFileDialogResult()
{
    static bool s_fileSelected = false;
    bool result = s_fileSelected;
    if (result) {
        s_fileSelected = false;
    }
    return result;
}

std::string BackendLinux::GetFileDialogResult()
{
    static std::string s_selectedFilePath;
    std::string result = s_selectedFilePath;
    s_selectedFilePath.clear();
    return result;
}

std::string BackendLinux::GetPersistentDataPath()
{
    const char* homeDir = getenv("HOME");
    
    if (!homeDir) {
        struct passwd* pw = getpwuid(getuid());
        if (pw) {
            homeDir = pw->pw_dir;
        }
    }
    
    if (!homeDir) {
        return "";
    }
    
    std::string path = std::string(homeDir) + "/.config/yage";
    
    // Create directory if it doesn't exist
    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        mkdir(path.c_str(), 0755);
    }
    
    return path;
}

void BackendLinux::GetDefaultInputMapping(std::unordered_map<uint32_t, InputActions>& inputMapping)
{
    // Default key mapping for Linux
    inputMapping[XK_Up] = InputActions::Up;
    inputMapping[XK_Down] = InputActions::Down;
    inputMapping[XK_Left] = InputActions::Left;
    inputMapping[XK_Right] = InputActions::Right;
    inputMapping[XK_x] = InputActions::A;
    inputMapping[XK_z] = InputActions::B;
    inputMapping[XK_BackSpace] = InputActions::Select;
    inputMapping[XK_Return] = InputActions::Start;
    inputMapping[XK_space] = InputActions::Pause;
    inputMapping[XK_Tab] = InputActions::Turbo;
    inputMapping[XK_F5] = InputActions::QuickSave;
    inputMapping[XK_F9] = InputActions::QuickLoad;
}

std::string BackendLinux::ConvertKeysymToString(KeySym keysym)
{
    char buffer[64];
    std::string keyName;
    
    // Handle special cases
    switch (keysym) {
        case XK_BackSpace: return "Backspace";
        case XK_Tab: return "Tab";
        case XK_Return: return "Return";
        case XK_Escape: return "Escape";
        case XK_space: return "Space";
        case XK_Up: return "Up";
        case XK_Down: return "Down";
        case XK_Left: return "Left";
        case XK_Right: return "Right";
        // Add more special cases as needed
    }
    
    // Try to get key name from X11
    if (char* name = XKeysymToString(keysym)) {
        keyName = name;
        return keyName;
    }
    
    // Fallback
    snprintf(buffer, sizeof(buffer), "Key_%lu", keysym);
    return buffer;
}

std::string BackendLinux::ConvertVirtualKeyToString(uint32_t virtualKey)
{
    // In our Linux backend, virtual keys are KeySym values
    return ConvertKeysymToString(static_cast<KeySym>(virtualKey));
}

KeySym BackendLinux::ConvertCharToKeysym(char c)
{
    return XStringToKeysym(&c);
}

// Make sure to close the ifdef at the end of the file
#endif // YAGE_PLATFORM_UNIX 
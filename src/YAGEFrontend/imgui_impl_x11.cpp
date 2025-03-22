#include "imgui_impl_x11.h"
#include "imgui.h"

#include "PlatformDefines.h"

#if YAGE_PLATFORM_UNIX

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/XKBlib.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_xlib.h>
#include <cstring>
#include <unordered_map>

// X11 data
struct ImGui_ImplX11_Data
{
    Display*    display;
    Window      window;
    Cursor      cursor_default;
    Cursor      cursor_hidden;

    // X11 cursors
    Cursor      cursors[ImGuiMouseCursor_COUNT];
    bool        cursor_changed;
    
    // Time data
    Time        time;
    Time        last_time;
    
    // Clipboard handling
    Atom        clipboard_atom;
    Atom        targets_atom;
    Atom        text_atom;
    Atom        utf8_atom;
    
    ImGui_ImplX11_Data() { memset(this, 0, sizeof(*this)); }
};

// Global backend data
static ImGui_ImplX11_Data* g_ImplX11Data = nullptr;

// Helper functions
static const char* ImGui_ImplX11_GetClipboardText(void* user_data)
{
    // Request clipboard content
    ImGui_ImplX11_Data* data = g_ImplX11Data;
    if (!data || !data->display)
        return nullptr;
        
    Window owner = XGetSelectionOwner(data->display, data->clipboard_atom);
    if (owner == None)
        return nullptr;

    // Not yet implemented fully - would need to handle X11 selection/clipboard events
    return "";
}

static void ImGui_ImplX11_SetClipboardText(void* user_data, const char* text)
{
    // Not yet implemented fully - would need to handle X11 selection/clipboard events
    ImGui_ImplX11_Data* data = g_ImplX11Data;
    if (!data || !data->display || !text)
        return;
}

// Map X11 keysyms to ImGui keys
static std::unordered_map<KeySym, ImGuiKey> g_KeysymMap;

static void ImGui_ImplX11_InitKeyMap()
{
    g_KeysymMap[XK_Tab] = ImGuiKey_Tab;
    g_KeysymMap[XK_Left] = ImGuiKey_LeftArrow;
    g_KeysymMap[XK_Right] = ImGuiKey_RightArrow;
    g_KeysymMap[XK_Up] = ImGuiKey_UpArrow;
    g_KeysymMap[XK_Down] = ImGuiKey_DownArrow;
    g_KeysymMap[XK_Page_Up] = ImGuiKey_PageUp;
    g_KeysymMap[XK_Page_Down] = ImGuiKey_PageDown;
    g_KeysymMap[XK_Home] = ImGuiKey_Home;
    g_KeysymMap[XK_End] = ImGuiKey_End;
    g_KeysymMap[XK_Insert] = ImGuiKey_Insert;
    g_KeysymMap[XK_Delete] = ImGuiKey_Delete;
    g_KeysymMap[XK_BackSpace] = ImGuiKey_Backspace;
    g_KeysymMap[XK_space] = ImGuiKey_Space;
    g_KeysymMap[XK_Return] = ImGuiKey_Enter;
    g_KeysymMap[XK_Escape] = ImGuiKey_Escape;
    g_KeysymMap[XK_apostrophe] = ImGuiKey_Apostrophe;
    g_KeysymMap[XK_comma] = ImGuiKey_Comma;
    g_KeysymMap[XK_minus] = ImGuiKey_Minus;
    g_KeysymMap[XK_period] = ImGuiKey_Period;
    g_KeysymMap[XK_slash] = ImGuiKey_Slash;
    g_KeysymMap[XK_semicolon] = ImGuiKey_Semicolon;
    g_KeysymMap[XK_equal] = ImGuiKey_Equal;
    g_KeysymMap[XK_bracketleft] = ImGuiKey_LeftBracket;
    g_KeysymMap[XK_backslash] = ImGuiKey_Backslash;
    g_KeysymMap[XK_bracketright] = ImGuiKey_RightBracket;
    g_KeysymMap[XK_grave] = ImGuiKey_GraveAccent;
    g_KeysymMap[XK_Caps_Lock] = ImGuiKey_CapsLock;
    g_KeysymMap[XK_Scroll_Lock] = ImGuiKey_ScrollLock;
    g_KeysymMap[XK_Num_Lock] = ImGuiKey_NumLock;
    g_KeysymMap[XK_Print] = ImGuiKey_PrintScreen;
    g_KeysymMap[XK_Pause] = ImGuiKey_Pause;
    g_KeysymMap[XK_KP_0] = ImGuiKey_Keypad0;
    g_KeysymMap[XK_KP_1] = ImGuiKey_Keypad1;
    g_KeysymMap[XK_KP_2] = ImGuiKey_Keypad2;
    g_KeysymMap[XK_KP_3] = ImGuiKey_Keypad3;
    g_KeysymMap[XK_KP_4] = ImGuiKey_Keypad4;
    g_KeysymMap[XK_KP_5] = ImGuiKey_Keypad5;
    g_KeysymMap[XK_KP_6] = ImGuiKey_Keypad6;
    g_KeysymMap[XK_KP_7] = ImGuiKey_Keypad7;
    g_KeysymMap[XK_KP_8] = ImGuiKey_Keypad8;
    g_KeysymMap[XK_KP_9] = ImGuiKey_Keypad9;
    g_KeysymMap[XK_KP_Decimal] = ImGuiKey_KeypadDecimal;
    g_KeysymMap[XK_KP_Divide] = ImGuiKey_KeypadDivide;
    g_KeysymMap[XK_KP_Multiply] = ImGuiKey_KeypadMultiply;
    g_KeysymMap[XK_KP_Subtract] = ImGuiKey_KeypadSubtract;
    g_KeysymMap[XK_KP_Add] = ImGuiKey_KeypadAdd;
    g_KeysymMap[XK_KP_Enter] = ImGuiKey_KeypadEnter;
    g_KeysymMap[XK_KP_Equal] = ImGuiKey_KeypadEqual;
    g_KeysymMap[XK_Control_L] = ImGuiKey_LeftCtrl;
    g_KeysymMap[XK_Shift_L] = ImGuiKey_LeftShift;
    g_KeysymMap[XK_Alt_L] = ImGuiKey_LeftAlt;
    g_KeysymMap[XK_Super_L] = ImGuiKey_LeftSuper;
    g_KeysymMap[XK_Control_R] = ImGuiKey_RightCtrl;
    g_KeysymMap[XK_Shift_R] = ImGuiKey_RightShift;
    g_KeysymMap[XK_Alt_R] = ImGuiKey_RightAlt;
    g_KeysymMap[XK_Super_R] = ImGuiKey_RightSuper;
    g_KeysymMap[XK_Menu] = ImGuiKey_Menu;
    
    // Add numeric keys 0-9
    g_KeysymMap[XK_0] = ImGuiKey_0;
    g_KeysymMap[XK_1] = ImGuiKey_1;
    g_KeysymMap[XK_2] = ImGuiKey_2;
    g_KeysymMap[XK_3] = ImGuiKey_3;
    g_KeysymMap[XK_4] = ImGuiKey_4;
    g_KeysymMap[XK_5] = ImGuiKey_5;
    g_KeysymMap[XK_6] = ImGuiKey_6;
    g_KeysymMap[XK_7] = ImGuiKey_7;
    g_KeysymMap[XK_8] = ImGuiKey_8;
    g_KeysymMap[XK_9] = ImGuiKey_9;
    
    // Add letter keys A-Z
    g_KeysymMap[XK_a] = ImGuiKey_A;
    g_KeysymMap[XK_b] = ImGuiKey_B;
    g_KeysymMap[XK_c] = ImGuiKey_C;
    g_KeysymMap[XK_d] = ImGuiKey_D;
    g_KeysymMap[XK_e] = ImGuiKey_E;
    g_KeysymMap[XK_f] = ImGuiKey_F;
    g_KeysymMap[XK_g] = ImGuiKey_G;
    g_KeysymMap[XK_h] = ImGuiKey_H;
    g_KeysymMap[XK_i] = ImGuiKey_I;
    g_KeysymMap[XK_j] = ImGuiKey_J;
    g_KeysymMap[XK_k] = ImGuiKey_K;
    g_KeysymMap[XK_l] = ImGuiKey_L;
    g_KeysymMap[XK_m] = ImGuiKey_M;
    g_KeysymMap[XK_n] = ImGuiKey_N;
    g_KeysymMap[XK_o] = ImGuiKey_O;
    g_KeysymMap[XK_p] = ImGuiKey_P;
    g_KeysymMap[XK_q] = ImGuiKey_Q;
    g_KeysymMap[XK_r] = ImGuiKey_R;
    g_KeysymMap[XK_s] = ImGuiKey_S;
    g_KeysymMap[XK_t] = ImGuiKey_T;
    g_KeysymMap[XK_u] = ImGuiKey_U;
    g_KeysymMap[XK_v] = ImGuiKey_V;
    g_KeysymMap[XK_w] = ImGuiKey_W;
    g_KeysymMap[XK_x] = ImGuiKey_X;
    g_KeysymMap[XK_y] = ImGuiKey_Y;
    g_KeysymMap[XK_z] = ImGuiKey_Z;

    // Add function keys
    g_KeysymMap[XK_F1] = ImGuiKey_F1;
    g_KeysymMap[XK_F2] = ImGuiKey_F2;
    g_KeysymMap[XK_F3] = ImGuiKey_F3;
    g_KeysymMap[XK_F4] = ImGuiKey_F4;
    g_KeysymMap[XK_F5] = ImGuiKey_F5;
    g_KeysymMap[XK_F6] = ImGuiKey_F6;
    g_KeysymMap[XK_F7] = ImGuiKey_F7;
    g_KeysymMap[XK_F8] = ImGuiKey_F8;
    g_KeysymMap[XK_F9] = ImGuiKey_F9;
    g_KeysymMap[XK_F10] = ImGuiKey_F10;
    g_KeysymMap[XK_F11] = ImGuiKey_F11;
    g_KeysymMap[XK_F12] = ImGuiKey_F12;
}

bool ImGui_ImplX11_Init(Display* display, Window window)
{
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.BackendPlatformUserData == nullptr && "Already initialized a platform backend!");

    // Setup backend data and store in io.BackendPlatformUserData
    ImGui_ImplX11_Data* data = IM_NEW(ImGui_ImplX11_Data)();
    g_ImplX11Data = data;
    io.BackendPlatformUserData = (void*)data;
    io.BackendPlatformName = "imgui_impl_x11";

    // Store display and window handles
    data->display = display;
    data->window = window;

    // Set up clipboard atoms
    data->clipboard_atom = XInternAtom(display, "CLIPBOARD", False);
    data->targets_atom = XInternAtom(display, "TARGETS", False);
    data->text_atom = XInternAtom(display, "TEXT", False);
    data->utf8_atom = XInternAtom(display, "UTF8_STRING", False);

    // Set up cursors (basic implementation)
    data->cursor_default = XCreateFontCursor(display, XC_left_ptr);
    
    // Initialize key mapping
    ImGui_ImplX11_InitKeyMap();

    // Setup clipboard handlers
    io.SetClipboardTextFn = ImGui_ImplX11_SetClipboardText;
    io.GetClipboardTextFn = ImGui_ImplX11_GetClipboardText;

    return true;
}

void ImGui_ImplX11_Shutdown()
{
    ImGui_ImplX11_Data* data = g_ImplX11Data;
    IM_ASSERT(data != nullptr && "No platform backend to shutdown, or already shutdown?");
    ImGuiIO& io = ImGui::GetIO();

    // Clean up cursors
    if (data->cursor_default)
        XFreeCursor(data->display, data->cursor_default);

    // Set io.BackendPlatformName to NULL
    io.BackendPlatformName = nullptr;
    io.BackendPlatformUserData = nullptr;
    IM_DELETE(data);
    g_ImplX11Data = nullptr;
}

void ImGui_ImplX11_NewFrame()
{
    ImGui_ImplX11_Data* data = g_ImplX11Data;
    IM_ASSERT(data != nullptr && "Did you call ImGui_ImplX11_Init()?");
    ImGuiIO& io = ImGui::GetIO();

    // Setup display size (every frame for flexibility)
    int width, height;
    Window root;
    int x, y;
    unsigned int border_width, depth;
    XGetGeometry(data->display, data->window, &root, &x, &y, (unsigned int*)&width, (unsigned int*)&height, &border_width, &depth);
    io.DisplaySize = ImVec2((float)width, (float)height);

    // Setup time step
    XEvent dummy;
    data->time = XLastKnownRequestProcessed(data->display);
    if (data->last_time == 0)
        data->last_time = data->time;
    io.DeltaTime = (data->time - data->last_time) / 1000.0f;
    if (io.DeltaTime <= 0.0f)
        io.DeltaTime = 0.001f;
    data->last_time = data->time;

    // Process queued key/mouse events (simplified - would need more robust X11 event processing)
}

bool ImGui_ImplX11_ProcessEvent(void* event)
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplX11_Data* data = g_ImplX11Data;
    XEvent* xevent = (XEvent*)event;

    switch (xevent->type)
    {
    case KeyPress:
    case KeyRelease:
    {
        KeySym keysym = XkbKeycodeToKeysym(data->display, xevent->xkey.keycode, 0, 0);
        bool is_key_down = (xevent->type == KeyPress);
        
        if (g_KeysymMap.find(keysym) != g_KeysymMap.end())
            io.AddKeyEvent(g_KeysymMap[keysym], is_key_down);
            
        return true;
    }
    case ButtonPress:
    case ButtonRelease:
    {
        bool is_button_down = (xevent->type == ButtonPress);
        int button = xevent->xbutton.button;
        if (button == Button1) io.AddMouseButtonEvent(ImGuiMouseButton_Left, is_button_down);
        if (button == Button2) io.AddMouseButtonEvent(ImGuiMouseButton_Middle, is_button_down);
        if (button == Button3) io.AddMouseButtonEvent(ImGuiMouseButton_Right, is_button_down);
        return true;
    }
    case MotionNotify:
    {
        io.AddMousePosEvent((float)xevent->xmotion.x, (float)xevent->xmotion.y);
        return true;
    }
    }

    return false;
}

VkResult ImGui_ImplX11_CreateVkSurface(VkInstance instance, Display* display, Window window, const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface)
{
    VkXlibSurfaceCreateInfoKHR info = {};
    info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    info.dpy = display;
    info.window = window;
    return vkCreateXlibSurfaceKHR(instance, &info, allocator, surface);
} 

#endif // YAGE_PLATFORM_UNIX

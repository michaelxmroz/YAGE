// dear imgui: Platform Backend for X11
// This needs to be used along with a Renderer (e.g. Vulkan, OpenGL3)

// Implemented features:
//  [X] Platform: Clipboard support.
//  [X] Platform: Mouse support.
//  [X] Platform: Keyboard support.

#pragma once
#include "imgui.h"      // IMGUI_IMPL_API

#include "PlatformDefines.h"

#if YAGE_PLATFORM_UNIX

#include "volk.h"

#include <vulkan/vulkan_xlib.h>

typedef unsigned long Window;

IMGUI_IMPL_API bool     ImGui_ImplX11_Init(Display* display, Window window);
IMGUI_IMPL_API void     ImGui_ImplX11_Shutdown();
IMGUI_IMPL_API void     ImGui_ImplX11_NewFrame();
IMGUI_IMPL_API bool     ImGui_ImplX11_ProcessEvent(void* event);  // X11 event

// X11-specific function for creating a Vulkan surface
IMGUI_IMPL_API VkResult ImGui_ImplX11_CreateVkSurface(VkInstance instance, Display* display, Window window, const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface); 

#endif // YAGE_PLATFORM_UNIX

// dear imgui: Platform Binding for Android native app
// This needs to be used along with the OpenGL 3 Renderer (Implopengl3)

// Implemented features:
//  [X] Platform: Keyboard support. Since 1.87 we are using the io.AddKeyEvent()
//  function. Pass ImGuiKey values to all key functions e.g.
//  ImGui::IsKeyPressed(ImGuiKey_Space). [Legacy AKEYCODE_* values will also be
//  supported unless IMGUI_DISABLE_OBSOLETE_KEYIO is set] [X] Platform: Mouse
//  support. Can discriminate Mouse/TouchScreen/Pen.
// Missing features:
//  [ ] Platform: Clipboard support.
//  [ ] Platform: Gamepad support. Enable with 'io.ConfigFlags |=
//  ImGuiConfigFlags_NavEnableGamepad'. [ ] Platform: Mouse cursor shape and
//  visibility. Disable with 'io.ConfigFlags |=
//  ImGuiConfigFlags_NoMouseCursorChange'. FIXME: Check if this is even possible
//  with Android.
// Important:
//  - Consider using SDL or GLFW backend on Android, which will be more
//  full-featured than this.
//  - FIXME: On-screen keyboard currently needs to be enabled by the application
//  (see examples/ and issue #3446)
//  - FIXME: Unicode character inputs needs to be passed by Dear ImGui by the
//  application (see examples/ and issue #3446)

// You can use unmodified Impl* files in your project. See examples/
// folder for examples of using this. Prefer including the entire imgui/
// repository into your project (either as a copy or as a submodule), and only
// build the backends you need. If you are new to Dear ImGui, read documentation
// from the docs/ folder + read the top of imgui.cpp. Read online:
// https://github.com/ocornut/imgui/tree/master/docs

#pragma once
#include "ImGui/ImGui.h" // IMGUI_IMPL_API
#ifndef IMGUI_DISABLE

struct ANativeWindow;
struct AInputEvent;

IMGUI_IMPL_API bool ImGui_ImplAndroid_Init(ANativeWindow *window);
IMGUI_IMPL_API int32_t
ImGui_ImplAndroid_HandleInputEvent(AInputEvent *input_event);
IMGUI_IMPL_API void ImGui_ImplAndroid_Shutdown();
IMGUI_IMPL_API void ImGui_ImplAndroid_NewFrame();

#endif // #ifndef IMGUI_DISABLE
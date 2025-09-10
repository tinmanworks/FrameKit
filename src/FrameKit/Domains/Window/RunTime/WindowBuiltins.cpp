#include "FrameKit/Window/Window.h"
#include "FrameKit/Window/WindowBuiltins.h"

namespace FrameKit {

#if FK_WINDOW_BACKEND_WIN32
extern "C" bool FrameKit_RegisterBackend_Win32();
#endif
#if FK_WINDOW_BACKEND_GLFW
extern "C" bool FrameKit_RegisterBackend_GLFW();
#endif
#if FK_WINDOW_BACKEND_COCOA
extern "C" bool FrameKit_RegisterBackend_Cocoa();
#endif

void RegisterBuiltInWindowBackends() {
#if FK_WINDOW_BACKEND_WIN32
    (void)FrameKit_RegisterBackend_Win32();
#endif
#if FK_WINDOW_BACKEND_GLFW
    (void)FrameKit_RegisterBackend_GLFW();
#endif
#if FK_WINDOW_BACKEND_COCOA
    (void)FrameKit_RegisterBackend_Cocoa();
#endif
}

} // namespace FrameKit

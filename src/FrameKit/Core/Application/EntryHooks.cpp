#include "FrameKit/Application/EntryHooks.h"

namespace FrameKit {

    static PreMainHookFn g_PreMainHook = nullptr;

    void SetPreMainHook(PreMainHookFn fn) {
        g_PreMainHook = fn;
    }

    PreMainHookFn GetPreMainHook() {
        return g_PreMainHook;
    }

} // namespace FrameKit

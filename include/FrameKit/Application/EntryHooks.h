#pragma once
#include <cstdint>

namespace FrameKit {

    using PreMainHookFn = void(*)(int argc, char** argv);

    // Thread-safety not really needed (runs before main work), but keep it simple.
    void SetPreMainHook(PreMainHookFn fn);
    PreMainHookFn GetPreMainHook();

} // namespace FrameKit

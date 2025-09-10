#include "FrameKit/Platform/Window.h"
#include "GlfwWindow.h"
#include <GLFW/glfw3.h>
#include <cassert>

namespace FrameKit {
static int s_glfwRef = 0;

static void glfwInitRef() {
    if (s_glfwRef++ == 0) { glfwInit(); }
}
static void glfwTermRef() {
    if (--s_glfwRef == 0) { glfwTerminate(); }
}

GlfwWindow::GlfwWindow(const WindowDesc& d) {
    glfwInitRef();
    glfwWindowHint(GLFW_VISIBLE, d.visible ? GLFW_TRUE : GLFW_FALSE);
#if defined(__APPLE__)
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, d.highDPI ? GLFW_TRUE : GLFW_FALSE);
#endif
    glfwWindowHint(GLFW_RESIZABLE, d.resizable ? GLFW_TRUE : GLFW_FALSE);
    m_w = glfwCreateWindow((int)d.width, (int)d.height, d.title.c_str(), nullptr, nullptr);
    assert(m_w);
    glfwGetWindowSize(m_w, (int*)&m_wd, (int*)&m_hd);
    float sx = 1.0f, sy = 1.0f;
    glfwGetWindowContentScale(m_w, &sx, &sy);
    m_sx = sx; m_sy = sy;
    setVSync(d.vsync);

    glfwSetWindowUserPointer(m_w, this);
    glfwSetKeyCallback(m_w, [](GLFWwindow* w, int key, int sc, int action, int mods){
        auto* self = (GlfwWindow*)glfwGetWindowUserPointer(w);
        if (self->onKey) self->onKey(KeyEvent{key, sc, action, mods});
    });
    glfwSetMouseButtonCallback(m_w, [](GLFWwindow* w, int button, int action, int mods){
        auto* self = (GlfwWindow*)glfwGetWindowUserPointer(w);
        if (self->onMouseBtn) self->onMouseBtn(MouseBtn{button, action, mods});
    });
    glfwSetCursorPosCallback(m_w, [](GLFWwindow* w, double x, double y){
        auto* self = (GlfwWindow*)glfwGetWindowUserPointer(w);
        if (self->onMouseMove) self->onMouseMove(MouseMove{x, y});
    });
    glfwSetScrollCallback(m_w, [](GLFWwindow* w, double dx, double dy){
        auto* self = (GlfwWindow*)glfwGetWindowUserPointer(w);
        if (self->onMouseWheel) self->onMouseWheel(MouseWheel{dx, dy});
    });
    glfwSetWindowSizeCallback(m_w, [](GLFWwindow* w, int vw, int vh){
        auto* self = (GlfwWindow*)glfwGetWindowUserPointer(w);
        self->m_wd = (uint32_t)vw; self->m_hd = (uint32_t)vh;
        if (self->onResize) self->onResize(Resize{vw, vh});
    });
    glfwSetWindowCloseCallback(m_w, [](GLFWwindow* w){
        auto* self = (GlfwWindow*)glfwGetWindowUserPointer(w);
        if (self->onCloseReq) self->onCloseReq(CloseReq{});
    });

    if (d.visible) glfwShowWindow(m_w);
}

GlfwWindow::~GlfwWindow() {
    if (m_w) { glfwDestroyWindow(m_w); m_w = nullptr; }
    glfwTermRef();
}

void GlfwWindow::poll()                { glfwPollEvents(); }
bool GlfwWindow::shouldClose() const   { return glfwWindowShouldClose(m_w); }
void GlfwWindow::requestClose()        { glfwSetWindowShouldClose(m_w, GLFW_TRUE); }
void* GlfwWindow::nativeHandle() const { return m_w; }
void* GlfwWindow::nativeDisplay() const{ return nullptr; }
uint32_t GlfwWindow::width() const     { return m_wd; }
uint32_t GlfwWindow::height() const    { return m_hd; }
float GlfwWindow::contentScaleX() const{ return m_sx; }
float GlfwWindow::contentScaleY() const{ return m_sy; }

void GlfwWindow::setTitle(const std::string& t) { glfwSetWindowTitle(m_w, t.c_str()); }
void GlfwWindow::setVSync(bool enabled) {
    m_vsync = enabled;
    // swap interval is usually set on a current GL context; leave as a hint
}
bool  GlfwWindow::getVSync() const { return m_vsync; }

void GlfwWindow::setCursorMode(CursorMode m) {
    int mode = GLFW_CURSOR_NORMAL;
    if (m == CursorMode::Hidden) mode = GLFW_CURSOR_HIDDEN;
    if (m == CursorMode::Locked) mode = GLFW_CURSOR_DISABLED;
    glfwSetInputMode(m_w, GLFW_CURSOR, mode);
}

// static registration (priority 100)
static std::unique_ptr<IWindow> CreateGLFW(const WindowDesc& d) {
    return std::unique_ptr<IWindow>(new GlfwWindow(d));
}
struct Registrar {
    Registrar(){ RegisterWindowBackend(WindowBackend::GLFW, "GLFW", &CreateGLFW, 100); }
} _rg;

} // namespace FrameKit

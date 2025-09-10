// =============================================================================
// Project      : FrameKit
// File         : src/FrameKit/Domains/Window/Backends/Cocoa/CocoaWindow.mm
// Author       : George Gil
// Created      : 2025-09-10
// Updated      : 2025-09-10
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  : Cocoa (macOS) window backend implementation
// =============================================================================

#import <Cocoa/Cocoa.h>
#include "FrameKit/Window/Window.h"
#include "CocoaWindow.h"

namespace FrameKit {
CocoaWindow::CocoaWindow(const WindowDesc& d) {
    [NSApplication sharedApplication];
    m_app = NSApp;
    [m_app setActivationPolicy:NSApplicationActivationPolicyRegular];

    NSRect rect = NSMakeRect(0,0,d.width,d.height);
    m_win = [[NSWindow alloc] initWithContentRect:rect
                                        styleMask:(NSWindowStyleMaskTitled|NSWindowStyleMaskClosable|NSWindowStyleMaskResizable)
                                          backing:NSBackingStoreBuffered
                                            defer:NO];
    [m_win setTitle:[NSString stringWithUTF8String:d.title.c_str()]];
    if (d.visible) [m_win makeKeyAndOrderFront:nil];
    [m_app activateIgnoringOtherApps:YES];

    NSScreen* s = [m_win screen];
    CGFloat scale = [s backingScaleFactor];
    m_sx = m_sy = (float)scale;
    m_w = d.width; m_h = d.height;
}

CocoaWindow::~CocoaWindow() {
    [m_win close];
}

void CocoaWindow::poll() {
    NSEvent* event;
    while ((event = [m_app nextEventMatchingMask:NSEventMaskAny untilDate:[NSDate dateWithTimeIntervalSinceNow:0] inMode:NSDefaultRunLoopMode dequeue:YES])) {
        [m_app sendEvent:event];
    }
}
bool CocoaWindow::shouldClose() const { return m_close; }
void CocoaWindow::requestClose(){ m_close = true; }

void* CocoaWindow::nativeHandle() const { return (void*)m_win; }
uint32_t CocoaWindow::width() const { return m_w; }
uint32_t CocoaWindow::height() const { return m_h; }
float CocoaWindow::contentScaleX() const { return m_sx; }
float CocoaWindow::contentScaleY() const { return m_sy; }

void CocoaWindow::setTitle(const std::string& t) {
    [m_win setTitle:[NSString stringWithUTF8String:t.c_str()]];
}

void CocoaWindow::setCursorMode(CursorMode m) {
    if (m == CursorMode::Hidden) [NSCursor hide];
    else [NSCursor unhide];
}

// static registration
static std::unique_ptr<IWindow> CreateCocoa(const WindowDesc& d) {
    return std::unique_ptr<IWindow>(new CocoaWindow(d));
}
struct Registrar {
    Registrar(){ RegisterWindowBackend(WindowBackend::Cocoa, "Cocoa", &CreateCocoa, 100); }
} _rg;

} // namespace FrameKit

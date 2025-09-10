#[[
===================================== FrameKit =========================================
  Project      : FrameKit
  File         : src/FrameKit/Domains/Window/Backends/CMakeLists.txt
  Author       : George Gil
  Created      : 2025-09-10
  Updated      : 2025-09-10
  License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
  Description  : Window backends: GLFW, Win32, Cocoa. Hybrid vendor/system GLFW.
========================================================================================
]]

# Expect from parent:
# - FRAMEKIT_WINDOW_DOMAIN_DIR  (src/FrameKit/Domains/Window)
# - FRAMEKIT_VENDOR_DIR         (defaults to ${CMAKE_SOURCE_DIR}/vendor)
# - FRAMEKIT_WINDOW_BACKEND_* toggles
# - FRAMEKIT_VENDOR_GLFW toggle

# ------------------------------ Resolve GLFW --------------------------------
set(_have_glfw FALSE)

if(FRAMEKIT_WINDOW_BACKEND_GLFW)
  if(FRAMEKIT_VENDOR_GLFW)
    # Build bundled GLFW if directory exists
    if(EXISTS "${FRAMEKIT_VENDOR_DIR}/glfw/CMakeLists.txt")
      add_subdirectory("${FRAMEKIT_VENDOR_DIR}/glfw"
                       "${CMAKE_BINARY_DIR}/vendor/glfw"
                       EXCLUDE_FROM_ALL)
      if(TARGET glfw)
        add_library(FrameKitDeps::GLFW INTERFACE IMPORTED)
        target_link_libraries(FrameKitDeps::GLFW INTERFACE glfw)
        set(_have_glfw TRUE)
      elseif(TARGET glfw3)
        add_library(FrameKitDeps::GLFW INTERFACE IMPORTED)
        target_link_libraries(FrameKitDeps::GLFW INTERFACE glfw3)
        set(_have_glfw TRUE)
      endif()
    else()
      message(WARNING "FRAMEKIT_VENDOR_GLFW=ON but ${FRAMEKIT_VENDOR_DIR}/glfw not found")
    endif()
  endif()

  if(NOT _have_glfw)
    # Try config package first
    find_package(glfw3 CONFIG QUIET)
    if(TARGET glfw OR TARGET glfw3)
      add_library(FrameKitDeps::GLFW INTERFACE IMPORTED)
      if(TARGET glfw)
        target_link_libraries(FrameKitDeps::GLFW INTERFACE glfw)
      else()
        target_link_libraries(FrameKitDeps::GLFW INTERFACE glfw3)
      endif()
      set(_have_glfw TRUE)
    else()
      # Fallback to pkg-config
      find_package(PkgConfig QUIET)
      if(PkgConfig_FOUND)
        pkg_check_modules(GLFW3 IMPORTED_TARGET glfw3)
        if(GLFW3_FOUND)
          add_library(FrameKitDeps::GLFW INTERFACE IMPORTED)
          target_link_libraries(FrameKitDeps::GLFW INTERFACE PkgConfig::GLFW3)
          set(_have_glfw TRUE)
        endif()
      endif()
    endif()
  endif()
endif()

# ----------------------------- GLFW backend ---------------------------------
if(FRAMEKIT_WINDOW_BACKEND_GLFW AND _have_glfw)
  framekit_add_backend(
    DOMAIN     Window
    TARGET     FrameKit.Window.GLFW
    COMPONENT  Window.GLFW
    SOURCES    ${FRAMEKIT_WINDOW_DOMAIN_DIR}/Backends/GLFW/GlfwWindow.cpp
    HEADERS    ${FRAMEKIT_WINDOW_DOMAIN_DIR}/Backends/GLFW/GlfwWindow.h
    PUBLIC_DEFINES FK_WINDOW_BACKEND_GLFW=1)
  target_link_libraries(FrameKit.Window.GLFW PRIVATE FrameKitDeps::GLFW)
elseif(FRAMEKIT_WINDOW_BACKEND_GLFW)
  message(WARNING "GLFW backend enabled but GLFW was not found. Skipping FrameKit.Window.GLFW.")
endif()

# ----------------------------- Win32 backend --------------------------------
if(FRAMEKIT_WINDOW_BACKEND_WIN32 AND WIN32)
  framekit_add_backend(
    DOMAIN     Window
    TARGET     FrameKit.Window.Win32
    COMPONENT  Window.Win32
    SOURCES    ${FRAMEKIT_WINDOW_DOMAIN_DIR}/Backends/Win32/Win32Window.cpp
    HEADERS    ${FRAMEKIT_WINDOW_DOMAIN_DIR}/Backends/Win32/Win32Window.h
    PUBLIC_DEFINES FK_WINDOW_BACKEND_WIN32=1)
endif()

# ----------------------------- Cocoa backend --------------------------------
if(FRAMEKIT_WINDOW_BACKEND_COCOA AND APPLE)
  framekit_add_backend(
    DOMAIN     Window
    TARGET     FrameKit.Window.Cocoa
    COMPONENT  Window.Cocoa
    SOURCES    ${FRAMEKIT_WINDOW_DOMAIN_DIR}/Backends/Cocoa/CocoaWindow.mm
    HEADERS    ${FRAMEKIT_WINDOW_DOMAIN_DIR}/Backends/Cocoa/CocoaWindow.h
    PUBLIC_DEFINES FK_WINDOW_BACKEND_COCOA=1)
  set_target_properties(FrameKit.Window.Cocoa PROPERTIES LINKER_LANGUAGE CXX)
  target_link_libraries(FrameKit.Window.Cocoa PUBLIC "-framework Cocoa")
endif()
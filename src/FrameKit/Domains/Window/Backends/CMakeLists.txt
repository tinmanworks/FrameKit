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

message(STATUS "============================= FrameKit Window Backends =================================")

# ------------------------------ Resolve GLFW --------------------------------
set(_have_glfw FALSE)
message(STATUS "Resolving GLFW dependency for FrameKit Window backend...")
if(FRAMEKIT_WINDOW_BACKEND_GLFW)
  if(TARGET FrameKitDeps.GLFW)
    message(STATUS "FRAMEKIT_WINDOW_BACKEND_GLFW=ON and found FrameKitDeps::GLFW target.")
    set(_have_glfw TRUE)

  else()
    message(STATUS "FRAMEKIT_WINDOW_BACKEND_GLFW=ON but FrameKitDeps::GLFW target not found.")

    # Try CMake package first (config or module)
    find_package(glfw3 QUIET CONFIG)
    if(TARGET glfw OR TARGET glfw3 OR TARGET glfw::glfw)
      message(STATUS "Found GLFW via find_package(glfw3).")

      add_library(FrameKitDeps.GLFW INTERFACE)  # not IMPORTED
      if(TARGET glfw)
        message(STATUS "Using target 'glfw'.")
        target_link_libraries(FrameKitDeps.GLFW INTERFACE glfw)
      elseif(TARGET glfw3)
        message(STATUS "Using target 'glfw3'.")
        target_link_libraries(FrameKitDeps.GLFW INTERFACE glfw3)
      else() # glfw::glfw
        message(STATUS "Using target 'glfw::glfw'.")
        target_link_libraries(FrameKitDeps.GLFW INTERFACE glfw::glfw)
      endif()
      set(_have_glfw TRUE)

    else()
      message(STATUS "Could not find GLFW via find_package(glfw3). Trying pkg-config.")

      find_package(PkgConfig QUIET)
      if(PkgConfig_FOUND)
        pkg_check_modules(GLFW3 IMPORTED_TARGET glfw3)
        if(GLFW3_FOUND)
          add_library(FrameKitDeps.GLFW INTERFACE)
          target_link_libraries(FrameKitDeps.GLFW INTERFACE PkgConfig::GLFW3)
          set(_have_glfw TRUE)
        endif()
      endif()
    endif()
  endif()

  if(NOT _have_glfw)
    message(FATAL_ERROR "FRAMEKIT_WINDOW_BACKEND_GLFW=ON but GLFW not found "
                        "(expected existing target FrameKitDeps::GLFW, a glfw/glfw3/glfw::glfw CMake package, or pkg-config 'glfw3').")
  endif()
endif()
message(STATUS "GLFW dependency resolved: ${_have_glfw}")

# ----------------------------- GLFW backend ---------------------------------
if(FRAMEKIT_WINDOW_BACKEND_GLFW AND _have_glfw)
    framekit_add_backend(
        DOMAIN     Window
        TARGET     FrameKit.Window.GLFW
        COMPONENT  Window.GLFW
        SOURCES    ${FRAMEKIT_WINDOW_DOMAIN_DIR}/Backends/GLFW/GlfwWindow.cpp
        HEADERS    ${FRAMEKIT_WINDOW_DOMAIN_DIR}/Backends/GLFW/GlfwWindow.h
        PUBLIC_LIBS FrameKitDeps.GLFW
        PUBLIC_DEFINES FK_WINDOW_BACKEND_GLFW=1)
elseif(FRAMEKIT_WINDOW_BACKEND_GLFW)
    message(STATUS "GLFW backend enabled but GLFW was not found. Skipping FrameKit.Window.GLFW.")
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

message(STATUS "========================================================================================")

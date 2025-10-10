#[[
===================================== FrameKit =========================================
  Project      : FrameKit
  File         : src/FrameKit/Domains/Window/CMakeLists.txt
  Author       : George Gil
  Created      : 2025-09-10
  Updated      : 2025-09-10
  License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
  Description  :
          CMakeLists.txt for FrameKit Window backend. 
          Includes GLFW, Win32, Cocoa backends.
========================================================================================
]]

message(STATUS "-------------------------------- FrameKit Window Domain --------------------------------")

# Window domain API (header-only) and runtime (registry + loader)
set(FRAMEKIT_WINDOW_DOMAIN_DIR "${FRAMEKIT_SRC_DIR}/FrameKit/Domains/Window")
include("${FRAMEKIT_ROOT_DIR}/cmake/FrameKitAddBackend.cmake")

message(STATUS "FrameKit Window Domain Dir: ${FRAMEKIT_WINDOW_DOMAIN_DIR}")
message(STATUS "")

message(STATUS "Setting up Window API")
# -------- API (INTERFACE) --------
add_library(FrameKit.WindowAPI INTERFACE)
target_include_directories(FrameKit.WindowAPI INTERFACE
  $<BUILD_INTERFACE:${FRAMEKIT_INCLUDE_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
add_library(FrameKit::WindowAPI ALIAS FrameKit.WindowAPI)
# Show headers in IDEs
file(GLOB_RECURSE FRAMEKIT_WINDOW_API_HEADERS
  "${FRAMEKIT_INCLUDE_DIR}/FrameKit/Window/*.h"
  "${FRAMEKIT_INCLUDE_DIR}/FrameKit/Window/*.hpp")
target_sources(FrameKit.WindowAPI INTERFACE
  FILE_SET window_api_headers TYPE HEADERS
  BASE_DIRS "${FRAMEKIT_INCLUDE_DIR}"
  FILES ${FRAMEKIT_WINDOW_API_HEADERS})
message(STATUS "FrameKit Window API Setup Complete")

message(STATUS "Setting up Window Runtime")
# -------- Runtime (registry + loader) --------
add_library(FrameKit.WindowRuntime STATIC
  "${FRAMEKIT_WINDOW_DOMAIN_DIR}/RunTime/WindowRegistry.cpp"
  "${FRAMEKIT_WINDOW_DOMAIN_DIR}/RunTime/WindowBackendRegistry.cpp"
  "${FRAMEKIT_WINDOW_DOMAIN_DIR}/RunTime/WindowPluginLoader.cpp"
  "${FRAMEKIT_WINDOW_DOMAIN_DIR}/RunTime/WindowBuiltins.cpp")
add_library(FrameKit::WindowRuntime ALIAS FrameKit.WindowRuntime)

target_link_libraries(FrameKit.WindowRuntime PUBLIC FrameKit.WindowAPI)
target_link_libraries(FrameKit PUBLIC FrameKit.WindowRuntime)
target_include_directories(FrameKit.WindowRuntime
  PUBLIC
    $<BUILD_INTERFACE:${FRAMEKIT_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# IDE folders and filters
set_target_properties(FrameKit.WindowRuntime PROPERTIES FOLDER "FrameKit")
# source_group(TREE "${FRAMEKIT_ROOT_DIR}" FILES
#   ${FRAMEKIT_WINDOW_API_HEADERS}
#   "${FRAMEKIT_WINDOW_DOMAIN_DIR}/Runtime/WindowRegistry.cpp"
#   "${FRAMEKIT_WINDOW_DOMAIN_DIR}/Runtime/WindowPluginLoader.cpp")

message(STATUS "FrameKit Window Runtime Setup Complete")

# -------- Backends --------
add_subdirectory(Backends)

# -------- Install --------
install(TARGETS
  FrameKit.WindowAPI
  EXPORT FrameKitTargets
  FILE_SET window_api_headers DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(TARGETS
  FrameKit.WindowRuntime
  EXPORT FrameKitTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/$<CONFIG>
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/$<CONFIG>
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/$<CONFIG>
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

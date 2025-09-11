#[[
==============================================================================
  Project      : FrameKit
  File         : CMakeLists.txt
  Author       : George Gil
  Created      : 2025-08-11
  Updated      : 2025-09-10
  License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
  Description  : 
          Root CMakeLists.txt file for FrameKit, a simple C++ framework for 
          building applications. Sets up project, language standards, includes
          helper modules, and configures FrameKit library and optional sandbox 
          application.
==============================================================================
]]

# ------------------------------- Pretty banner ------------------------------
message(STATUS "======================================= FrameKit =======================================")
message(STATUS "Configuring FrameKit...")
message(STATUS "========================================================================================")

cmake_minimum_required(VERSION 3.23)


# IDE organization
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake")  # ALL_BUILD, ZERO_CHECK, INSTALL


# ----------------------------------------------------------------------------
# ------------------------------ Build Options -------------------------------
# ----------------------------------------------------------------------------

# ---------------------------- General Options -------------------------------
option(FRAMEKIT_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)
option(BUILD_SANDBOX               "Build the sandbox app" OFF)

# -------------------------- Dependency Options ------------------------------
option(FRAMEKIT_VENDOR_GLFW "Build bundled GLFW instead of finding system GLFW" ON)

# -------------------------- Window Backend Options --------------------------
option(FRAMEKIT_WINDOW_BACKEND_GLFW  "Build GLFW backend"  ON)
option(FRAMEKIT_WINDOW_BACKEND_WIN32 "Build Win32 backend" ${WIN32})
option(FRAMEKIT_WINDOW_BACKEND_COCOA "Build Cocoa backend" ${APPLE})
option(BUILD_WINDOW_PLUGINS          "Build loadable window plugins" OFF)


# ----------------------------------------------------------------------------
# ------------------------------ Main Build ----------------------------------
# ----------------------------------------------------------------------------

# ------------------------------ Paths ---------------------------------------
set(FRAMEKIT_ROOT_DIR    "${CMAKE_CURRENT_LIST_DIR}")
set(FRAMEKIT_INCLUDE_DIR "${FRAMEKIT_ROOT_DIR}/include")
set(FRAMEKIT_SRC_DIR     "${FRAMEKIT_ROOT_DIR}/src")
set(FRAMEKIT_BUILD_DIR   "${CMAKE_CURRENT_BINARY_DIR}")
set(FRAMEKIT_VENDOR_DIR  "${FRAMEKIT_ROOT_DIR}/vendor" CACHE PATH "Vendor libs root")

# ---------------------------------- Project ---------------------------------
# Languages: enable ObjC++ only on macOS
if(APPLE)
  project(FrameKit VERSION 0.1.0 DESCRIPTION "FrameKit C++ framework" LANGUAGES C CXX OBJCXX)
else()
  project(FrameKit VERSION 0.1.0 DESCRIPTION "FrameKit C++ framework" LANGUAGES C CXX)
endif()

# --------------------------------- Toolchain ---------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(BUILD_SHARED_LIBS OFF)

# Output dirs per configuration
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_OUTPUT_DIRECTORY}/lib/$<CONFIG>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_OUTPUT_DIRECTORY}/lib/$<CONFIG>")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_OUTPUT_DIRECTORY}/bin/$<CONFIG>")

# ------------------------------ CMake Modules --------------------------------
include(GNUInstallDirs)

# ------------------------------ Subdirectories -------------------------------
add_subdirectory(src/FrameKit)
if(BUILD_SANDBOX)
  add_subdirectory(SandBox)
endif()

# ------------------------------ Package Config -------------------------------
include(CMakePackageConfigHelpers)
# Config file that sets up imported target namespace
set(FRAMEKIT_CONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/FrameKit")

configure_package_config_file(
  "${CMAKE_CURRENT_LIST_DIR}/cmake/FrameKitConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/FrameKitConfig.cmake"
  INSTALL_DESTINATION "${FRAMEKIT_CONFIG_INSTALL_DIR}")

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/FrameKitConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/FrameKitConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/FrameKitConfigVersion.cmake"
  DESTINATION "${FRAMEKIT_CONFIG_INSTALL_DIR}")
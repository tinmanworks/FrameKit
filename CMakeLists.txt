#[[
==============================================================================
  Project      : FrameKit
  File         : CMakeLists.txt
  Author       : George Gil
  Created      : 2025-08-11
  Updated      : 2025-09-11
  License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
  Description  : 
          Root CMakeLists.txt file for FrameKit, a simple C++ framework for 
          building applications. Sets up project, language standards, includes
          helper modules, and configures FrameKit library and optional sandbox 
          application.
==============================================================================
]]

# ------------------------------- Pretty banner ------------------------------
message(STATUS "====================================== FrameKit ========================================")
message(STATUS "Configuring FrameKit...")
message(STATUS "========================================================================================")
message(STATUS "CMake version: ${CMAKE_VERSION}")
message(STATUS "")
cmake_minimum_required(VERSION 3.23)


# IDE organization
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMake")  # ALL_BUILD, ZERO_CHECK, INSTALL

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
set(PROJECT_NAME "FrameKit")
set(PROJECT_DESCRIPTION "A simple C++ framework for building applications")
set(PROJECT_VERSION 0.1.0)

# Languages: enable ObjC++ only on macOS
if(APPLE)
  project(${PROJECT_NAME} VERSION ${PROJECT_VERSION} DESCRIPTION "${PROJECT_DESCRIPTION}" LANGUAGES C CXX OBJCXX)
else()
  project(${PROJECT_NAME} VERSION ${PROJECT_VERSION} DESCRIPTION "${PROJECT_DESCRIPTION}" LANGUAGES C CXX)
endif()

# ----------------------------------------------------------------------------
# ------------------------------ Build Options -------------------------------
# ----------------------------------------------------------------------------

set(FRAMEKIT_IS_TOP_LEVEL ${PROJECT_IS_TOP_LEVEL})

# ---------------------------- General Options -------------------------------
option(FRAMEKIT_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)
option(BUILD_EXAMPLES              "Build the examples" ${FRAMEKIT_IS_TOP_LEVEL})

# -------------------------- Dependency Options ------------------------------
option(FRAMEKIT_VENDOR_GLFW "Build bundled GLFW instead of finding system GLFW" ${FRAMEKIT_IS_TOP_LEVEL})

# -------------------------- Window Backend Options --------------------------
option(FRAMEKIT_WINDOW_BACKEND_GLFW  "Build GLFW backend"  ON)
option(FRAMEKIT_WINDOW_BACKEND_WIN32 "Build Win32 backend" ${WIN32})
option(FRAMEKIT_WINDOW_BACKEND_COCOA "Build Cocoa backend" ${APPLE})
option(BUILD_WINDOW_PLUGINS          "Build loadable window plugins" OFF)

# ----------------------------- MediaKit options ------------------------------
option(FK_MEDIA_ENABLE_STB    "Enable stb_image backend" ON)
option(FK_MEDIA_ENABLE_FFMPEG "Enable FFmpeg backends" ON)

# --------------------------------- Toolchain ---------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(BUILD_SHARED_LIBS OFF)

# Output dirs per configuration
if(FRAMEKIT_IS_TOP_LEVEL)
  set(_fk_out "${CMAKE_CURRENT_BINARY_DIR}")
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${_fk_out}/lib/$<CONFIG>")
  set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${_fk_out}/lib/$<CONFIG>")
  set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${_fk_out}/bin/$<CONFIG>")
endif()

# ------------------------------ CMake Modules --------------------------------
include(GNUInstallDirs)

# ------------------------------ Subdirectories -------------------------------
add_subdirectory(vendor)
add_subdirectory(src/FrameKit)

message(STATUS "============================ FrameKit Configuration Ended ==============================")
message(STATUS "========================================================================================")
message(STATUS "")
message(STATUS "")

if(BUILD_EXAMPLES)
  message(STATUS "")
  message(STATUS "=============================== Configuring Example Projects =============================")
  add_subdirectory(Examples)
endif()

# MSVC: build Debug+Release
if(MSVC)
  set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
endif()

# ------------------------------ Package Config -------------------------------
if(FRAMEKIT_IS_TOP_LEVEL)
  include(CMakePackageConfigHelpers)
  # Config file that sets up imported target namespace
  set(FRAMEKIT_CONFIG_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/FrameKit")

  install(EXPORT FrameKitTargets
    NAMESPACE FrameKit::
    DESTINATION "${FRAMEKIT_CONFIG_INSTALL_DIR}")

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
endif()
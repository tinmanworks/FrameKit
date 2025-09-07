#[[
===============================================================================
  Project      : FrameKit
  File         : CMakeLists.txt
  Author       : George Gil
  Created      : 2025-08-11
  Updated      : 2025-09-06
  License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
  Description  : 
          Root CMakeLists.txt file for FrameKit, a simple C++ framework for 
          building applications. Sets up project, language standards, includes
          helper modules, and configures FrameKit library and optional sandbox 
          application.
===============================================================================
]]

# --- Pretty banner ---


cmake_minimum_required(VERSION 3.23)

# ---------------------------------- Project ----------------------------------
project(FrameKit
  VERSION 0.1.0
  DESCRIPTION "FrameKit C++ framework"
  LANGUAGES C CXX)

# ---------------------------------- Options ----------------------------------
option(BUILD_SANDBOX "Build the sandbox app" OFF)
option(FRAMEKIT_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)

# --------------------------------- Toolchain ---------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(BUILD_SHARED_LIBS OFF)               # static only

# Output dirs (build tree)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Paths
set(FRAMEKIT_ROOT_DIR    "${CMAKE_CURRENT_LIST_DIR}")
set(FRAMEKIT_INCLUDE_DIR "${FRAMEKIT_ROOT_DIR}/include")
set(FRAMEKIT_SRC_DIR     "${FRAMEKIT_ROOT_DIR}/src/FrameKit")

# --------------------------------- Sources -----------------------------------
# Public headers = everything under include/ is installable
file(GLOB_RECURSE FRAMEKIT_PUBLIC_HEADERS CONFIGURE_DEPENDS
  "${FRAMEKIT_INCLUDE_DIR}/*.h" "${FRAMEKIT_INCLUDE_DIR}/*.hpp" "${FRAMEKIT_INCLUDE_DIR}/*.inl")

# Private sources/headers under src/FrameKit
file(GLOB_RECURSE FRAMEKIT_PRIVATE_SOURCES CONFIGURE_DEPENDS
  "${FRAMEKIT_SRC_DIR}/*.cpp")
file(GLOB_RECURSE FRAMEKIT_PRIVATE_HEADERS CONFIGURE_DEPENDS
  "${FRAMEKIT_SRC_DIR}/*.h" "${FRAMEKIT_SRC_DIR}/*.hpp" "${FRAMEKIT_SRC_DIR}/*.inl")

add_library(FrameKit STATIC
  ${FRAMEKIT_PRIVATE_SOURCES}
  ${FRAMEKIT_PUBLIC_HEADERS}
  ${FRAMEKIT_PRIVATE_HEADERS})

add_library(FrameKit::FrameKit ALIAS FrameKit)

target_include_directories(FrameKit
  PUBLIC
    $<BUILD_INTERFACE:${FRAMEKIT_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE "${FRAMEKIT_ROOT_DIR}/src")     # for private headers like Core/*.h

target_compile_features(FrameKit PUBLIC cxx_std_20)
set_target_properties(FrameKit PROPERTIES POSITION_INDEPENDENT_CODE ON)

# -------------------------------- Warnings -----------------------------------
if(MSVC)
  target_compile_options(FrameKit PRIVATE /W4 /permissive- /Zc:preprocessor)
  if(FRAMEKIT_WARNINGS_AS_ERRORS)
    target_compile_options(FrameKit PRIVATE /WX)
  endif()
else()
  target_compile_options(FrameKit PRIVATE -Wall -Wextra -Wpedantic)
  if(FRAMEKIT_WARNINGS_AS_ERRORS)
    target_compile_options(FrameKit PRIVATE -Werror)
  endif()
endif()

# Small, consistent defines for PlatformDetection.h if you want them
if(WIN32)
  target_compile_definitions(FrameKit PUBLIC CMAKE_PLATFORM_WIN=1)
elseif(UNIX AND NOT APPLE)
  target_compile_definitions(FrameKit PUBLIC CMAKE_PLATFORM_LINUX=1)
endif()
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  target_compile_definitions(FrameKit PUBLIC CMAKE_PLATFORM_64_BIT=1)
else()
  target_compile_definitions(FrameKit PUBLIC CMAKE_PLATFORM_32_BIT=1)
endif()

# --------------------------------- Vendor ------------------------------------
if(EXISTS "${FRAMEKIT_ROOT_DIR}/vendor/CMakeLists.txt")
  add_subdirectory("${FRAMEKIT_ROOT_DIR}/vendor")
endif()

# --------------------------------- Sandbox -----------------------------------
if(BUILD_SANDBOX AND EXISTS "${FRAMEKIT_ROOT_DIR}/sandbox/CMakeLists.txt")
  add_subdirectory("${FRAMEKIT_ROOT_DIR}/sandbox")
endif()

# --------------------------------- Install -----------------------------------
include(GNUInstallDirs)
install(TARGETS FrameKit
  EXPORT FrameKitTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(DIRECTORY "${FRAMEKIT_INCLUDE_DIR}/"
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

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

install(EXPORT FrameKitTargets
  NAMESPACE FrameKit::
  DESTINATION "${FRAMEKIT_CONFIG_INSTALL_DIR}")
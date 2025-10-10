#[[
===================================== FrameKit =========================================
  Project      : FrameKit
  File         : src/FrameKit/CMakeLists.txt
  Author       : George Gil
  Created      : 2025-09-10
  Updated      : 2025-09-11
  License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
  Description  : CMakeLists.txt for FrameKit core library and domains.
========================================================================================
]]

message(STATUS "===================================== FrameKit SDK =====================================")

cmake_minimum_required(VERSION 3.23)

# -------- Public headers (installed) --------
file(GLOB_RECURSE FRAMEKIT_PUBLIC_HEADERS CONFIGURE_DEPENDS
  "${FRAMEKIT_INCLUDE_DIR}/*.h" "${FRAMEKIT_INCLUDE_DIR}/*.hpp" "${FRAMEKIT_INCLUDE_DIR}/*.inl"
)

# -------- Core sources/headers only (no domains/backends) --------
file(GLOB_RECURSE FRAMEKIT_CORE_SOURCES  CONFIGURE_DEPENDS
  "${FRAMEKIT_SRC_DIR}/FrameKit/Core/*.cpp" "${FRAMEKIT_SRC_DIR}/FrameKit/Core/*.cxx"
)
file(GLOB_RECURSE FRAMEKIT_CORE_HEADERS  CONFIGURE_DEPENDS
  "${FRAMEKIT_SRC_DIR}/FrameKit/Core/*.h" "${FRAMEKIT_SRC_DIR}/FrameKit/Core/*.[hi]pp" "${FRAMEKIT_SRC_DIR}/FrameKit/Core/*.inl"
)

list(FILTER FRAMEKIT_CORE_SOURCES EXCLUDE REGEX ".*/FrameKit/Core/MediaKit/.*")
list(FILTER FRAMEKIT_CORE_HEADERS EXCLUDE REGEX ".*/FrameKit/Core/MediaKit/.*")

# MediaKit sources
set(FRAMEKIT_MEDIAKIT_SOURCES
  ${FRAMEKIT_SRC_DIR}/FrameKit/Core/MediaKit/FFmpeg/FFCommon.h
  ${FRAMEKIT_SRC_DIR}/FrameKit/Core/MediaKit/FFmpeg/FFCommon.cpp
  ${FRAMEKIT_SRC_DIR}/FrameKit/Core/MediaKit/FFmpeg/FFPlayer.h
  ${FRAMEKIT_SRC_DIR}/FrameKit/Core/MediaKit/FFmpeg/FFPlayer.cpp
  ${FRAMEKIT_SRC_DIR}/FrameKit/Core/MediaKit/FFmpeg/FFVideoReader.h
  ${FRAMEKIT_SRC_DIR}/FrameKit/Core/MediaKit/FFmpeg/FFVideoReader.cpp
  ${FRAMEKIT_SRC_DIR}/FrameKit/Core/MediaKit/ImageLoader_Stb.cpp
  ${FRAMEKIT_SRC_DIR}/FrameKit/Core/MediaKit/VideoReader_FFmpeg.cpp
  ${FRAMEKIT_SRC_DIR}/FrameKit/Core/MediaKit/Player_FFmpeg.cpp
  ${FRAMEKIT_SRC_DIR}/FrameKit/Core/MediaKit/MediaKit.cpp
)

# Filter by options
if(NOT FK_MEDIA_ENABLE_STB)
  list(REMOVE_ITEM FRAMEKIT_MEDIAKIT_SOURCES
       ${FRAMEKIT_SRC_DIR}/FrameKit/Core/MediaKit/ImageLoader_Stb.cpp)
endif()
if(NOT FK_MEDIA_ENABLE_FFMPEG)
  list(REMOVE_ITEM FRAMEKIT_MEDIAKIT_SOURCES
        ${FRAMEKIT_SRC_DIR}/FrameKit/Core/MediaKit/VideoReader_FFmpeg.cpp
        ${FRAMEKIT_SRC_DIR}/FrameKit/Core/MediaKit/Player_FFmpeg.cpp
        ${FRAMEKIT_SRC_DIR}/FrameKit/Core/MediaKit/FFmpeg/FFCommon.h
        ${FRAMEKIT_SRC_DIR}/FrameKit/Core/MediaKit/FFmpeg/FFCommon.cpp
        ${FRAMEKIT_SRC_DIR}/FrameKit/Core/MediaKit/FFmpeg/FFPlayer.h
        ${FRAMEKIT_SRC_DIR}/FrameKit/Core/MediaKit/FFmpeg/FFPlayer.cpp
        ${FRAMEKIT_SRC_DIR}/FrameKit/Core/MediaKit/FFmpeg/FFVideoReader.h
        ${FRAMEKIT_SRC_DIR}/FrameKit/Core/MediaKit/FFmpeg/FFVideoReader.cpp
  )
endif()

# IDE filters
source_group(TREE "${FRAMEKIT_ROOT_DIR}" FILES
  ${FRAMEKIT_CORE_SOURCES} ${FRAMEKIT_CORE_HEADERS}
  ${FRAMEKIT_PUBLIC_HEADERS} ${FRAMEKIT_MEDIAKIT_SOURCES})

# -------- Target: FrameKit (static) --------
add_library(FrameKit STATIC
  ${FRAMEKIT_CORE_HEADERS}
  ${FRAMEKIT_CORE_SOURCES}
  ${FRAMEKIT_PUBLIC_HEADERS})
add_library(FrameKit::FrameKit ALIAS FrameKit)
target_sources(FrameKit PRIVATE ${FRAMEKIT_MEDIAKIT_SOURCES})
set_target_properties(FrameKit PROPERTIES FOLDER "FrameKit" POSITION_INDEPENDENT_CODE ON)

target_include_directories(FrameKit
  PUBLIC
    $<BUILD_INTERFACE:${FRAMEKIT_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE "${FRAMEKIT_SRC_DIR}")

# stb_image: header-only, keep private
if(FK_MEDIA_ENABLE_STB)
  if(EXISTS "${FRAMEKIT_VENDOR_DIR}/stb_image")
    target_include_directories(FrameKit PRIVATE ${FRAMEKIT_VENDOR_DIR}/stb_image)
    target_compile_definitions(FrameKit PRIVATE FK_MEDIA_ENABLE_STB=1)
  else()
    message(WARNING "FK_MEDIA_ENABLE_STB=ON but ${FRAMEKIT_VENDOR_DIR}/stb_image not found.")
  endif()
endif()

message(STATUS "FFMpeg linkage: ${FK_MEDIA_ENABLE_FFMPEG}")

# ---- FFmpeg linkage (single block) ----
if(FK_MEDIA_ENABLE_FFMPEG)
  if(MSVC)
    # find_package(FFMPEG REQUIRED)             # via vcpkg toolchain
    # target_include_directories(FrameKit PUBLIC ${FFMPEG_INCLUDE_DIRS})
    # if(FFMPEG_LIBRARY_DIRS)
    #   target_link_directories(FrameKit PUBLIC ${FFMPEG_LIBRARY_DIRS})
    # endif()
    # target_link_libraries(FrameKit PUBLIC ${FFMPEG_LIBRARIES})
    find_package(FFMPEG REQUIRED)  # vcpkg
    # Do NOT export vcpkg include paths
    target_include_directories(FrameKit PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<INSTALL_INTERFACE:include>
    )
    target_link_libraries(FrameKit PUBLIC ${FFMPEG_LIBRARIES})
    target_include_directories(FrameKit PRIVATE ${FFMPEG_INCLUDE_DIRS})
  else()
    find_package(PkgConfig QUIET)
    if(PKG_CONFIG_FOUND)
        pkg_check_modules(AVFORMAT   REQUIRED IMPORTED_TARGET libavformat)
        pkg_check_modules(AVCODEC    REQUIRED IMPORTED_TARGET libavcodec)
        pkg_check_modules(AVUTIL     REQUIRED IMPORTED_TARGET libavutil)
        pkg_check_modules(SWSCALE    REQUIRED IMPORTED_TARGET libswscale)
        pkg_check_modules(SWRESAMPLE REQUIRED IMPORTED_TARGET libswresample)
        find_package(Threads REQUIRED)
        target_link_libraries(FrameKit
          PUBLIC
            PkgConfig::AVFORMAT
            PkgConfig::AVCODEC
            PkgConfig::AVUTIL
            PkgConfig::SWSCALE
            PkgConfig::SWRESAMPLE
          PRIVATE
            Threads::Threads m dl
        )
    else()
      message(WARNING "pkg-config not found; FFmpeg disabled on this platform.")
      set(FK_MEDIA_ENABLE_FFMPEG OFF)
    endif()
  endif()

  if(FK_MEDIA_ENABLE_FFMPEG)
    target_compile_definitions(FrameKit PRIVATE FK_MEDIA_ENABLE_FFMPEG=1)
  endif()
endif()

# Common compile options
target_compile_features(FrameKit PUBLIC cxx_std_20)

if(MSVC)
  target_compile_options(FrameKit PRIVATE /W4 /permissive- /Zc:preprocessor $<$<BOOL:${FRAMEKIT_WARNINGS_AS_ERRORS}>:/WX>)
else()
  target_compile_options(FrameKit PRIVATE -Wall -Wextra -Wpedantic $<$<BOOL:${FRAMEKIT_WARNINGS_AS_ERRORS}>:-Werror>)
endif()

# Platform defs
if(WIN32)
  target_compile_definitions(FrameKit PUBLIC CMAKE_PLATFORM_WIN=1)
elseif(APPLE)
  target_compile_definitions(FrameKit PUBLIC CMAKE_PLATFORM_MAC=1)
else()
  target_compile_definitions(FrameKit PUBLIC CMAKE_PLATFORM_LINUX=1)
endif()
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  target_compile_definitions(FrameKit PUBLIC CMAKE_PLATFORM_64_BIT=1)
endif()

# Profile flag per-config
target_compile_definitions(FrameKit PUBLIC
  $<$<CONFIG:Debug>:FK_PROFILE=1>
  $<$<NOT:$<CONFIG:Debug>>:FK_PROFILE=0>)

message(STATUS "FrameKit core library configured.")
message(STATUS "========================================================================================")

# -------- Domains (Window, Audio, Gfx, …) --------
add_subdirectory("${FRAMEKIT_SRC_DIR}/FrameKit/Domains")

# Install core + headers
if(FRAMEKIT_IS_TOP_LEVEL)
  include(GNUInstallDirs)
  install(TARGETS FrameKit
    EXPORT FrameKitTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}/$<CONFIG>
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/$<CONFIG>
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/$<CONFIG>
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
  install(DIRECTORY "${FRAMEKIT_INCLUDE_DIR}/" DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
  # Export everything (core + domains)
  # install(EXPORT FrameKitTargets
  #   NAMESPACE FrameKit::
  #   DESTINATION ${CMAKE_INSTALL_PREFIX}/cmake/FrameKit)
endif()
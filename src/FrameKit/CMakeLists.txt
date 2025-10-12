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

# IDE filters
source_group(TREE "${FRAMEKIT_ROOT_DIR}" FILES
  ${FRAMEKIT_CORE_SOURCES} ${FRAMEKIT_CORE_HEADERS}
  ${FRAMEKIT_PUBLIC_HEADERS})

# -------- Target: FrameKit (static) --------
add_library(FrameKit STATIC
  ${FRAMEKIT_CORE_HEADERS}
  ${FRAMEKIT_CORE_SOURCES}
  ${FRAMEKIT_PUBLIC_HEADERS})
add_library(FrameKit::FrameKit ALIAS FrameKit)
set_target_properties(FrameKit PROPERTIES FOLDER "FrameKit" POSITION_INDEPENDENT_CODE ON)

target_include_directories(FrameKit
  PUBLIC
    $<BUILD_INTERFACE:${FRAMEKIT_INCLUDE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
  PRIVATE "${FRAMEKIT_SRC_DIR}")

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

get_target_property(_ill FrameKit INTERFACE_LINK_LIBRARIES)
get_target_property(_iinc FrameKit INTERFACE_INCLUDE_DIRECTORIES)
message(STATUS "INTERFACE_LINK_LIBRARIES='${_ill}'")
message(STATUS "INTERFACE_INCLUDE_DIRECTORIES='${_iinc}'")

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
#pragma once
/*
===============================================================================
 Project      : FrameKit
 File         : SharedMemoryVersion.h
 Author       : George Gil
 Created      : 2025-08-15
 Description  : Defines the current FrameKit SharedMemory system version.
                Use FK_VERSION_* macros for compile-time version checks or
                display in application UIs.
 License      : Proprietary (c) 2025 George Gil
===============================================================================
*/

#define FK_SHM_VERSION_MAJOR 2
#define FK_SHM_VERSION_MINOR 0
#define FK_SHM_VERSION_PATCH 0

// Optional: Derived string versions
#define FK_SHM_VERSION_STRING   "2.0.0"
#define FK_SHM_VERSION_WSTRING  L"2.0.0"
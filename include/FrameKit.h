/*
         _____                         _  ___ _   
        |  ___| __ __ _ _ __ ___   ___| |/ (_) |_ 
        | |_ | '__/ _` | '_ ` _ \ / _ \ ' /| | __|
        |  _|| | | (_| | | | | | |  __/ . \| | |_ 
        |_|  |_|  \__,_|_| |_| |_|\___|_|\_\_|\__|

                     FRAMEKIT
                     by George Gil
*/

// =============================================================================
// Project      : FrameKit
// File         : include/FrameKit.h
// Author       : George Gil
// Created      : 2025-09-07
// Updated      : 2025-09-08
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//      Main include file for the FrameKit framework.
// =============================================================================


#pragma once

// FrameKit Info and Version
#include "FrameKit/Version.h"

// FrameKit Core Functionality
#include "FrameKit/Application/Application.h"


// Debugging and Profiling
#include "FrameKit/Debug/Log.h"
#include "FrameKit/Debug/Instrumentor.h"

// FrameKit Utilities
#include "FrameKit/Utilities/Time.h"
#include "FrameKit/Utilities/Memory.h"
#include "FrameKit/Utilities/Utilities.h"
#include "FrameKit/Utilities/UUID.h"
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
// Created      : 2025-08-12
// Updated      : 2025-09-06
// License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
// Description  :
//      Main include file for the FrameKit framework.
// =============================================================================

#pragma once

// FrameKit Info and Version
#include "FKVersion.h"

// FrameKit Core Functionality
#include "FrameKit/Base/Assert.h"
#include "FrameKit/Application/Application.h"
#include "FrameKit/Base/Layer.h"


// Debugging and Profiling
#include "FrameKit/Debug/Log.h"
#include "FrameKit/Debug/Instrumentor.h"

// FrameKit Utilities
#include "FrameKit/Utils/Timer.h"
#include "FrameKit/Utils/Timestep.h"
#include "FrameKit/Utils/UUID.h"
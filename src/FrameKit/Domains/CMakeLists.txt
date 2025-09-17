#[[
===================================== FrameKit =========================================
  Project      : FrameKit
  File         : src/FrameKit/Domains/CMakeLists.txt
  Author       : George Gil
  Created      : 2025-09-10
  Updated      : 2025-09-10
  License      : Dual Licensed: GPLv3 or Proprietary (c) 2025 George Gil
  Description  :
            CMakeLists.txt for FrameKit domains. Includes Window, Audio, Gfx domains.
========================================================================================
]]

# Add each domain here. Keeps root and core clean.
message(STATUS "=================================== FrameKit Domains ====================================")
add_subdirectory(Window)
# add_subdirectory(Gfx)    # later
# add_subdirectory(Audio)  # later
